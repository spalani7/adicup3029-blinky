/*!
 *****************************************************************************
   @file:    programmer.c

   @brief:   Memory Programmer Tool

   @details: Must be run in order to switch from the generic HCI communication
             layer (defined by the Bluetooth specification) to the proprietary
             communication protocol used in the ADI BLE SDK.

             All BLE sources are designed to work with this proprietary protocol,
             if the user wishes to use the Bluetooth radio with the HCI, all
             BLE sources (aside to the transport layer) are not usable.
  -----------------------------------------------------------------------------

Copyright (c) 2016 Analog Devices, Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Modified versions of the software must be conspicuously marked as such.
  - This software is licensed solely and exclusively for use with processors
    manufactured by or for Analog Devices, Inc.
  - This software may not be combined or merged with other code in any manner
    that would cause the software to become subject to terms and conditions
    which differ from those listed here.
  - Neither the name of Analog Devices, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.
  - The use of this software may or may not infringe the patent rights of one
    or more patent holders.  This license does not release you from the
    requirement that you obtain separate licenses from these patent holders
    to use this software.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
TITLE, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL ANALOG DEVICES, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, PUNITIVE OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, DAMAGES ARISING OUT OF CLAIMS OF INTELLECTUAL
PROPERTY RIGHTS INFRINGEMENT; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/


#include <common.h>
#include <drivers/pwr/adi_pwr.h>
#include <radio/adi_ble_radio.h>
#include <drivers/gpio/adi_gpio.h>
#include <common/adi_error_handling.h>
#include <transport/adi_ble_transport.h>
#include <framework/noos/adi_ble_noos.h>


/*! Switch that allows the user to switch back from ACI to HCI, set to 0 to avoid "multiple programs" in a factory setting */
#ifdef ADI_DEBUG
#define DEBUG_INFO DEBUG_MESSAGE 
#else
#define DEBUG_INFO
#endif


/*! Switch that allows the user to inspect OTP - writing is disabled and the content of OTP is dumped via DEBUG_INFO */
#define INSPECT_OTP (0u)


typedef enum
{
    ADI_PROGRAMMER_ERROR_NONE                = 0u,
    ADI_PROGRAMMER_ERROR_PWR_INIT            = 1u,
    ADI_PROGRAMMER_ERROR_COMPANION_INIT      = 2u,
    ADI_PROGRAMMER_ERROR_TAL_WRITE           = 3u,
    ADI_PROGRAMMER_ERROR_TAL_READ            = 4u,
    ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE   = 5u,
    ADI_PROGRAMMER_ERROR_TAL_INIT            = 6u,
    ADI_PROGRAMMER_ERROR_TAL_UNINIT          = 7u,
    ADI_PROGRAMMER_ERROR_WAIT_TIMEOUT        = 8u,
    ADI_PROGRAMMER_ERROR_ACI_LOCKED          = 9u,
    ADI_PROGRAMMER_ERROR_OTP_READBACK        = 10u,
    ADI_PROGRAMMER_ERROR_UART_INIT           = 11u
} ADI_PROGRAMMER_ERROR;


typedef ADI_PROGRAMMER_ERROR (* ADI_OTP_FUNCTION) (uint32_t nAddress, uint8_t nSize, uint8_t * pData);  


/*! Size of basic patch to switch modes */
#define PATCH_SIZE          (40u)

/*! Starting address of EM9304 OTP */
#define OTP_START           (0x100000u)

/*! End address of EM9304 OTP */
#define OTP_END             (0x1FFFFFu)

/*! Magic word to signal start of a patch, byte 0 */
#define MAGIC_BYTE_0        (0x33u)

/*! Magic word to signal start of a patch, byte 1 */
#define MAGIC_BYTE_1        (0x39u)

/*! Magic word to signal start of a patch, byte 2 */
#define MAGIC_BYTE_2        (0x6Du)

/*! Magic word to signal start of a patch, byte 3 */
#define MAGIC_BYTE_3        (0x65u)

/*! Starting location (in 40 byte patch) of the checksum */
#define CRC_START            (16u)

/*! Size of the checksum in the patch */
#define CRC_SIZE             (4u)

/*! Location of last byte of the MAC address (in 40 byte patch) */
#define MAC_LAST_BYTE_START  (35u)

/*! Size of buffer used for packing and unpacking radio transactions */
#define WORKING_MEMORY_SIZE  (64u)


/*! Buffer used for packing and unpacking radio transactions */
static uint8_t gaWorkingMemory[WORKING_MEMORY_SIZE];

/*! Global flag used for callback notification for HCI commands */
static volatile bool gbEventHappened;

/* Abstractions for dynamically determining the interface */
static ADI_OTP_FUNCTION adi_otp_Write;
static ADI_OTP_FUNCTION adi_otp_Read;
static uint8_t * pPatch;

/*! Patch to disable HCI */
static uint8_t gPatchDisableHci[PATCH_SIZE] = {MAGIC_BYTE_0, MAGIC_BYTE_1, MAGIC_BYTE_2, MAGIC_BYTE_3,  /* magic number */ 
                                               0x28u, 0x00u, 0x00u, 0x00u,                              /* size of container */
                                               0x01u,                                                   /* format version */
                                               0x01u,                                                   /* container type */
                                               0x00u,                                                   /* container identifier */
                                               0x14u,                                                   /* container header length */
                                               0x11u, 0x0Cu,                                            /* vendor build number */ 
                                               0x00u, 0x00u,                                            /* user build number */
                                               0x49u, 0x36u, 0xD5u, 0x59u,                              /* crc32 of the entire container */
                                               0x34u, 0x02u, 0x00u, 0x00u,                              /* payload */
                                               0x01u, 0x02u, 0x01u, 0x01u,                              /* payload */
                                               0xB7u, 0x04u, 0x00u, 0x05u,                              /* payload */
                                               0xF7u, 0x01u, 0x41u, 0x44u,                              /* payload */
                                               0x34u, 0xC2u, 0x01u, 0x00u};                             /* payload */

/*! Patch to enable HCI */
static uint8_t gPatchDisableAci[PATCH_SIZE] = {MAGIC_BYTE_0, MAGIC_BYTE_1, MAGIC_BYTE_2, MAGIC_BYTE_3,  /* magic number */ 
                                               0x28u, 0x00u, 0x00u, 0x00u,                              /* size of container */
                                               0x01u,                                                   /* format version */
                                               0x01u,                                                   /* container type */
                                               0x00u,                                                   /* container identifier */
                                               0x14u,                                                   /* container header length */
                                               0x11u, 0x0Cu,                                            /* vendor build number */ 
                                               0x00u, 0x00u,                                            /* user build number */
                                               0xD8u, 0xA7u, 0xBDu, 0xF7u,                              /* crc32 of the entire container */
                                               0x34u, 0x02u, 0x00u, 0x00u,                              /* payload */
                                               0x00u, 0x02u, 0x01u, 0x01u,                              /* payload */
                                               0xB7u, 0x04u, 0x00u, 0x05u,                              /* payload */
                                               0xF7u, 0x01u, 0x41u, 0x44u,                              /* payload */
                                               0x34u, 0xC2u, 0x01u, 0x00u};                             /* payload */
#ifdef CHANGE_MAC_ADDRESS

#if defined(MAC_ADDRESS_45)

static uint8_t gMac_CRC[4u]  = {0xF9u, 0x1Fu, 0xB5u, 0x64u};
static uint8_t gMac_LastByte = 0x45u;

#elif defined(MAC_ADDRESS_46)

static uint8_t gMac_CRC[4u]  = {0x29u, 0x65u, 0x15u, 0x23u};
static uint8_t gMac_LastByte = 0x46u;

#elif defined(MAC_ADDRESS_47)

static uint8_t gMac_CRC[4u]  = {0x99u, 0x4Cu, 0x75u, 0x1Eu};
static uint8_t gMac_LastByte = 0x47u;

#elif defined(MAC_ADDRESS_48)

static uint8_t gMac_CRC[4u]  = {0x48u, 0xDBu, 0x25u, 0x9Cu};
static uint8_t gMac_LastByte = 0x48u;

#elif defined(MAC_ADDRESS_49)

static uint8_t gMac_CRC[4u]  = {0xF8u, 0xF2u, 0x45u, 0xA1u};
static uint8_t gMac_LastByte = 0x49u;

#else

#error "Specify what the final byte of the MAC address should be using these macros."

#endif
#endif


/*! Local prototypes */
extern int32_t              adi_initpinmux                  (void);
#ifdef CHANGE_MAC_ADDRESS
static void                 change_mac                      (void);
#endif
static void                 programmer                      (void);
static ADI_PROGRAMMER_ERROR adi_programmer_Init             (void);
static ADI_PROGRAMMER_ERROR adi_programmer_Sequence         (void);
static ADI_PROGRAMMER_ERROR adi_otp_hci_Read                (uint32_t nAddress, uint8_t nSize, uint8_t * pData);
static ADI_PROGRAMMER_ERROR adi_otp_aci_Read                (uint32_t nAddress, uint8_t nSize, uint8_t * pData);
static ADI_PROGRAMMER_ERROR adi_otp_hci_Write               (uint32_t nAddress, uint8_t nSize, uint8_t * pData);
static ADI_PROGRAMMER_ERROR adi_otp_aci_Write               (uint32_t nAddress, uint8_t nSize, uint8_t * pData);
static void                 adi_programmer_AppCallback      (void * pCBParam, uint32_t Event, void * pArg);
static void                 adi_programmer_TalCallback      (void * pCBParam, uint32_t Event, void * pArg);
static ADI_PROGRAMMER_ERROR adi_programmer_hci_Reset        (void);
static uint8_t              adi_programmer_Wait             (void);
static bool                 adi_programmer_IsEventPending   (void);
static void                 adi_programmer_ClearEventPending(void);
static uint8_t              adi_programmer_Compare          (uint8_t * pExpected, uint8_t nExpected, uint8_t * pResult, uint8_t nResult);
static void                 adi_programmer_SpinLED          (bool bFailed);
static void                 adi_programmer_LightLED0        (bool bHigh);
static void                 adi_programmer_LightLED1        (bool bHigh);


/*********************************************************************************
                            TOP LEVEL FUNCTIONALITY
*********************************************************************************/


int main(void)
{
    /* This program will be overloaded to support MAC address changing and the usual programmer */
#ifdef CHANGE_MAC_ADDRESS
    /* We assume ACI */
    change_mac();
#else
    /* Supports HCI and ACI */
    programmer();
#endif

    return (0u);
}


#ifdef CHANGE_MAC_ADDRESS
static void change_mac(void)
{
    ADI_PROGRAMMER_ERROR eError;
    bool                 bFailed;

    eError = adi_programmer_Init();
    if (eError == ADI_PROGRAMMER_ERROR_NONE) 
    {
        if (adi_ble_Init(adi_programmer_AppCallback, NULL) == ADI_BLER_SUCCESS) 
        {
            /* ACI mode only supported for changing the MAC address (otherwise too many patches to support) */
            adi_otp_Write = adi_otp_aci_Write;
            adi_otp_Read  = adi_otp_aci_Read; 

            /* Use this patch since it still has "ACI enable" (i.e. we are not trying to switch modes) */
            pPatch        = gPatchDisableHci;

            /* Now overload the MAC address and CRC portions of the patch with the one specified */
            memcpy(&pPatch[CRC_START], &gMac_CRC, CRC_SIZE);
            pPatch[MAC_LAST_BYTE_START] = gMac_LastByte;

            /* Program the patch */
            eError = adi_programmer_Sequence();
        }
        else
        {
            eError = ADI_PROGRAMMER_ERROR_COMPANION_INIT;
        }
    }

    switch(eError)
    {
        case ADI_PROGRAMMER_ERROR_NONE:
                DEBUG_MESSAGE("MAC CHANGE SUCCESS.\r\n");
                bFailed = false;
            break;

        default:
                DEBUG_INFO("FAILURE: MAC programming failed with error code %d. Exiting.\r\n", (int) eError);
                DEBUG_MESSAGE("MAC CHANGE FAILED.\r\n");
                bFailed = true;
            break;   
    }

    adi_programmer_SpinLED(bFailed);
}
#endif


static void programmer(void)
{
    ADI_PROGRAMMER_ERROR eError    = ADI_PROGRAMMER_ERROR_NONE;
    bool                 bFailed   = true;

    /* Initialize and determine the interface currently being used */
    eError = adi_programmer_Init();
    switch(eError)
    {
        case ADI_PROGRAMMER_ERROR_NONE:
            DEBUG_INFO("INFO: Determined the interface, ready to switch.\r\n");

            /* Write OTP to switch to the other interface */
            eError = adi_programmer_Sequence();
            switch(eError)
            {
                case ADI_PROGRAMMER_ERROR_NONE:
                    DEBUG_INFO("INFO: Passed!\r\n");
                    DEBUG_MESSAGE("PROGRAMMING COMPLETE.\r\n");
                    bFailed = false;
                    break;

                default:
                    DEBUG_INFO("FAILURE: Sequence failed with error code %d. Exiting.\r\n", (int) eError);
                    bFailed = true;
                    break;
            }           
                 
            break;

        case ADI_PROGRAMMER_ERROR_ACI_LOCKED:
            DEBUG_INFO("INFO: In ACI mode, but locked from switching back. Exiting.\r\n");
            DEBUG_MESSAGE("PROGRAMMER SKIPPED.\r\n");
            bFailed = false;
            break;

        default:
            DEBUG_INFO("FAILURE: Initialziation failed with error code %d. Exiting.\r\n", (int) eError);
            bFailed = true;
            break;
    }

    if (bFailed == false)
    {
        DEBUG_MESSAGE("PROGRAMMER PASSED\r\n");
    }
    else
    {
        DEBUG_MESSAGE("PROGRAMMER FAILED.\r\n");
    }

    adi_programmer_SpinLED(bFailed);
}


static ADI_PROGRAMMER_ERROR adi_programmer_Init(void)
{
    adi_initpinmux();
    
    if (adi_pwr_Init() != ADI_PWR_SUCCESS)
    {
        return ADI_PROGRAMMER_ERROR_PWR_INIT;
    }

    if (adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u) != ADI_PWR_SUCCESS)
    {
        return ADI_PROGRAMMER_ERROR_PWR_INIT;
    }

    if (adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u) != ADI_PWR_SUCCESS)
    {
        return ADI_PROGRAMMER_ERROR_PWR_INIT;
    }

    common_Init();

#ifdef CHANGE_MAC_ADDRESS
    return ADI_PROGRAMMER_ERROR_NONE;
#endif

    if (adi_tal_Init(adi_programmer_TalCallback) != ADI_BLE_TRANSPORT_SUCCESS)
    {
        return ADI_PROGRAMMER_ERROR_TAL_INIT;
    }

    /* Clear LEDs (note that GPIO driver is initialized in adi_tal_Init) */
    adi_programmer_LightLED0(false);
    adi_programmer_LightLED1(false);

    /* Attempt an HCI reset, if it worked we are in HCI mode */
    if (adi_programmer_hci_Reset() == ADI_PROGRAMMER_ERROR_NONE)
    {
        /* Apply HCI functions and data to abstractions */
        adi_otp_Write = adi_otp_hci_Write;
        adi_otp_Read  = adi_otp_hci_Read;
        pPatch        = gPatchDisableHci;

        /* Notify user */
        DEBUG_INFO("INFO: Detected HCI. Switching to ACI.\r\n");
        
        /* Nothing else to do for HCI, we have the right functions and initializations - ready to go */
        return ADI_PROGRAMMER_ERROR_NONE;
    }
    /* If it didn't work, we are (probably) in ACI mode */
    else
    {
#ifdef ALLOW_SWITCHING_FROM_ACI_TO_HCI
        /* Apply ACI functions and data to abstractions */
        adi_otp_Write = adi_otp_aci_Write;
        adi_otp_Read  = adi_otp_aci_Read;
        pPatch        = gPatchDisableAci;

        /* Notify user */
        DEBUG_INFO("INFO: Detected ACI. Switching to HCI.\r\n");

        /* Uninitialize TAL, need to start using the radio layer instead of the transport layer */
        if (adi_tal_Uninit() != ADI_BLE_TRANSPORT_SUCCESS)
        {
            return ADI_PROGRAMMER_ERROR_TAL_UNINIT;
        }

        /* Initialize the framework layer and companion module */
        if (adi_ble_Init(adi_programmer_AppCallback, NULL) != ADI_BLER_SUCCESS)
        {
            return ADI_PROGRAMMER_ERROR_COMPANION_INIT;
        }
        
        /* Nothing else to do for ACI, we have the right functions and initializations - ready to go */
        return ADI_PROGRAMMER_ERROR_NONE;
#else                
        /* The ability to switch from ACI --> HCI has been locked to prevent multiple programs, quit! */
        return ADI_PROGRAMMER_ERROR_ACI_LOCKED; 
#endif
    }
}


static ADI_PROGRAMMER_ERROR adi_programmer_Sequence(void)
{
    ADI_PROGRAMMER_ERROR eError = ADI_PROGRAMMER_ERROR_NONE;
    uint32_t nTemp              = 0u;
    uint32_t nPrograms          = 0u;
    uint32_t nAddress           = OTP_START;
    uint32_t nLastAddress       = OTP_START;
    uint8_t  aOtpData[PATCH_SIZE];

    DEBUG_INFO("INFO: Reading the one-time programmable memory space.\r\n");
    
    while (nAddress < OTP_END)
    {
        eError = adi_otp_Read(nAddress, PATCH_SIZE, aOtpData);
        if (eError != ADI_PROGRAMMER_ERROR_NONE)
        {
            return (eError);
        }
#if defined(INSPECT_OTP) && INSPECT_OTP == 1u
        for (uint8_t i = 0u; i < PATCH_SIZE; i++)
        {
            DEBUG_INFO("%X : %X\r\n", nAddress + i, aOtpData[i]);
        }
#endif
        if ((aOtpData[0u] == MAGIC_BYTE_0) && (aOtpData[1u] == MAGIC_BYTE_1) && (aOtpData[2u] == MAGIC_BYTE_2) && (aOtpData[3u] == MAGIC_BYTE_3))
        {
            nPrograms++;
            
            nTemp = nAddress;
                
            nAddress += (((uint32_t) aOtpData[7u]) << 24u) | (((uint32_t) aOtpData[6u]) << 16u) | (((uint32_t) aOtpData[5u]) << 8u) | ((uint32_t) aOtpData[4u]);

            DEBUG_INFO("INFO: Found a valid patch at address %X of size %d bytes.\r\n", (int) nTemp, (int) (nAddress - nTemp));
            
            /* Get next 32-byte aligned address*/
            if ((nAddress % 32) != 0u)
            {
               nAddress = ((nAddress + 31) & ~0x1F );
            }
            
            nLastAddress = nAddress;
        }
        else
        {
            uint32_t nZeros = 0x0ul;
            for (uint32_t i = 0x0ul; i < PATCH_SIZE; i++)
            {
                if (aOtpData[i] == 0x0u)
                {
                    nZeros++;
                }
            }            

            if (nZeros != PATCH_SIZE)
            {
                DEBUG_INFO("WARNING: Found a region of invalid memory. This is not catastrophic, but indicates memory was programmed incorrectly once.\r\n");
                nLastAddress = nAddress + PATCH_SIZE;
            }
            else
            {
               /* We found enough space for the patch */
               break;
            }

            nAddress += PATCH_SIZE;
        }
    }

    float fTotal = (float) (OTP_END - OTP_START);
    float fUsed  = (((float) (nLastAddress - OTP_START)) / fTotal) * 100.0;

    DEBUG_INFO("INFO: Done traversing the programmable memory space.\r\n");
    DEBUG_INFO("INFO: The memory has been programmed %d times already.\r\n", (int) nPrograms);
    DEBUG_INFO("INFO: The next available space to program is address %X. %f percent of the usable memory space has been used.\r\n", (int) nLastAddress, fUsed);
#if !defined(INSPECT_OTP) || INSPECT_OTP == 0u
    DEBUG_INFO("INFO: Writing the patch to programmable memory... (this is irreversible)\r\n");

    eError = adi_otp_Write(nLastAddress, PATCH_SIZE, pPatch);
    if (eError != ADI_PROGRAMMER_ERROR_NONE)
    {
        return (eError);
    }

    DEBUG_INFO("INFO: The write was successful. Performing read-back for verification...\r\n");

    eError = adi_otp_Read(nLastAddress, PATCH_SIZE, aOtpData);
    if (eError != ADI_PROGRAMMER_ERROR_NONE)
    {
        return (eError);
    }    

    for (uint8_t i = 0u; i < PATCH_SIZE; i++)
    {
        if (aOtpData[i] != pPatch[i])
        {
            DEBUG_INFO("FAILURE: The write did not go through. Restart the program and try again.\r\n");
            return ADI_PROGRAMMER_ERROR_OTP_READBACK;     
        }
    }
    
    DEBUG_INFO("INFO: The readback was successful. Power cycle the part for the changes to take effect.\r\n");
#endif
    return ADI_PROGRAMMER_ERROR_NONE;
}


/*********************************************************************************
                            OTP INTERFACE
*********************************************************************************/


static ADI_PROGRAMMER_ERROR adi_otp_hci_Read(uint32_t nAddress, uint8_t nSize, uint8_t * pData)
{
    ASSERT(pData != NULL);

    ADI_PROGRAMMER_ERROR eError = ADI_PROGRAMMER_ERROR_NONE;
    uint8_t aHciCommandOtpRead[9u];
    uint8_t nReadBytes = 0u;

    aHciCommandOtpRead[0u] = 0x01u;
    aHciCommandOtpRead[1u] = 0x20u;
    aHciCommandOtpRead[2u] = 0xFCu;
    aHciCommandOtpRead[3u] = 0x05u;
    aHciCommandOtpRead[4u] = (nAddress & 0x000000FFu);
    aHciCommandOtpRead[5u] = (nAddress & 0x0000FF00u) >> 8u;
    aHciCommandOtpRead[6u] = (nAddress & 0x00FF0000u) >> 16u;
    aHciCommandOtpRead[7u] = (nAddress & 0xFF000000u) >> 24u;
    aHciCommandOtpRead[8u] = nSize;

    do
    {
        if (adi_tal_Write(aHciCommandOtpRead, 9u) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_WRITE;
            break;
        }

        if (adi_programmer_Wait() != 0u)
        {
            eError = ADI_PROGRAMMER_ERROR_WAIT_TIMEOUT;
            break;
        }

        if (adi_tal_Read(gaWorkingMemory, 64u, &nReadBytes) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ;
            break;
        }

        if (nReadBytes == (7u + nSize))
        {
            if (((((uint16_t) gaWorkingMemory[5u]) << 8u) | ((uint16_t) gaWorkingMemory[4u])) == 0xFC20u)
            {
                for (uint8_t i = 7u; i < (7u + nSize); i++)
                {
                    pData[i - 7u] = gaWorkingMemory[i];
                }
            }
            else
            {
                eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
                break;
            }
        }
        else
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }    

    } while(0u);

    return (eError);
}


static ADI_PROGRAMMER_ERROR adi_otp_aci_Read(uint32_t nAddress, uint8_t nSize, uint8_t * pData)
{
    ASSERT(pData != NULL);

    ADI_PROGRAMMER_ERROR eError = ADI_PROGRAMMER_ERROR_NONE;
    ADI_BLE_VENDOR_DATA  sVendorData;
    uint8_t              aAciCommandOtpRead[5u];

    aAciCommandOtpRead[0u] = (nAddress & 0x000000FFu);
    aAciCommandOtpRead[1u] = (nAddress & 0x0000FF00u) >> 8u;
    aAciCommandOtpRead[2u] = (nAddress & 0x00FF0000u) >> 16u;
    aAciCommandOtpRead[3u] = (nAddress & 0xFF000000u) >> 24u;
    aAciCommandOtpRead[4u] = nSize;

    do
    {
        if (adi_radio_SendVendorCommand(0xFC20u, 5u, aAciCommandOtpRead) != ADI_BLER_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_WRITE;
            break;
        }

        if (adi_ble_WaitForEventWithTimeout(CORE_VENDOR_COMMAND_COMPLETE, 3u) != ADI_BLER_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ;
            break;
        }

        adi_ble_GetVendorData(&sVendorData);

        if (sVendorData.nVendorOpCode != 0xFC20u)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }

        if (sVendorData.nDataLen != (nSize + 1u))
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }

        /* The first byte received is the status byte, so skip that when performing the memcpy */
        memcpy(pData, &sVendorData.pData[1u], nSize);

    } while(0u);

    return (eError);
}


static ADI_PROGRAMMER_ERROR adi_otp_hci_Write(uint32_t nAddress, uint8_t nSize, uint8_t * pData)
{
    ASSERT(pData != NULL);
    ASSERT((8u + nSize) < WORKING_MEMORY_SIZE);

    ADI_PROGRAMMER_ERROR eError     = ADI_PROGRAMMER_ERROR_NONE;
    uint8_t              nReadBytes = 0u;

    gaWorkingMemory[0u] = 0x01u;
    gaWorkingMemory[1u] = 0x22u;
    gaWorkingMemory[2u] = 0xFCu;
    gaWorkingMemory[3u] = 0x04u + nSize;
    gaWorkingMemory[4u] = (nAddress & 0x000000FFu);
    gaWorkingMemory[5u] = (nAddress & 0x0000FF00u) >> 8u;
    gaWorkingMemory[6u] = (nAddress & 0x00FF0000u) >> 16u;
    gaWorkingMemory[7u] = (nAddress & 0xFF000000u) >> 24u;
    memcpy(&gaWorkingMemory[8u], pData, nSize);

    do
    {
        if (adi_tal_Write(gaWorkingMemory, 8u + nSize) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_WRITE;
            break;
        }

        if (adi_programmer_Wait() != 0u)
        {
            eError = ADI_PROGRAMMER_ERROR_WAIT_TIMEOUT;
            break;
        }

        if (adi_tal_Read(gaWorkingMemory, 64u, &nReadBytes) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ;
            break;
        }        

        if (nReadBytes != 7u)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }

        if (((((uint16_t) gaWorkingMemory[5u]) << 8u) | ((uint16_t) gaWorkingMemory[4u])) != 0xFC22u)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }

    } while(0u);

    return (eError);
}


static ADI_PROGRAMMER_ERROR adi_otp_aci_Write(uint32_t nAddress, uint8_t nSize, uint8_t * pData)
{
    ASSERT(pData != NULL);
    ASSERT((4u + nSize) < WORKING_MEMORY_SIZE);

    ADI_PROGRAMMER_ERROR eError = ADI_PROGRAMMER_ERROR_NONE;
    ADI_BLE_VENDOR_DATA  sVendorData;

    gaWorkingMemory[0u] = (nAddress & 0x000000FFu);
    gaWorkingMemory[1u] = (nAddress & 0x0000FF00u) >> 8u;
    gaWorkingMemory[2u] = (nAddress & 0x00FF0000u) >> 16u;
    gaWorkingMemory[3u] = (nAddress & 0xFF000000u) >> 24u;
    memcpy(&gaWorkingMemory[4u], pData, nSize);

    do
    {
        if (adi_radio_SendVendorCommand(0xFC22u, 4u + nSize, gaWorkingMemory) != ADI_BLER_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_WRITE;
            break;
        }

        if (adi_ble_WaitForEventWithTimeout(CORE_VENDOR_COMMAND_COMPLETE, 3u) != ADI_BLER_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ;
            break;
        }

        adi_ble_GetVendorData(&sVendorData);

        if (sVendorData.nVendorOpCode != 0xFC22u)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }

        if (sVendorData.nDataLen != 1u)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
            break;
        }        

    } while(0u);

    return (eError);
}


/*********************************************************************************
                            CALLBACK FUNCTIONS
*********************************************************************************/

/**
 *  @brief      Application Callback
 *
 *  @details    This function is only relevent during the ACI programming sequence
 *              (switch from ACI -> HCI), since the Non-RTOS "framework" is used.
 *              But this function will only be called if an unexpected event happens,
 *              since we aren't doing any flushing or dispatching of events - just 
 *              responses and waiting explicitly for events.
 *
 */
static void adi_programmer_AppCallback(void * pCBParam, uint32_t Event, void * pArg)
{
    switch(Event)
    {
        case BLE_RADIO_ERROR_READING:
            DEBUG_INFO("Failed to read a packet from the radio.\r\n");
            break;

        case BLE_RADIO_ERROR_PARSING:
            DEBUG_INFO("Failed to parse a packet from the radio.\r\n");
            break;      
            
        case BLE_RADIO_ERROR_PROCESSING:
            DEBUG_INFO("Failed to process a packet from the radio.\r\n");
            break;                                

        default:
            DEBUG_INFO("Unexpected event received: %X.\r\n", (int) Event);
            break;
    }
}


/*
 *  @brief      Transport Layer Callback
 *
 *  @details    For calls directly into the TAL, we need to implement the waiting 
 *              functionality.
 *
 */
static void adi_programmer_TalCallback(void * pCBParam, uint32_t Event, void * pArg)
{   
    gbEventHappened = true;
}


/*********************************************************************************
                            HELPER FUNCTIONS
*********************************************************************************/


static ADI_PROGRAMMER_ERROR adi_programmer_hci_Reset(void)
{
    ADI_PROGRAMMER_ERROR eError   = ADI_PROGRAMMER_ERROR_NONE;
    uint8_t aHciCommandReset [4u] = {0x01u, 0x03u, 0x0Cu, 0x00u};
    uint8_t aHciResponseReset[7u] = {0x04u, 0x0Eu, 0x04u, 0x01u, 0x03u, 0x0Cu, 0x00u};
    uint8_t nReadBytes            = 0u;

    do
    {
        if (adi_tal_Write(aHciCommandReset, 4u) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_WRITE;
            break;
        }

        if (adi_programmer_Wait() != 0u)
        {
            eError = ADI_PROGRAMMER_ERROR_WAIT_TIMEOUT;
            break;
        }

        if (adi_tal_Read(gaWorkingMemory, 64u, &nReadBytes) != ADI_BLE_TRANSPORT_SUCCESS)
        {
            eError = ADI_PROGRAMMER_ERROR_TAL_READ;
            break;
        }

        if (adi_programmer_Compare(aHciResponseReset, 7u, gaWorkingMemory, nReadBytes) != 0u)
        {
            /* FIXME:
             *
             * In theory, we should just error out and break here. But for some reason when 
             * using the EM9304 in HCI mode, the first HCI reset response is some junk code.
             * So we need to wait for the second response which will have the correct response.
             * 
             * We've seen this behavior differ slightly on versions, so we need to be wary when
             * a new version is released and verify the behavior is consistent.
             *
             * For the purposes of the programmer, this is safe. In HCI mode, we will get a passing
             * return code in this functon. For ACI mode, we will error out while waiting for the next 
             * packet, or if does return something a second time - it will still be garbage causing us to 
             * error out.
             *
             */

            if (adi_programmer_Wait() != 0u)
            {
                eError = ADI_PROGRAMMER_ERROR_WAIT_TIMEOUT;
                break;
            }

            if (adi_tal_Read(gaWorkingMemory, 64u, &nReadBytes) != ADI_BLE_TRANSPORT_SUCCESS)
            {
                eError = ADI_PROGRAMMER_ERROR_TAL_READ;
                break;
            }

            if (adi_programmer_Compare(aHciResponseReset, 7u, gaWorkingMemory, nReadBytes) != 0u)
            {
                eError = ADI_PROGRAMMER_ERROR_TAL_READ_RESPONSE;
                break;
            }
        }

    } while(0u);

    return (eError);
}


static uint8_t adi_programmer_Wait(void)
{
    uint32_t nCounter = 0xFFFFul;
    uint8_t  nError   = 0u;

    while(adi_programmer_IsEventPending() == false) 
    {
        nCounter--;
        if (nCounter == 0u)
        {
            nError = 1u;
            break;
        }
    }

    adi_programmer_ClearEventPending();

    return (nError);
}


static bool adi_programmer_IsEventPending(void)
{
    return (gbEventHappened);
}


static void adi_programmer_ClearEventPending(void)
{
    gbEventHappened = false;
}


static uint8_t adi_programmer_Compare(uint8_t * pExpected, uint8_t nExpected, uint8_t * pResult, uint8_t nResult)
{
    uint8_t nError = 0u;

    if (nExpected != nResult)
    {
        nError = 1u;
    }
    else
    {
        for (uint8_t i = 0u; i < nExpected; i++)
        {
            if (pExpected[i] != pResult[i])
            {
                nError = 1u;
                break;
            }
        }
    }

    return (nError);
}


static void adi_programmer_SpinLED(bool bFailed)
{
    adi_programmer_LightLED0(false);
    adi_programmer_LightLED1(false);

    while(1u)
    {
        if (bFailed == false)
        {
            adi_programmer_LightLED0(true);
            for (volatile uint32_t i = 0u; i < 0x2FFFFFul; i++);
            adi_programmer_LightLED0(false);
            for (volatile uint32_t i = 0u; i < 0x2FFFFFul; i++);
        } 
        else
        {
            adi_programmer_LightLED1(true);
            for (volatile uint32_t i = 0u; i < 0x2FFFFul; i++);
            adi_programmer_LightLED1(false);            
            for (volatile uint32_t i = 0u; i < 0x2FFFFul; i++);
        }
    }
}


static void adi_programmer_LightLED0(bool bHigh)
{
    adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_0, true);

    /* Green light */
    if (bHigh == true)
    {
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_0);
    }
    else
    {
        adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_0);   
    }
}


static void adi_programmer_LightLED1(bool bHigh)
{
    adi_gpio_OutputEnable(ADI_GPIO_PORT1, ADI_GPIO_PIN_15, true);

    /* Red light */
    if (bHigh == true)
    {
        adi_gpio_SetHigh(ADI_GPIO_PORT1, ADI_GPIO_PIN_15);
    }
    else
    {
        adi_gpio_SetLow(ADI_GPIO_PORT1, ADI_GPIO_PIN_15);   
    }    
}
