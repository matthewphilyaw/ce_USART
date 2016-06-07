#include "common.h"
#include "MCU/led.h"
#include "MCU/tick.h"

int main(void) {
  Tick_Init();
  Led_Init();

  for (;;) {
    Led_Toggle();
    Tick_DelayMs(2000);
  }
}
