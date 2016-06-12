/**
 * \file
 * \brief Led Driver
 *
 * Red Led on the Nucleo F446ZE board
 *
 * \author Matthew Philyaw
 */
#include <MCU/LED/blue_led.h>

/**
 * \brief GPIO defines for driving the output
 */
#define LED_PIN_BS  GPIO_BSRR_BS_14
#define LED_PIN_BR  GPIO_BSRR_BR_14
#define LED_PIN_ODR GPIO_ODR_ODR_14

/**
 * \brief Turn Led on
 */
static void Led_On(void) {
  GPIOB->BSRR |= LED_PIN_BS;
}

/**
 * \brief Turn Led off
 */
static void Led_Off(void) {
  GPIOB->BSRR |= LED_PIN_BR;
}

/**
 * \brief Toggle Led
 */
static void Led_Toggle(void) {
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
static void Led_Init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER   &= ~GPIO_MODER_MODER14;
  GPIOB->MODER   |= GPIO_MODER_MODER14_0;
  GPIOB->OTYPER  &= ~GPIO_OTYPER_OT_14;
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR14;
  GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR14;

  Led_Off();
}

/**
 * \brief Setup LedInterface Interface
 */
LedInterface RedLed = {
    Led_On,
    Led_Off,
    Led_Toggle,
    Led_Init
};
