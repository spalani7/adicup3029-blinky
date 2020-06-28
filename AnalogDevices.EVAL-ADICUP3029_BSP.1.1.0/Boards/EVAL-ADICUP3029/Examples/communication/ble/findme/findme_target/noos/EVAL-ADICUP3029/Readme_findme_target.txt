            Analog Devices, Inc. ADICUP3029 Application Example


Project Name: findme_target


Description:  Use the ADICUP3029 on board Bluetooth Radio to demonstrate the findme profile with the host application. 
 

Overview:
=========
    The single threaded, non-RTOS program will set up the immediate alert Bluetooth service and receive alerts from the host application. The host application has the option to send three different alert levels, No Alert, Medium Alert, and High Alert. 



User Configuration Macros:
==========================
    None. 


Hardware Setup:
===============   
    None.
 
Host App Setup:
===============
    Please refer to section "Installation" in the Android_Application_Users_Guide.pdf for details on how to install the Android app.


External connections:
=====================
    Connect the ADICUP3029 to the host PC using the USB cable.


How to build and run:
=====================
    In CCES, build the project ("Project" --> "Build All"). Then click one of the ".launch" files located in the Project Explorer, and then press "Debug" icon. This will open the "Debug" perspective in CCES. Run the program ("Run" -> "Resume"). A shortcut in CCES to begin a debug session is to click on a ".launch" file and press F5 on the keyboard. 

    To run the example:
        1. Build and run the demo. 
        2. On the phone/table open the Android application ADIotNode.
        3. Press the "SCAN" button.
        4. Click on "ADI_BLE_FINDME_TARGET" once it becomes available on the list of devices. This will create a connection to the ADICUP3029 board.
        5. Click the buttons "NO ALERT", "MEDIUM ALERT" and "HIGH ALERT" on the Android applicationto send alerts to the ADICUP3029. These can be seen on the console.
        6. Click "DISCONNECT" to disconnect from the ADICUP3029 board.


Expected Result:
=====================
    Starting Findme Target Example.

	Waiting for connection. Initiate connection on central device please.

	Connected!

	Connected Address 0x35:0x36:0x4c:0x61:0x82:0x44

	GAP mode changed.

	Connection interval updated.

	Connection interval updated.

	FindMe Target Alert Event. Level = No Alert

	FindMe Target Alert Event. Level = Medium Alert

	FindMe Target Alert Event. Level = High Alert

	FindMe Target Alert Event. Level = Medium Alert

	FindMe Target Alert Event. Level = No Alert

	FindMe Target Alert Event. Level = High Alert

	Disconnected!


References:
===========
    ADICUP3029 Schematic.
    EVAL-ADICUP3029_Users_Guide.pdf
    IoT_Node_Users_Guide.pdf
