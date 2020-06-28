/*****************************************************************************
 * mqtt_wifi_demo.h
 *****************************************************************************/

#ifndef __MQTT_WIFI_DEMO_H__
#define __MQTT_WIFI_DEMO_H__

#include <base_sensor/adi_sensor.h>
#include <base_sensor/adi_sensor_packet.h>
#include <base_sensor/adi_sensor_errors.h>
#include <axl/adxl362/adi_adxl362.h>
#include <framework/noos/adi_wifi_noos.h>

/* Defined in pinmux.c. */
extern "C" int32_t adi_initpinmux(void);

/********************************* Wi-Fi Configuration *********************************/
/* SSID of the access point. */
uint8_t aWifiSSID[] = "";

/* Password of the access point. */
uint8_t aWifiPassword[] = "";

/********************************* MQTT Configuration *********************************/

/* IP address of the broker to publish to. */
uint8_t aMQTTBrokerIp[] = "";

/* Port of the broker to publish to. */
uint8_t aMQTTBrokerPort[] = "";

/*! MQTT publisher name. */
uint8_t aMQTTPublisherName[] = "ADI_WIFI_MQTT";

/*! MQTT Topic name. This size of this string should be ADI_WIFI_MQTT_PACKET_SIZE - sizeof(ADI_DATA_PACKET) - 2u,
 *  If you want a longer topic name, increase the size of ADI_WIFI_MQTT_PACKET_SIZE.
 */
uint8_t aMQTTTopicName[] = "TOPIC_ADI_ADXL362";

/*! MQTT publish packet quality of service. */
#define ADI_WIFI_MQTT_PUBLISER_QOS                 (0u)

/*! MQTT publisher version. */
#define ADI_WIFI_MQTT_PUBLISHER_VERSION            (3u)

/*! MQTT publisher <-> broker connection keep alive timeout. */
#define ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE          (7200u)

/*! Connection link id, valid id range (0-4) for multiple connections. 5 indicates single connection mode. */
#define ADI_WIFI_CONNECTION_ID                     (5u)

/*! MQTT publisher <-> broker PING command timeout. */
#define ADI_WIFI_MQTT_PING_TIMEOUT                 (ADI_WIFI_MQTT_PUBLISHER_KEEPALIVE - 1000u)

/********************************* Sensor Configuration *********************************/

/* Accelerometer instance ID. */
#define ADI_ACCELEROMETER_ID    				   (1u)

#endif /* __MQTT_WIFI_DEMO_H__ */
