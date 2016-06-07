#include "MCU/led.h"

#define LED_PIN_BS  GPIO_BSRR_BS_7
#define LED_PIN_BR  GPIO_BSRR_BR_7
#define LED_PIN_ODR GPIO_ODR_ODR_7

void Led_On(void) {
  GPIOB->BSRR |= LED_PIN_BS;
}

void Led_Off(void) {
  GPIOB->BSRR |= LED_PIN_BR;
}

void Led_Toggle(void) {
  if (GPIOB->ODR & LED_PIN_ODR) {
    Led_Off();
    return;
  }

  Led_On();
}

void Led_Init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER   &= ~GPIO_MODER_MODER7;
  GPIOB->MODER   |= GPIO_MODER_MODER7_0;
  GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_7;
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;
  GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR7;

  Led_Off();
}
