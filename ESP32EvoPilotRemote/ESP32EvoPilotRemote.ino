/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// NMEA2000 Remote Control for EV-1
//   - Reads 433 MHz commands via RXB6 receiver
//   - Sends NMEA2000 messages to EV-1 Course Computer

// Version 0.4, 04.08.2020, AK-Homberger

#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to 5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to 4

#include <Arduino.h>
#include <Preferences.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <NMEA2000_CAN.h>
#include <RCSwitch.h>

#include "RaymarinePilot.h"
#include "N2kDeviceList.h"

#define ESP32_RCSWITCH_PIN GPIO_NUM_15  // Set RCSWITCH port to 15 (RXB6 receiver)
#define KEY_DELAY 300  // 300 ms break between keys
#define BEEP_TIME 200  // 200 ms beep time

#define BUZZER_PIN 2  // Buzzer connected to GPIO 2

int NodeAddress;  // To store last Node Address

Preferences preferences;             // Nonvolatile storage on ESP32 - To store LastDeviceAddress

RCSwitch mySwitch = RCSwitch();

unsigned long key_time = 0;
unsigned long beep_time = 0;
bool beep_status = false;

const unsigned long Key_Minus_1 = 1111001; // Change values to individual values programmed to remote control
const unsigned long Key_Plus_1 = 1111002;
const unsigned long Key_Minus_10 = 1111003;
const unsigned long Key_Plus_10 = 1111004;
const unsigned long Key_Auto = 1111005;
const unsigned long Key_Standby = 1111006;

const unsigned long TransmitMessages[] PROGMEM = {126208UL,   // Set Pilot Mode
                                                  126720UL,   // Send Key Command
                                                  65288UL,    // Send Seatalk Alarm State
                                                  0
                                                 };

const unsigned long ReceiveMessages[] PROGMEM = { 127250UL,   // Read Heading
                                                  65288UL,    // Read Seatalk Alarm State
                                                  65379UL,    // Read Pilot Mode
                                                  0
                                                };

tN2kDeviceList *pN2kDeviceList;
short pilotSourceAddress = -1;


void setup() {
  uint8_t chipid[6];
  uint32_t id = 0;
  int i = 0;

  esp_efuse_read_mac(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

  // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
  NMEA2000.SetN2kCANReceiveFrameBufSize(150);
  NMEA2000.SetN2kCANMsgBufSize(8);
  // Set Product information
  NMEA2000.SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "Evo Pilot Remote",  // Manufacturer's Model ID
                                 "1.0.0.0",  // Manufacturer's Software version code
                                 "1.0.0.0" // Manufacturer's Model version
                                );
  // Set device information
  NMEA2000.SetDeviceInformation(id, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );

  Serial.begin(115200);
  delay(100);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  mySwitch.enableReceive(ESP32_RCSWITCH_PIN);  // Receiver on GPIO15 on ESP32

  // Uncomment 3 rows below to see, what device will send to bus
  //NMEA2000.SetForwardStream(&Serial);  // PC output on due programming port
  //NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
  //NMEA2000.SetForwardOwnMessages();

  preferences.begin("nvs", false);                          // Open nonvolatile storage (nvs)
  NodeAddress = preferences.getInt("LastNodeAddress", 32);  // Read stored last NodeAddress, default 32
  preferences.end();
  Serial.printf("NodeAddress=%d\n", NodeAddress);

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly, NodeAddress); //N2km_NodeOnly N2km_ListenAndNode
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.ExtendReceiveMessages(ReceiveMessages);

  NMEA2000.SetMsgHandler(RaymarinePilot::HandleNMEA2000Msg);

  pN2kDeviceList = new tN2kDeviceList(&NMEA2000);
  //NMEA2000.SetDebugMode(tNMEA2000::dm_ClearText); // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.Open();

  Serial.println((String) "NMEA2000 Open");
}


// Beep on if key received

void BeepOn() {
  if (beep_status == true) return;  // Already On

  digitalWrite(BUZZER_PIN, HIGH);
  beep_time = millis();
  beep_status = true;
}


// Beep off after BEEP_TIME

void BeepOff() {
  if (beep_status == true && millis() > beep_time + BEEP_TIME) {
    digitalWrite(BUZZER_PIN, LOW);
    beep_status = false;
  }
}


// Get device source address (of EV-1)

int getDeviceSourceAddress(String model) {
  if (!pN2kDeviceList->ReadResetIsListUpdated()) return -1;
  for (uint8_t i = 0; i < N2kMaxBusDevices; i++) {
    const tNMEA2000::tDevice *device = pN2kDeviceList->FindDeviceBySource(i);
    if ( device == 0 ) continue;

    String modelVersion = device->GetModelVersion();

    if (modelVersion.indexOf(model) >= 0) {
      return device->GetSource();
    }
  }
  return -2;
}


// Receive 433 MHz commands from remote and send SeatalkNG codes to EV-1 (if available)

void Handle_AP_Remote(void) {
  unsigned long key = 0;

  if (pilotSourceAddress < 0) pilotSourceAddress = getDeviceSourceAddress("EV-1"); // Try to get EV-1 Source Address

  if (mySwitch.available()) {
    key = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();
  }

  if (key > 0 && millis() > key_time + KEY_DELAY) {
    key_time = millis();   // Remember time of last key received

    if (key == Key_Standby) {
      Serial.println("Setting PILOT_MODE_STANDBY");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::SetEvoPilotMode(N2kMsg, pilotSourceAddress, PILOT_MODE_STANDBY);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }

    else if (key == Key_Auto) {
      Serial.println("Setting PILOT_MODE_AUTO");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::SetEvoPilotMode(N2kMsg, pilotSourceAddress, PILOT_MODE_AUTO);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }

    else if (key == Key_Plus_1) {
      Serial.println("+1");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::KeyCommand(N2kMsg, pilotSourceAddress, KEY_PLUS_1);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }

    else if (key == Key_Plus_10) {
      Serial.println("+10");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::KeyCommand(N2kMsg, pilotSourceAddress, KEY_PLUS_10);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }

    else if (key == Key_Minus_1) {
      Serial.println("-1");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::KeyCommand(N2kMsg, pilotSourceAddress, KEY_MINUS_1);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }

    else if (key == Key_Minus_10) {
      Serial.println("-10");

      if (pilotSourceAddress < 0) return; // No EV-1 detected. Return!
      tN2kMsg N2kMsg;
      RaymarinePilot::KeyCommand(N2kMsg, pilotSourceAddress, KEY_MINUS_10);
      NMEA2000.SendMsg(N2kMsg);
      BeepOn();
    }
  }
  BeepOff();
}


void loop() {
  Handle_AP_Remote();
  NMEA2000.ParseMessages();

  int SourceAddress = NMEA2000.GetN2kSource();
  if (SourceAddress != NodeAddress) { // Save potentially changed Source Address to NVS memory
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }

}
