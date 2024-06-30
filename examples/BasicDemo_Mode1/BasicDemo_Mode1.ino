#include <SPI.h>
#include "df_can.h"
// "df_can.h" can be downloaded from below
// https://wiki.dfrobot.com/CAN-BUS_Shield_V2__SKU__DFR0370_

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);

unsigned char len = 0;
unsigned char buf[8];
char str[20];

uint16_t pps = 0;

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  int count = 50;
  do {
    CAN.init();
    if (CAN_OK == CAN.begin(CAN_1000KBPS))                  // init can bus : baudrate = 1000k
    {
      Serial.println("CAN BUS Shield init ok");
      break;
    }
    else
    {
      Serial.println("CAN BUS Shield init fail");
      delay(100);
    }

  } while (count--);

  attachInterrupt(0, MCP2515_ISR, FALLING);
}

void loop()
{
  
  mode1(0x001, 1, 0, pps);  // CAN_ID, ENABL복E, DIRECTION, PULSE PER SECOND
   
  (pps > 20000) ? pps = 0 : pps+=100; // increasing the rotation speed
  delay(100);
}

void mode1(uint16_t can_id, uint8_t ena, uint8_t dir, uint16_t pps)
{
  uint8_t data[5];
  data[0] = 0x01; // Header
  data[1] = ena; // Enable
  data[2] = dir; // Direction
  data[3] = (uint8_t) (pps     ); // Pulse Per Second Low
  data[4] = (uint8_t) (pps >> 8); // Pulse Per Second High
  
  CAN.sendMsgBuf(can_id, 0, 5, data);

  Serial.print(F("<< CAN Tx:  0x00")); Serial.print(can_id, HEX);
  hexPrint(data[0]);
  hexPrint(data[1]);
  hexPrint(data[2]);
  hexPrint(data[3]);
  hexPrint(data[4]);
  Serial.println();
}

void MCP2515_ISR()
{
  while (CAN_MSGAVAIL == CAN.checkReceive())
  {
    buf[8] = {0, };
    CAN.readMsgBuf(&len, buf);
    Serial.print(F(">> CAN Rx:  0x")); Serial.print(CAN.getCanId(), HEX);

    // print the data
    for (int i = 0; i < len; i++)
    {
      hexPrint(buf[i]);
    }
    Serial.println();
  }
}

void hexPrint(uint8_t data)
{
  if (data <= 15)
  {
    Serial.print(F("  0x0")); Serial.print(data, HEX);
  }
  else
  {
    Serial.print(F("  0x")); Serial.print(data, HEX);
  }
}
