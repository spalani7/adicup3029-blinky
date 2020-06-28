/*****************************************************************************
 * mqtt_wifi_demo.c
 *****************************************************************************/


#include "mqtt_publisher_adxl362.h"
#include <common.h>
#include <common/adi_timestamp.h>

using namespace adi_sensor_swpack;

/* Static data */
static ADI_DATA_PACKET    			gSensorData;
static ADI_WIFI_TCP_CONNECT_CONFIG  gTCPConnect = {
													.nLinkID = ADI_WIFI_CONNECTION_ID,
													.pType = (uint8_t *) "TCP",
													.pIP   = aMQTTBrokerIp,
													.pPort =  aMQTTBrokerPort,
													.nTCPKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE};

static ADI_WIFI_MQTT_CONNECT_CONFIG gMQTTConnect = {
													.nLinkID = ADI_WIFI_CONNECTION_ID,
													.nVersion = ADI_WIFI_MQTT_PUBLISHER_VERSION,
													.pName = aMQTTPublisherName,
													.nMQTTKeepAlive = ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE };

static ADI_WIFI_PUBLISH_CONFIG 		gPublishConfig = {
		 	 	 	 	 	 	 	 	 	 	 	 .pMQTTData = (uint8_t *) &gSensorData,
		 	 	 	 	 	 	 	 	 	 	 	 .nMQTTDataSize = sizeof(ADI_DATA_PACKET),
													 .pTopic = aMQTTTopicName,
													 .nLinkID = ADI_WIFI_CONNECTION_ID,
													 .nQos = ADI_WIFI_MQTT_PUBLISER_QOS};



/* Local Wi-Fi functions */
static void            InitWiFiConnection(void);
static void            adi_wifi_AplicationCallback(void * pCBParam, uint32_t nEvent, void * pArg);
static void			   MQTTPublishADXL(Accelerometer *pAxl);

/* Local system functions */
static void InitSystem(void);
static void Trap(void);
static void RepairConnection(void);

/*!
 * @brief      Main
 *
 * @details    Application entry point.
 *
 * @param [in] argc : Number of arguments (unused)
 *
 * @param [in] argv : Arguments (unused)
 *
 */
int main(int argc, char *argv[])
{
    ADXL362         adxl362;
    Accelerometer  *pAxl = &adxl362;
    SENSOR_RESULT   eSensorResult;

    /* Initialize the system */
    InitSystem();

    /* Open Accelerometer */
    eSensorResult = pAxl->open();

    if(eSensorResult != SENSOR_ERROR_NONE) {
        PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
        Trap();
    }

    /* Start measurement */
    eSensorResult = pAxl->start();

    if(pAxl->start() != SENSOR_ERROR_NONE) {
        PRINT_SENSOR_ERROR(DEBUG_MESSAGE, eSensorResult);
        Trap();
    }

	/* Publish ADXL362 accelerometer data using MQTT */
	MQTTPublishADXL(pAxl);
}

/*!
 * @brief      Application Callback
 *
 * @details    Called by the framework layer (adi_wifi_noos.c) when an event occurs.
 *
 * @param [in] pCBParam : Callback parameter (unused)
 *
 * @param [in] Event : Event of type #ADI_WIFI_AT_CMDCODE.
 *
 * @param [in] pArg : Callback argument (unused)
 *
 */
void adi_wifi_AplicationCallback(void * pCBParam, uint32_t nEvent, void * pArg)
{
	switch (nEvent)
		{
			case CMD_NONE:
				/* Read the publish Data. */
				adi_wifi_ParseSubscriberData();
				break;
			case CMD_HW_ERROR:
			{
			    break;
			}

			default:
				break;
		}

	return;
}

/*!
 * @brief      ESP8266 MQTT publisher demo using ADXL362 accelerometer data.
 *
 * @details    Publishes data over Wi-Fi to the broker.
 */
static void MQTTPublishADXL(Accelerometer *pAxl)
{
	ADI_WIFI_RESULT eResult;
    uint32_t        nTime = 0u;
    uint32_t        nPacketId = 0u;
    uint8_t         nConnectionFailFlag;

    /* Initialize the ESP8266 Wi-Fi module and establish a TCP connection */
    InitWiFiConnection();

    /* Send MQTT "CONNECT" message to broker */
    eResult = adi_wifi_radio_MQTTConnect(&gMQTTConnect);
    DEBUG_RESULT("Error connecting to the MQTT broker.\r\n", eResult, ADI_WIFI_SUCCESS);

    /* Start the ping timer to ping the MQTT broker before the keep alive expires */
    if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) == ADI_WIFI_SUCCESS)
    {
    	DEBUG_MESSAGE("Starting to Publish\n");

		gPublishConfig.nPacketId = nPacketId;

		while(1)
		{
			nConnectionFailFlag = 0;

            /* If ping timer has expired */
            if (adi_wifi_IsTimerDone(1u) == 1u)
            {
        		/* Send MQTT "PINGREQ" message to broker to keep the connection alive. */
        		eResult = adi_wifi_radio_MQTTPing(ADI_WIFI_CONNECTION_ID);

        		/* Restart the ping timer */
        		if((adi_wifi_StopTimer(1u) == ADI_WIFI_SUCCESS) && (eResult == ADI_WIFI_SUCCESS))
            	{
        			if(adi_wifi_StartTimer(ADI_WIFI_MQTT_PING_TIMEOUT, 1u) != ADI_WIFI_SUCCESS)
                    {
        		    	DEBUG_MESSAGE("Failed to start ping timer\n");

                    }
            	}
            	else
            	{
	    	        DEBUG_MESSAGE("Failed to ping broker\n");
    		    	nConnectionFailFlag = 1;            	
    		    }
            }

           	/* If we did not fail to ping the broker, publish data */
            if(nConnectionFailFlag == 0u)
            {
            	DEBUG_MESSAGE("Publishing data..\n");

			    /* Pack the accelerometer data packet */
			    gSensorData.nPacketHeader = ADI_SET_HEADER(ADI_DATA_PACKET_TYPE, ADI_ACCELEROMETER_ID);
			    gSensorData.eSensorType = ADI_ACCELEROMETER_2G_TYPE;
			    
				/* Get timestamp */
				nTime = GET_TIME();
				memcpy(&gSensorData.aTimestamp, &nTime, 4u);

				/* Get x,y,x accelerometer data */
				pAxl->getXYZ((uint8_t*)&gSensorData.aPayload, 6u);

				/* Publish accelerometer data */
				if(adi_wifi_radio_MQTTPublish(&gPublishConfig) == ADI_WIFI_FAILURE)
				{
					nConnectionFailFlag = 1u;
				}
            }

            /* If we failed to ping the broker or failed to publish data, check the connection */
			if(nConnectionFailFlag == 1u)
			{
				DEBUG_MESSAGE("Troubleshooting failed connection..\n");
				RepairConnection();
			}

			gPublishConfig.nPacketId++;
		}
    }
}

/*!
 * @brief      Initializes the ESP8266 Wi-Fi module.
 *
 * @details    Initialize the ESP8266 Wi-Fi module and test the
 *             hardware connectivity. Try to connect to an access
 *             point and establish a TCP connection.
 */
void InitWiFiConnection(){
	ADI_WIFI_RESULT eResult;

    /* Initialize the ESP8266 Wi-Fi module */
	eResult = adi_wifi_Init(adi_wifi_AplicationCallback, NULL);
    DEBUG_RESULT("Error initializing the ESP8266 module.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_TestAT();
    DEBUG_RESULT("Error testing hardware connectivity.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_SetWiFiMode(1);
    DEBUG_RESULT("Error establishing a tcp connection.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_DisconnectFromAP();
    DEBUG_RESULT("Error disconnecting from access point.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL);
    DEBUG_RESULT("Error connecting to access point.\r\n", eResult, ADI_WIFI_SUCCESS);

	eResult = adi_wifi_radio_EstablishTCPConnection(&gTCPConnect);
    DEBUG_RESULT("Error establishing a tcp connection.\r\n", eResult, ADI_WIFI_SUCCESS);
}

/*!
 * @brief      Initializes the system
 *
 * @details    This function is responsible for initializing the pinmuxing, power service
 *             and wifi subsystem. It also initializes the realtime clock for to timestamp
 *             the outgoing sensor data packets.
 */
static void InitSystem(void)
{
    ADI_PWR_RESULT  ePwr;

    /* Explicitly disable the watchdog timer */
    *pREG_WDT0_CTL = 0x0u;

    /* Pinmux */
    adi_initpinmux();

    /* Initialize clocks */
    ePwr = adi_pwr_Init();
    DEBUG_RESULT("Error initializing the power service.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u);
    DEBUG_RESULT("Error configuring the core clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    ePwr = adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u);
    DEBUG_RESULT("Error configuring the peripheral clock.\r\n", ePwr, ADI_PWR_SUCCESS);

    /* Init timestamping */
    INIT_TIME();

    DEBUG_MESSAGE("Starting MQTT publisher Wi-Fi ADXL362 accelerometer demo application\r\n");

}

/*!
 * @brief      Troubleshoot and attempt to fix connection
 *
 * @details    This function is called if the ESP8266 has failed to communicate with the AP or broker.
 *             It will attempt to diagnose and repair the connectivity problem.
 */
static void RepairConnection()
{
    uint8_t         nConnectionStatus;

	if(adi_wifi_radio_GetConnectionStatus(&nConnectionStatus) == ADI_WIFI_SUCCESS)
	{

		switch(nConnectionStatus)
		{
			/*The  Station is connected to an AP and its IP is obtained, so try to connect to broker. */
			case 2:
			{
				/* Send MQTT "CONNECT" message to broker */
				if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
				{
					DEBUG_MESSAGE("Failed to do MQTT Connect\n");
				}
				break;
			}
			
			/* The  Station has created a TCP or UDP transmission so you should try to publish again. */
			case 3:
				break;

			/* The TCP or UDP transmission of Station is disconnected. So try to establish a TCP
			 * connection and then connect to the broker.
			 */
			case 4:
			{
				if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) == ADI_WIFI_SUCCESS)
				{
					if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
					{
						DEBUG_MESSAGE("Failed to do MQTT Connect\n");
					}
				}
				else
				{
					DEBUG_MESSAGE("Failed to establish connection to broker\n");
				}
				break;
			}
			/*The  Station does NOT connect to an AP. So try to connect to the AP. */
			case 5:
			{
				if(adi_wifi_radio_ConnectToAP(aWifiSSID, aWifiPassword,  NULL) == ADI_WIFI_SUCCESS)
				{
					if(adi_wifi_radio_EstablishTCPConnection(&gTCPConnect) == ADI_WIFI_SUCCESS)
					{
						if(adi_wifi_radio_MQTTConnect(&gMQTTConnect) != ADI_WIFI_SUCCESS)
						{
							DEBUG_MESSAGE("Failed to do MQTT Connect\n");
						}
					}
					else
					{
						DEBUG_MESSAGE("Failed to establish connection to broker\n");
					}

				}
				else
				{
					DEBUG_MESSAGE("Failed to Connect to AP\n");
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

/*!
 * @brief      Trap function
 *
 * @details    In case of catastrophic errors this function is called to block
 *             infinitely.
 */
static void Trap()
{
    while(1);
}
