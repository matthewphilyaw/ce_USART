/**
 * \file
 * \brief Entry point
 *
 * \author Matthew Philyaw (matthew.philyaw@gmail.com)
 */
#include "common.h"

#include <MCU/LED/green_led.h>
#include <MCU/LED/blue_led.h>
#include <MCU/LED/red_led.h>

#include "MCU/tick.h"
#include "MCU/usart3.h"

static void PrintHeader(void);

/**
 * \brief Main function
 */
int main(void) {
  Tick_Init();

  GreenLed.Init();
  RedLed.Init();
  BlueLed.Init();

  SerialPort3.Open(115200);

  PrintHeader();

  uint8_t chr = 0;
  for (;;) {
    int_fast8_t result = SerialPort3.GetByte(&chr);

    if (!result) {
      continue;
    }
    else if (result == SERIAL_FRAMING_ERROR){
      GreenLed.On();
      continue;
    }
    else if (result == SERIAL_OVERRUN) {
      BlueLed.On();
    }

    SerialPort3.SendByte(chr);
  }
}

/**
 * \brief Print serial header
 *
 * Prints a header out to the USART device that contains details about the firmware/hardware version
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
 * \brief override default fault handlers to blink red led
 */
void HardFault_Handler(void) {
  for(;;) {
    RedLed.Toggle();
    for (int i = 0; i < 1000000; i++);
  }
}

void UsageFault_Handler(void) {
  for(;;) {
    RedLed.Toggle();
    for (int i = 0; i < 100000; i++);
  }
}
