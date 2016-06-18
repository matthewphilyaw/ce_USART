#include <MCU/LED/green_led.h>
#include <MCU/LED/blue_led.h>
#include <MCU/LED/red_led.h>
#include "MCU/tick.h"
#include "MCU/usart3.h"

/**
 * @brief Sets the baudrate for the serial port
 */
#define BAUDRATE 115200

static void PrintHeader(void);
void HardFault_Handler(void);

/**
 * @brief Main function for USART echo server
 */
void main(void) {
  Tick_Init();

  GreenLed.Init();
  RedLed.Init();
  BlueLed.Init();

  SerialPort3.Open(BAUDRATE);

  PrintHeader();

  uint8_t buf[USART_MAX_BUFFER];
  uint32_t numRead = 0;
  for (;;) {
    numRead = SerialPort3.GetByte(buf, USART_MAX_BUFFER);
    if (numRead > 0) {
      SerialPort3.SendArray(buf, numRead);
    }
    Tick_DelayMs(500);
  }
}

/**
 * @brief Print info header
 *
 * This function prints a info head to the serial device that contains the
 * firmware and hardware versions as well as last compile date.
 */
static void PrintHeader() {
  SerialPort3.SendString("\e[2J");
  SerialPort3.SendString("#################################################################\r");
  SerialPort3.SendString("    Firmware Version: ");
  SerialPort3.SendString(FIRMWARE_VERSION);
  SerialPort3.SendString("\r");
  SerialPort3.SendString("    Hardware Version: ");
  SerialPort3.SendString(HARDWARE_VERSION);
  SerialPort3.SendString("\r");
  SerialPort3.SendString("    Build Date: ");
  SerialPort3.SendString(COMPILED_DATA_TIME);
  SerialPort3.SendString("\r");
  SerialPort3.SendString("#################################################################\r\r");
}

/**
 * @brief Provides a implementation for the CMSIS hard fault handler
 */
void HardFault_Handler(void) {
  for(;;) {
    RedLed.Toggle();
    for (int i = 0; i < 1000000; i++);
  }
}

