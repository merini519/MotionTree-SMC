#include <SPI.h>
#include "df_can.h"
// "df_can.h" can be downloaded from below
// https://wiki.dfrobot.com/CAN-BUS_Shield_V2__SKU__DFR0370_

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);

unsigned char len = 0;
unsigned char buf[8];
char str[20];

int32_t current_position;
uint16_t loop_cycle = 0;
uint8_t flag = 0;

void setup()
{
  Serial.begin(115200);
  
  int count = 50;
  do {
    CAN.init();
    if (CAN_OK == CAN.begin(CAN_1000KBPS))                  // init can bus : baudrate = 1000k
    {
      break;
    }
    else
    {
      Serial.println("CAN BUS Shield init fail");
      delay(100);
    }

  } while (count--);

  attachInterrupt(0, MCP2515_ISR, FALLING);

  encoder_reset(0x001); // Encoder Reset
  current_position = 0;
  delay(100);
}

void loop()
{
  if (loop_cycle <= 0)
  {
    loop_cycle = 400;
    switch(flag)
    {
      case 0: mode2(0x001, 1, 20000, 2, 0.5);
              flag = 1;
              break;
      case 1: mode2(0x001, 1, 0, 2, 0.5);
              flag = 0;
              break;
    }
  }
  else
  {
    loop_cycle -= 1;
  }
  request_position(0x001);
  Serial.println(current_position);
  
  delay(10);
}

void encoder_reset(uint16_t can_id)
{
  uint8_t data[2];
  data[0] = 0xFD; // Header
  data[1] = 0x01; // Request
  
  CAN.sendMsgBuf(can_id, 0, 2, data);  
}

void mode2(uint16_t can_id, uint8_t ena, int32_t pos, float kp, float kd)
{
  uint8_t data[8];
  data[0] = 0x02; // Header
  data[1] = ena; // Enable
  data[2] = (uint8_t) (pos      ); // Position Target Lowest
  data[3] = (uint8_t) (pos >> 8 ); // Position Target Low
  data[4] = (uint8_t) (pos >> 16); // Position Target High
  data[5] = (uint8_t) (pos >> 24); // Position Target Highest
  data[6] = (uint8_t) (kp * 10); // P Gain
  data[7] = (uint8_t) (kd * 10); // D Gain 
  
  CAN.sendMsgBuf(can_id, 0, 8, data);
}

void request_position(uint16_t can_id)
{
  uint8_t data[2];
  data[0] = 0xFD; // Header
  data[1] = 0x00; // Request
  
  CAN.sendMsgBuf(can_id, 0, 2, data);  
}

void read_position(uint16_t can_id)
{
  if ((buf[0] == 0xFD) && (CAN.getCanId() == can_id + 0x100))
  {
    current_position = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
  } 
}

void MCP2515_ISR()
{
  while (CAN_MSGAVAIL == CAN.checkReceive())
  {
    buf[8] = {0, };
    CAN.readMsgBuf(&len, buf);
    read_position(0x001);
  }
}
