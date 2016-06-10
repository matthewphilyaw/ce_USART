/**
 * \file
 * \author Matthew Philyaw
 * \brief Led Driver
 *
 * Green Led on the Nucleo F446ZE board
 *
 */
#include <MCU/LED/blue_led.h>

/**
 * \brief GPIO defines for driving the output
 */
#define LED_PIN_BS  GPIO_BSRR_BS_0
#define LED_PIN_BR  GPIO_BSRR_BR_0
#define LED_PIN_ODR GPIO_ODR_ODR_0

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

  GPIOB->MODER   &= ~GPIO_MODER_MODER0;
  GPIOB->MODER   |= GPIO_MODER_MODER0_0;
  GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_0;
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0;
  GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR0;

  Led_Off();
}

/**
 * \brief Setup LedInterface Interface
 */
LedInterface GreenLed = {
    Led_On,
    Led_Off,
    Led_Toggle,
    Led_Init
};
