#include "common.h"
#include "MCU/led.h"
#include "MCU/tick.h"
#include "MCU/usart3.h"

int main(void) {
  Tick_Init();
  Led_Init();

  SerialPort3.Open(115200);
  SerialPort3.SendString("\e[2J");
  SerialPort3.SendString("#################################################################\r\n");
  SerialPort3.SendString("    Firmware Version: ");
  SerialPort3.SendString(FIRMWARE_VERSION);
  SerialPort3.SendString("\r\n");
  SerialPort3.SendString("    Hardware Version: ");
  SerialPort3.SendString(HARDWARE_VERSION);
  SerialPort3.SendString("\r\n");
  SerialPort3.SendString("    Build Date: ");
  SerialPort3.SendString(COMPILED_DATA_TIME);
  SerialPort3.SendString("\r\n");
  SerialPort3.SendString("#################################################################\r\n");

  SerialPort3.SendString("\r\n");

  for (;;) {
    Tick_DelayMs(1000);
    SerialPort3.SendString("    ping\r\n");
  }
}
