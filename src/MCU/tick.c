#include<MCU/tick.h>

#define TIMER_FREQUENCY_HZ 1000

static volatile uint32_t TickCounter;

void Tick_Init(void) {
  SysTick_Config(SystemCoreClock / TIMER_FREQUENCY_HZ);
}

uint32_t Tick_GetMs(void) {
  return TickCounter;
}

void Tick_DelayMs(uint32_t delayMs) {
  uint32_t startTickValue = TickCounter;

  // loop until tick counter is >= start tick
  while((TickCounter - startTickValue) < delayMs);
}

int_fast8_t Tick_DelayMs_NonBlocking(uint_fast8_t reset, TickType *config) {
  uint32_t lapsedTick = 0;

  if (!config) {
    return -1; // can't use null pointer
  }

  if (reset) {
    config->StartMs = Tick_GetMs();
    return FALSE;
  }

  lapsedTick = (Tick_GetMs() - config->StartMs);

  if (lapsedTick < config->DelayMs) {
    return FALSE;
  }

  return TRUE;
}

void SysTick_Handler(void)
{
    TickCounter++;
}
