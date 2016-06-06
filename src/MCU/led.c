#include "MCU/led.h"

#define LED_PIN 7

void Led_On(void) {
  GPIOB->BSRR |= ((uint32_t)1 << LED_PIN);
}

void Led_Off(void) {
  GPIOB->BSRR |= ((uint32_t)1 << LED_PIN);
}

void Led_Toggle(void) {
  if (GPIOB->ODR & (1 << LED_PIN)) {
    Led_Off();
  }
  else {
    Led_On();
  }
}

void Led_Init(void) {
  RCC->AHB1ENR = ((uint32_t)1 << 1);

  GPIOB->MODER &= ~((uint32_t)0x3 << 14);
  GPIOB->MODER |= ((uint32_t)0x1 << 14);
  GPIOB->OTYPER &= ~((uint32_t)1 << 7);
  GPIOB->OSPEEDR |= ((uint32_t)0x3 << 14);
  GPIOB->PUPDR &= ~((uint32_t)0x3 << 14);

  Led_Off();
}
