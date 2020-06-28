            Analog Devices, Inc. ADICUP3029 Application Example


Project Name: proximity_reporter


Description:  Use the ADICUP3029 on board Bluetooth Radio to demonstarte the proximity profile with the host application. 
 

Overview:
=========
    The single threaded, non-RTOS program will set up the proximity monitor profile and receive alerts from the host application. The user will set a path loss threshold value on the Android app, and when the path loss value exceeds that threshold a medium alert immediate alert event will be sent to the ADICUP3029. If the link is broken (can happen if the host app gets too far away from the ADICUP3029) a link loss event will be sent to the ADICUP3029 from the host application. 

User Configuration Macros:
==========================
    None. 


Hardware Setup:
===============   
    None.
 
Host App Setup:
===============
    Please refer to section "Installation" in the Android_Application_Users_Guide.pdf for details on
    how to install the Android app.


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
        4. Click on "ADI_BLE_PROXIMITY_MONITOR" once it becomes available on the list of devices. This will create a connection to the ADICUP3029 board.
        5. Change the path loss threshold bar on the Android application. If the path loss exceeds the bar then an medium alert event will be sent to the ADICUP3029. This can be seen on the console.
        6. Click "DISCONNECT" to disconnect from the ADICUP3029 board.


Expected Result:
=====================
    
	Starting Proximity Monitor Example.

	Waiting for connection. Initiate connection on central device please.

	Connected!

	GAP mode changed.

	LinkLoss Alert Event. Level = No Alert

	Connection interval updated.

	Connection interval updated.

	Immediate Alert Event. Level = Medium Alert

	Disconnected!


References:
===========
    ADICUP3029 Schematic.
    EVAL-ADICUP3029_Users_Guide.pdf
    IoT_Node_Users_Guide.pdf