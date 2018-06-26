#include <Wire.h>
#include <EEPROM.h>

// GY-203 sensor commands
#define devAddr  0x40
#define tempReq  0xE3
#define humReq   0xE5
#define resetReq 0xFE

// dynamic memory allocation
byte humidity[2];
byte temperature[2];
byte light[2];
#define MaxR 3
#define MaxT 11
byte Received[MaxR];
byte Transmit[MaxT];
unsigned long int pT = 0;
int L = 0;
int i = 0;
int s = 0;

// config memory allocation
#define _SP 0

// pin definitions
#define LDR A0
#define LDW 10
#define EN  9
#define SET 8
#define AUX 2

// node info
const byte CentID[] = {0x00, 0x03};
const byte NodeID[] = {0x00, 0x03};
const byte CastID[] = {0xFF, 0xFF};

// constant chars
const byte tempFlag     = 0x54;       //char T
const byte humFlag      = 0x48;       //char H
const byte lightFlag    = 0x4C;       //char L
const byte measureFlag  = 0x4D;       //char M
const byte configFlag   = 0x43;       //char C

// configs
unsigned long int send_period = 120000;

void setup() {

  // initiate channels
  Serial.begin(9600);
  Wire.begin();

  // read last config
  if (EEPROM[_SP] != 0 && EEPROM[_SP + 1] != 0) {
    send_period = ((EEPROM[_SP] << 8) + EEPROM[_SP + 1]) * 1000;
  }

  // initiate LoRa modem
  pinMode(EN, OUTPUT);
  pinMode(SET, OUTPUT);
  digitalWrite(EN, HIGH);
  digitalWrite(SET, HIGH);

  // initiate LDR
  pinMode(LDW, OUTPUT);
  digitalWrite(LDW, LOW);

  //
  Serial.println(send_period);

  // first commit
  //measureSens();
  //notifyCenter();
}

void loop() {

  // if there is an UART activity
  i = 0;
  while (Serial.available() && i < MaxR) {
    Received[i] = (byte)Serial.read();
    i++;
  }
  if (i > 0) {
    switch (Received[0])
    {
      case measureFlag:
        measureSens();
        notifyCenter();
        break;
      case configFlag:
        updateConfig();
        break;
      default:
        break;
    }
  }
  if (millis() - pT > send_period) {
    measureSens();
    notifyCenter();
    pT = millis();
  }
}

void SI7021_Routine() {

  s = 0;

  // begin communication
  Wire.beginTransmission(devAddr);

  // ask temperature
  Wire.write(tempReq);
  Wire.requestFrom(devAddr, 2);

  // write temperature to the buffer
  s = 0;
  while (Wire.available()) {
    temperature[s] = Wire.read();
    s++;
  }
  Wire.endTransmission();
  Wire.beginTransmission(devAddr);

  // ask humidity
  Wire.write(humReq);
  Wire.requestFrom(devAddr, 2);

  // write humidity to the buffer
  s = 0;
  while (Wire.available()) {
    humidity[s] = Wire.read();
    s++;
  }
  // end communication
  Wire.endTransmission();
}

void LDR_Routine() {

  // let the current flow
  digitalWrite(LDW, HIGH);
  delay(200);
  // read the value
  L = analogRead(LDR);

  // write into the buffer
  light[0] = (byte)(L / 256);
  light[1] = (byte)(L % 256);

  // stop current flow
  digitalWrite(LDW, LOW);
}

void packMes() {

  // group buffers into one
  Transmit[0] = CentID[0]; Transmit[1] = CentID[1];
  Transmit[2] = tempFlag;  Transmit[3] = temperature[0]; Transmit[4] = temperature[1];
  Transmit[5] = humFlag;   Transmit[6] = humidity[0];    Transmit[7] = humidity[1];
  Transmit[8] = lightFlag; Transmit[9] = light[0];       Transmit[10] = light[1];
}

void notifyCenter() {

  // change to express communication
  digitalWrite(EN, LOW);  delay(10);
  digitalWrite(SET, LOW);

  // pack message
  packMes();

  // send message
  Serial.write(Transmit, MaxT);

  // change back to breath period communication
  digitalWrite(EN, HIGH); delay(10);
  digitalWrite(SET, HIGH);
}

void measureSens() {
  SI7021_Routine();
  delay(10);
  LDR_Routine();
  delay(10);
}

void updateConfig() {

  // write period configs into non-volatile memory
  if (Received[1] != 0x00 && Received[2] != 0x00) {
    EEPROM.write(_SP, Received[1]);
    EEPROM.write(_SP + 1, Received[2]);
    send_period = ((Received[2] << 8) + Received[3]) * 1000;
  }
}
