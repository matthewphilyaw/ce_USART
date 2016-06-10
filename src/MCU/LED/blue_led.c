/**
 * \file
 * \author Matthew Philyaw
 * \brief Led Driver
 *
 * Blue Led on the Nucleo F446ZE board
 *
 */
#include <MCU/LED/blue_led.h>

/**
 * \brief GPIO defines for driving the output
 */
#define LED_PIN_BS  GPIO_BSRR_BS_7
#define LED_PIN_BR  GPIO_BSRR_BR_7
#define LED_PIN_ODR GPIO_ODR_ODR_7

/**
 * \brief Turn Led on
 */
void static Led_On(void) {
  GPIOB->BSRR |= LED_PIN_BS;
}

/**
 * \brief Turn Led off
 */
void static Led_Off(void) {
  GPIOB->BSRR |= LED_PIN_BR;
}

/**
 * \brief Toggle Led
 */
void static Led_Toggle(void) {
  if (GPIOB->ODR & LED_PIN_ODR) {
    Led_Off();
    return;
  }

  Led_On();
}

/**
 * \brief Init Led
 *
 * This will be specific to the Led in use
 */
void static Led_Init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER   &= ~GPIO_MODER_MODER7;
  GPIOB->MODER   |= GPIO_MODER_MODER7_0;
  GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_7;
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;
  GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR7;

  Led_Off();
}

/**
 * \brief Setup LedInterface Interface
 */
LedInterface BlueLed = {
    Led_On,
    Led_Off,
    Led_Toggle,
    Led_Init
};
