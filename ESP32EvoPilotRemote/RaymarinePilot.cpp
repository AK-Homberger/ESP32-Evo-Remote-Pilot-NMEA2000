#include "RaymarinePilot.h"
#include <string.h>

double RaymarinePilot::Heading = 0;
double RaymarinePilot::Variation = 0;

//destinationAddress muss aus der tN2kDeviceList ausgelesen werden. Beispiel dazu: DeviceAnalyzer.ino

void RaymarinePilot::SetEvoPilotMode(tN2kMsg &N2kMsg, uint8_t destinationAddress, RaymarinePilotModes mode) {
  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(1); // Field 1, 1 = Command Message, 2 = Acknowledge Message...
  N2kMsg.AddByte(0x63);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0xf8);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0x3b);
  N2kMsg.AddByte(0x07);
  N2kMsg.AddByte(0x03);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x04);

  // 0x00 = standby, 0x40 = auto, 0x0100=vane, 0x0180=track
  switch (mode) {
    case PILOT_MODE_STANDBY:
      N2kMsg.AddByte(0x00);
      N2kMsg.AddByte(0x00);
      break;
    case PILOT_MODE_AUTO:
      N2kMsg.AddByte(0x40);
      N2kMsg.AddByte(0x00);
      break;
    case PILOT_MODE_WIND:
      N2kMsg.AddByte(0x00);
      N2kMsg.AddByte(0x01);
      break;
    case PILOT_MODE_TRACK:
      N2kMsg.AddByte(0x80);
      N2kMsg.AddByte(0x01);
      break;
  }

  N2kMsg.AddByte(0x05);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);

  /*N2kMsg.Add3ByteInt(126208UL); // Field 2, commanded PGN 3 byte,  LSB is transmitted first
    N2kMsg.AddByte(0xf0 | 0x8 ); // Field 3, 0x8 = Do not change priority and field 4 is nmea reserved
    N2kMsg.AddByte(1); // Field 5, 1 field/value pair
    N2kMsg.AddByte(4); // Field 6, field 4 to be commanded
    N2kMsg.Add4ByteUInt(0); // Field 7, value for first pair =0 for reset distance log.*/
}

void RaymarinePilot::SetEvoPilotCourse(tN2kMsg &N2kMsg, uint8_t destinationAddress, double heading, int change) {
  double course = heading + change;
  if ((course) >= 360) {
    course -= 360;
  } else if ((course) < 0) {
    course += 360;
  }

  uint16_t courseRadials10000 = (uint16_t) (DegToRad(course) * 10000); //(newCourse * 174.53);


  byte byte0, byte1;
  byte0 = courseRadials10000 & 0xff;
  byte1 = courseRadials10000 >> 8;

  //01,50,ff,00,f8,03,01,3b,07,03,04,06,00,00

  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(1); // Field 1, 1 = Command Message, 2 = Acknowledge Message...
  N2kMsg.AddByte(0x50);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0xf8);
  N2kMsg.AddByte(0x03);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0x3b);
  N2kMsg.AddByte(0x07);
  N2kMsg.AddByte(0x03);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x06);
  N2kMsg.AddByte(byte0);
  N2kMsg.AddByte(byte1);
}

void RaymarinePilot::TurnToWaypointMode(tN2kMsg &N2kMsg, uint8_t destinationAddress) {
  //"01,63,ff,00,f8,04,01,3b,07,03,04,04,81,01,05,ff,ff"
  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(0x01); // Field 1, 1 = Command Message, 2 = Acknowledge Message...
  N2kMsg.AddByte(0x63);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0xf8);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0x3b);
  N2kMsg.AddByte(0x07);
  N2kMsg.AddByte(0x03);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x81);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0x05);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
}

void RaymarinePilot::TurnToWaypoint(tN2kMsg &N2kMsg, uint8_t destinationAddress) {
  //00,00,ef,01,ff,ff,ff,ff,ff,ff,04,01,3b,07,03,04,04,6c,05,1a,50"

  N2kMsg.SetPGN(126208UL);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0xef);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x01);
  N2kMsg.AddByte(0x3b);
  N2kMsg.AddByte(0x07);
  N2kMsg.AddByte(0x03);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x04);
  N2kMsg.AddByte(0x6c);
  N2kMsg.AddByte(0x05);
  N2kMsg.AddByte(0x1a);
  N2kMsg.AddByte(0x50);
}

void RaymarinePilot::KeyCommand(tN2kMsg &N2kMsg, uint8_t destinationAddress, uint16_t command) {
  //const key_command = "3b,9f,f0,81,86,21,%s,%s,ff,ff,ff,ff,ff,c1,c2,cd,66,80,d3,42,b1,c8"

  byte commandByte0, commandByte1;
  commandByte0 = command >> 8;
  commandByte1 = command & 0xff;

  N2kMsg.SetPGN(126720UL);
  N2kMsg.Priority = 7;
  N2kMsg.Destination = destinationAddress;

  N2kMsg.AddByte(0x3b);
  N2kMsg.AddByte(0x9f);
  N2kMsg.AddByte(0xf0);
  N2kMsg.AddByte(0x81);
  N2kMsg.AddByte(0x86);
  N2kMsg.AddByte(0x21);
  //N2kMsg.Add2ByteUInt(command);
  N2kMsg.AddByte(commandByte0);
  N2kMsg.AddByte(commandByte1);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xff);
  N2kMsg.AddByte(0xc1);
  N2kMsg.AddByte(0xc2);
  N2kMsg.AddByte(0xcd);
  N2kMsg.AddByte(0x66);
  N2kMsg.AddByte(0x80);
  N2kMsg.AddByte(0xd3);
  N2kMsg.AddByte(0x42);
  N2kMsg.AddByte(0xb1);
  N2kMsg.AddByte(0xc8);
}

void RaymarinePilot::HandleNMEA2000Msg(const tN2kMsg &N2kMsg) {
  if (N2kMsg.PGN == 127250L) { //Heading
    unsigned char SID;
    tN2kHeadingReference ref;
    double _Deviation = 0;
    double _Variation;
    double _HeadingRad;

    if (ParseN2kHeading(N2kMsg, SID, _HeadingRad, _Deviation, _Variation, ref)) {
      if ( !N2kIsNA(_Variation) ) Variation = _Variation; // Update Variation
      if ( !N2kIsNA(_HeadingRad) && !N2kIsNA(Variation) ) _HeadingRad -= Variation;
      Heading = RadToDeg(_HeadingRad);
    }
  }
  else if (N2kMsg.PGN == 65288L) { //Alarm
    unsigned char AlarmCode;
    unsigned char AlarmGroup;

    if (ParseN2kAlarm(N2kMsg, AlarmCode, AlarmGroup)) {
      Serial.print("Alarm ");
      Serial.print(AlarmCode, HEX);
      Serial.println(AlarmGroup, HEX);

      if (AlarmCode == 0x1d && AlarmGroup == 0x01) {
        Serial.println("Alarm Waypoint");
      }
    }
  }

  else if (N2kMsg.PGN == 65379L) { //Pilot State
    unsigned char Mode;
    unsigned char Submode;

    if (ParseN2kPiloteState(N2kMsg, Mode, Submode)) {
      Serial.print("Mode / Submode ");
      Serial.print(Mode, HEX);
      Serial.print(" ");
      Serial.println(Submode, HEX);

      if (Mode == 0x00 && Submode == 0x00) {
        //RaymarinePilot::PilotMode = PILOT_MODE_STANDBY;
        Serial.println("PILOT_MODE_STANDBY");
      }
      else if (Mode == 0x40 && Submode == 0x00) {
        //RaymarinePilot::PilotMode = PILOT_MODE_AUTO;
        Serial.println("PILOT_MODE_AUTO");
      }

      else if (Mode == 0x00 && Submode == 0x01) {
        //RaymarinePilot::PilotMode = PILOT_MODE_WIND;
        Serial.println("PILOT_MODE_WIND");
      }

      else if (Mode == 0x81 && Submode == 0x01) {
        //RaymarinePilot::PilotMode = PILOT_MODE_TRACK;
        Serial.println("PILOT_MODE_TRACK");
      }

      else if (Mode == 0x80 && Submode == 0x01) {
        //RaymarinePilot::PilotMode = PILOT_MODE_TRACK;
        Serial.println("PILOT_MODE_TRACK");
      }

    }
  }
}


// Read Seatalk Alarm State
bool RaymarinePilot::ParseN2kPGN65288(const tN2kMsg &N2kMsg, unsigned char &AlarmCode, unsigned char &AlarmGroup) {
  if (N2kMsg.PGN != 65288L) return false;

  int Index = 4;

  AlarmCode = N2kMsg.GetByte(Index);
  AlarmGroup = N2kMsg.GetByte(Index);

  return true;
}


// Set Seatalk Alarm State
void RaymarinePilot::SetN2kPGN65288(tN2kMsg &N2kMsg, uint8_t destinationAddress, unsigned char AlarmState, unsigned char AlarmCode, unsigned char AlarmGroup) {

  N2kMsg.SetPGN(65288L);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(0xe7);  // Binary; 011100111011 00 100
  N2kMsg.AddByte(0x64);
  N2kMsg.AddByte(0x00);  // SID
  N2kMsg.AddByte(AlarmState); // Alarm State
  N2kMsg.AddByte(AlarmCode);  // Alarm ID
  N2kMsg.AddByte(AlarmGroup); // Alarm Group
  N2kMsg.AddByte(0x00);  // Alarm Priority (0)
  N2kMsg.AddByte(0x01);  // Alarm Priority (1)
}



// Seatalk Silence Alarm
void RaymarinePilot::SetN2kPGN65361(tN2kMsg &N2kMsg, uint8_t destinationAddress, unsigned char AlarmCode, unsigned char AlarmGroup) {

  N2kMsg.SetPGN(65361L);
  N2kMsg.Priority = 3;
  N2kMsg.Destination = destinationAddress;
  N2kMsg.AddByte(0xe7);  // Binary; 011100111011 00 100
  N2kMsg.AddByte(0x64);
  N2kMsg.AddByte(AlarmCode);  // Alarm ID
  N2kMsg.AddByte(AlarmGroup); // Alarm Group
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0x00);
  N2kMsg.AddByte(0x00);
}



bool RaymarinePilot::ParseN2kPGN65379(const tN2kMsg &N2kMsg, unsigned char &Mode, unsigned char &Submode) {
  Serial.println(N2kMsg.PGN);
  if (N2kMsg.PGN != 65379L) return false;

  int Index = 2;

  Mode = N2kMsg.GetByte(Index);
  Submode = N2kMsg.GetByte(Index);

  return true;
}
