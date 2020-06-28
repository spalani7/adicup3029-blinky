            Analog Devices, Inc. ADICUP3029 Application Example


Project Name: mqtt_publisher_adxl362


Description:  Use the ADICUP3029's add-on Wi-Fi module ESP8266 to pusblish adxl362 accelerometer data using MQTT.

Overview:
=========
    The single threaded, non-RTOS program sends accelerometer adxl362 data to a MQTT broker via Wi-Fi. Any MQTT subscriber that is subscribed to the topic aMQTTTopicName defined in mqtt_publisher_adxl362.h should receive this data.


User Configuration Parameters:
==========================
	The following configuration parameters can be found in mqtt_publisher_adxl362.h. These should be in a string format.
	
	- aWifiSSID is the SSID of the access point to connect to. 
	- aWifiPassword is the password of the access point to connect to. 
	- aMQTTBrokerIp is the ip address of the broker to establish an MQTT connection with. 
	- aMQTTBrokerPort is the port of the broker to establish an MQTT connection with. 
	

Hardware Setup:
===============   
    Connect the PmodACL2 acceleromter to the SPI1 PMOD connector on the ADICUP3029 board. PMOD connectors are on 
    the opposite end of the board from the power connector. The underside of the board labels which of the two 
    PMOD connectors is for SPI1. 

    The ESP8266 Wi-Fi module plugs into the P1 connector on the ADICUP3029. This 
    example uses UART0 as the connection to the ESP8266. As a result, the following switch settings will need
    to be modified to route the UART signal to the Wi-Fi module. 
    
        Switch       Position
        ======       ========
        S2           WIFI
 
MQTT Broker and Subscriber Setup:
===============
    Please refer to the WiFi_Software_Users_Guide.pdf for details on
    how to set up the local subscriber and broker for this example.

External connections:
=====================
	None.

How to build and run:
=====================
    In CCES, build the project ("Project" --> "Build All"). Then click one of the ".launch" files located in the Project Explorer, and then press "Debug" icon. This will open the "Debug" perspective in CCES. Run the program ("Run" -> "Resume"). A shortcut in CCES to begin a debug session is to click on a ".launch" file and press F5 on the keyboard. 

Expected Result:
=====================
    Note: These messages are only printed to the debug console in debug mode. In release mode, nothing will print to 
    console or terminal.

	Starting MQTT publisher Wi-Fi ADXL362 accelerometer demo application
	
	Starting to Publish
	
	Publishing data..
	
	Publishing data..
	
	Publishing data..
	
	...
References:
===========
    ADICUP3029 Schematic.
    EVAL-ADICUP3029_Users_Guide.pdf
    WiFi_Software_Users_Guide.pdf