// Installation instructions for the ESP32-Board
// https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

// Boards:
// esp32 by Espressif Systems@2.0.17

// Select "ESP32 Dev Module" as your board

// Libraries:
// esp32_can by collin80 > https://github.com/collin80/esp32_can
// can_common by collin80 > https://github.com/collin80/can_common
// ESP8266 Influxdb by Tobias Schürg@3.13.2
// TinyGPSPlus by Mikal Hart@1.0.3

// MrDIY - CAN Shield - https://youtu.be/gtVQeTRps7o
/************************  IMPORTANT NOTE ******************************
  For shields v1.0, v1.1, and v1.2, use D5 for CAN RX and D4 for CAN TX
  For shields v1.3 and later, use D4 for CAN RX and D5 for CAN TX*/
  

#include <esp32_can.h>

#define  SHIELD_LED_PIN 26

void setup()
{
  Serial.begin(115200);
  Serial.println("CAN INIT...");
  CAN0.setCANPins(GPIO_NUM_4, GPIO_NUM_5); //config for shield v1.3+, see important note above!
  CAN0.begin(500000); // 500Kbps
  CAN0.watchFor();
  Serial.println("SUCCES");
}


void loop()
{
  CAN_FRAME can_message;

  if (CAN0.read(can_message))
  {
    Serial.print("CAN MSG: 0x");
    Serial.print(can_message.id, HEX);
    Serial.print(" [");
    Serial.print(can_message.length, DEC);
    Serial.print("] <");
    for (int i = 0; i < can_message.length; i++)
    {
      if (i != 0) Serial.print(":");
      Serial.print(can_message.data.byte[i], HEX);
    }
    Serial.println(">");
  }
}