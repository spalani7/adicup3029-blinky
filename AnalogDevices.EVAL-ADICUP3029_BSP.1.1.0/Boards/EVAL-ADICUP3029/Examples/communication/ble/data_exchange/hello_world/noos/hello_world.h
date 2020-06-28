/*!
 *****************************************************************************
   @file:    hello_world.h

   @brief:   Header file for HelloWorld.c
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


#ifndef HELLO_WORLD_H
#define HELLO_WORLD_H


#include <string.h>
#include <common.h>
#include <adi_ble_config.h>
#include <drivers/pwr/adi_pwr.h>
#include <radio/adi_ble_radio.h>
#include <common/adi_error_handling.h>
#include <framework/noos/adi_ble_noos.h>

/* Maximum string size that the data packet can handle  */
#define ADI_MAX_STRING_SIZE         (13u)

/* ID of the sensor sending "data". In this case data is a string */
#define SENSOR_ID                   (0x01u)

/* Macro function to assist in setting the packet header */
#define ADI_SET_HEADER(packet_type, id) ((id & 0x7Fu) | (packet_type << 7))

/*
 * enum ADI_PACKET_TYPE
 * brief Packet type IDs
 *
 * note These correlate with the Android application. So any
 *       changes here must be reflected in SharedDefines.java
 */
typedef enum {
    ADI_REGISTRATION_PACKET     = 0x00u,
    ADI_SENSOR_PACKET           = 0x01u,
} ADI_PACKET_TYPE;

/**
 * \enum ADI_SENSOR_TYPE
 * \brief Senor type IDs for the various types of sensors.
 *
 * \note These correlate with the Android application. So any
 *       changes here must be reflected in SharedDefines.java
 */
  typedef enum {
      ADI_GENERIC_TYPE              = 0x00u,
      ADI_ACCELEROMETER_2G_TYPE     = 0x01u,  
      ADI_CO_TYPE                   = 0x02u,
      ADI_TEMPERATURE_TYPE          = 0x03u,
      ADI_VISIBLELIGHT_TYPE         = 0x04u,
      ADI_PRINTSTRING_TYPE          = 0x05u,
      ADI_ACCELEROMETER_4G_TYPE     = 0x0Bu,
      ADI_ACCELEROMETER_8G_TYPE     = 0x0Cu,
  }ADI_SENSOR_TYPE;


/*
 *  @struct ADI_STRING_DATA
 *
 *  @brief  data structure for the #ADI_PRINTSTRING_TYPE
 *
 */
#pragma pack(push)
#pragma pack(1)
typedef struct
{
  uint8_t           nStringSize;      /*< Size of the string to send. Must not be larger than 13 bytes. */
  uint8_t           aStringData[13];  /*< String to send.                                               */

}ADI_STRING_DATA;
#pragma pack(pop)

/*
 *  @struct ADI_DATA_PACKET
 *
 *  @brief  packet structure to send data to the Android application
 *
 */
#pragma pack(push)
#pragma pack(1)
typedef struct
{
  uint8_t                     nPacketHeader;      /*< Packet header has the first bit set to 0x1u to indicated this is a
                                                      data packet. The rest of the 7 bits are used for the sensor id.
                                                      This should be a unique identifier of the sensor instance.    */
  ADI_SENSOR_TYPE             eSensorType;        /*< Sensor type                                                   */
  uint8_t                     aTimestamp[4];      /*< Timestamp value                                               */
  ADI_STRING_DATA             eStringData;        /*< String data                                                   */
} ADI_DATA_PACKET;
#pragma pack(pop)

#endif /* HELLO_WORLD_H */
