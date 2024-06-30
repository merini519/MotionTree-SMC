#include <SPI.h>
#include "df_can.h"
// "df_can.h" can be downloaded from below
// https://wiki.dfrobot.com/CAN-BUS_Shield_V2__SKU__DFR0370_

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);

unsigned char len = 0;
unsigned char buf[8];
char str[20];

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
  mode0(0x001, 1, 0, 32000, 9600);  // CAN_ID, ENABL복E, DIRECTION, PULSE TARGET, PULSE PER SECOND
  delay(5000);

  mode0(0x001, 1, 1, 32000, 9600);  // CAN_ID, ENABLE, DIRECTION, PULSE TARGET, PULSE PER SECOND
  delay(5000);
}

void mode0(uint16_t can_id, uint8_t ena, uint8_t dir, uint32_t pulse_target, uint16_t pps)
{
  uint8_t data[8];
  data[0] = 0x00; // Mode 0
  data[1] = ena; // Enable
  data[2] = dir; // Direction
  data[3] = (uint8_t) (pulse_target      ); // Pulse Target Low
  data[4] = (uint8_t) (pulse_target >> 8 ); // Pulse Target Mid
  data[5] = (uint8_t) (pulse_target >> 16); // Pulse Target High
  data[6] = (uint8_t) (pps     ); // Pulse Per Second Low
  data[7] = (uint8_t) (pps >> 8); // Pulse Per Second High
  
  CAN.sendMsgBuf(can_id, 0, 8, data);

  Serial.print(F("<< CAN Tx:  0x00")); Serial.print(can_id, HEX);
  hexPrint(data[0]);
  hexPrint(data[1]);
  hexPrint(data[2]);
  hexPrint(data[3]);
  hexPrint(data[4]);
  hexPrint(data[5]);
  hexPrint(data[6]);
  hexPrint(data[7]);
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
