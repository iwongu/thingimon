/*
Measurement of relative humidity and temperature using HTS221
*/

#include <SoftwareSerial.h>
#include <Wire.h>

#define ADDRESS_HTS221 0x5F
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define STATUS_REG 0x27
#define HUMIDITY_OUT_L_REG 0x28
#define T0_degC_x8 0x32
#define T0_OUT 0x3C
#define H0_rH_x2 0x30
#define H0_T0_OUT 0x36
#define H1_T0_OUT 0x3A

byte buffer[] = {0, 0, 0, 0};
byte status = 0;

float t0, t1, h0, h1;
int t0out, t1out, h0out, h1out;

int outHumi = 0;
int outTemp = 0;
float valueHumi = 0;
float valueTemp = 0;

SoftwareSerial XBee(2, 3);

void setup()
{
    Wire.begin();
    Serial.begin(9600);
    XBee.begin(9600);

    //turn on the HTS221, set the update mode to one shot
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(CTRL_REG1);
    Wire.write(0x84);
    Wire.endTransmission();

    ReadCaliData();

    //print the calibration coefficients
    Serial.println("Calibration coefficients: ");
    Serial.print("t0 = "); Serial.print(t0); Serial.print(" `C, t0out = "); Serial.println(t0out);
    Serial.print("t1 = "); Serial.print(t1); Serial.print(" `C, t1out = "); Serial.println(t1out);
    Serial.print("h0 = "); Serial.print(h0); Serial.print(" \%RH, h0out = "); Serial.println(h0out);
    Serial.print("h1 = "); Serial.print(h1); Serial.print(" \%RH, h1out = "); Serial.println(h1out);
    Serial.println("------------------------");
}

void loop()
{
    if (XBee.available()) {
      int c = XBee.read();
      if (0xDE != c) {
        Serial.print("received: "); Serial.println(c, HEX);
        return;
      }
    } else {
      return;
    }
    //perform a measurement
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(CTRL_REG2);
    Wire.write(0x01);
    Wire.endTransmission();

    //check the status
    status = 0;
    while (status != 0x03) //typical conversition time: 3ms
    {
        delayMicroseconds(3000);
        Wire.beginTransmission(ADDRESS_HTS221);
        Wire.write(STATUS_REG);
        Wire.endTransmission();

        Wire.requestFrom(ADDRESS_HTS221, 1);
        if(Wire.available() >= 1)
        {
            status = Wire.read(); 
        }
        delayMicroseconds(500);
        // Serial.println(status, HEX);
    }

    //read multiple bytes incrementing the register address
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(HUMIDITY_OUT_L_REG | 0x80);
    Wire.endTransmission();

    Wire.requestFrom(ADDRESS_HTS221, 4);
    if (Wire.available() >= 4)
    {
        for (byte i = 0; i < 4; i++)
        {
            buffer[i] = Wire.read();
        }
    }

    outHumi = (buffer[1] << 8) | buffer[0];
    outTemp = (buffer[3] << 8) | buffer[2];

    valueTemp = mapFloat(outTemp, t0out, t1out, t0, t1);
    valueHumi = mapFloat(outHumi, h0out, h1out, h0, h1);
    Serial.print(valueTemp); Serial.print(" `C, ");
    Serial.print(valueHumi); Serial.println(" \%RH");

    writeXBee(0xDEAD);
    writeXBee(0xBEAF);
    writeXBee(valueTemp * 100);
    writeXBee(valueHumi * 100);
}

void writeXBee(int value) {
  XBee.write((value >> 8) & 0xFF);
  XBee.write(value & 0xFF);
}

void ReadCaliData()
{
    //read out t0degCx8, t1degCx8
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(T0_degC_x8 | 0x80);
    Wire.endTransmission();
    Wire.requestFrom(ADDRESS_HTS221, 4);
    if (Wire.available() >= 4)
    {
        for (byte i = 0; i < 4; i++)
        {
            buffer[i] = Wire.read();
        }
    }
    word t0degCx8 = ((buffer[3] & 0b00000011) << 8) | buffer[0];
    word t1degCx8 = ((buffer[3] & 0b00001100) << 6) | buffer[1];
    t0 = t0degCx8/8.0;
    t1 = t1degCx8/8.0;

    //read out t0out, t1out
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(T0_OUT | 0x80);
    Wire.endTransmission();
    Wire.requestFrom(ADDRESS_HTS221, 4);
    if (Wire.available() >= 4)
    {
        for (byte i = 0; i < 4; i++)
        {
            buffer[i] = Wire.read();
        }
    }
    t0out = (buffer[1] << 8) | buffer[0];
    t1out = (buffer[3] << 8) | buffer[2];

    //read out h0RHx2, h1RHx2
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(H0_rH_x2 | 0x80);
    Wire.endTransmission();

    Wire.requestFrom(ADDRESS_HTS221, 2);
    if (Wire.available() >= 2)
    {
        for (byte i = 0; i < 2; i++)
        {
            buffer[i] = Wire.read();
        }
    }
    byte h0RHx2 = buffer[0];
    byte h1RHx2 = buffer[1];
    h0 = h0RHx2/2.0;
    h1 = h1RHx2/2.0;

    //read out h0t0Out
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(H0_T0_OUT | 0x80);
    Wire.endTransmission();
    Wire.requestFrom(ADDRESS_HTS221, 2);
    if (Wire.available() >= 2)
    {
        for (byte i = 0; i < 2; i++)
        {
            buffer[i] = Wire.read();
        }
    }
    h0out = (buffer[1] << 8) | buffer[0];

    //read out h1t0Out
    Wire.beginTransmission(ADDRESS_HTS221);
    Wire.write(H1_T0_OUT | 0x80);
    Wire.endTransmission();
    Wire.requestFrom(ADDRESS_HTS221, 2);
    if (Wire.available() >= 2)
    {
        for (byte i = 0; i < 2; i++)
        {
            buffer[i] = Wire.read();
        }
    }
    h1out = (buffer[1] << 8) | buffer[0];
}

float mapFloat(int x, int in_min, int in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
