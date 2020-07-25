/*
  Code to program 433 MHz KeyFob

  Transmitter must be connected to ESP32 Pin #15
  Receiver must be connected to ESP32 Pin #19

*/

#include <RCSwitch.h>

unsigned long Key[] = {1000001, 1000002, 1000003, 1000004 };  // Change to individual/random values!
int Keys = sizeof(Key) / sizeof(unsigned long); // Number of keys is calculated atomatically

RCSwitch mySwitch = RCSwitch();


void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("Press 'Send' to start programming or press a key on remote to see the value.");


  mySwitch.enableTransmit(19); // Transmitter is connected to ESP32 GPIO 19
  mySwitch.enableReceive(15);  // RF Receiver is connected to ESP32 GPIO 15

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);

  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);

  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);

}

void loop() {
  int i;

  while (!Serial.available()) {    // Receive Key Codes until Send was pressed

    if (mySwitch.available()) {
      long unsigned int value = mySwitch.getReceivedValue();
      Serial.print("Received Key: ");
      Serial.println(value);
      mySwitch.resetAvailable();
    }
  }
  while (Serial.available()) Serial.read();

  Serial.println();

  for (i = 0; i < Keys; i++) {
    Serial.print("Sending Key: ");
    Serial.print(i + 1);
    Serial.print(", Value: ");
    Serial.println(Key[i]);
    Serial.println("Press 'Send' for next Key.");

    while (!Serial.available()) {
      mySwitch.send(Key[i], 24);
      Serial.print(".");
      delay(100);
    }
    Serial.println();

    while (Serial.available()) Serial.read();
  }
  Serial.println("Press 'Send' to start programming or press a key on remote to see the value.");
}
