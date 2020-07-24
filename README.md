# ESP32-Evo-Remote-Pilot-NMEA2000
This is a Remote Control for EVO-1 Auto Pilot via NMEA2000.

This is an early stage development for EVO-1 Remot Controle with a 433MHz Sender (Key Fob).

It is based on the work of this project in GitHub: https://github.com/matztam/raymarine-evo-pilot-remote.
Many thaks for all the work to identify the NMEA 2000 codes for the EV-1 Course Computer.

The hardware consists of an ESP32 NodeMCU (here from AZ-Delivery). A CAN Bus transceiver and a power converter.
Thats's all what is required to controle an EV-1 based Auto Pilot with a simple 433 MHz remote control.

![Schematics](https://github.com/AK-Homberger/ESP32-Evo-Remote-Pilot-NMEA2000/blob/master/ESP32EvoPilotSchematics.png)


The code is based on the NMEA 2000 / NMEA0183 libraries from Timo Lappalainen (https://github.com/ttlappalainen). Download and install the libraries from GitHub link above:

 - NMEA2000
 - NMEA2000_esp32
 - NMEA0183

The Remote Control is using the RCswitch library (to be installed with the Arduiono IDE Library Manager).

For the ESP32 CAN bus, I used the "Waveshare SN65HVD230 Can Board" as transceiver. It works well with the ESP32. For the Gateway, I use the pins GPIO4 for CAN RX and GPIO2 for CAN TX.

The 12 Volt is reduced to 5 Volt with a DC Step-Down_Converter (D24V10F5, https://www.pololu.com/product/2831).

GPIO15 is used for the data receiver from the RXB6 receiver.
For programming the KeyFob. please have a look to the Arduino/Seatalk version of the Remote Control (code will be added soon for ESP32).


Version 0.1, 24.07.2020: Initial version.
