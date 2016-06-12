/**
 * \file
 *
 * \brief USART driver
 * \author Matthew Philyaw (matthew.philyaw@gmail.com)
 */
#include "MCU/usart3.h"

/**
 * \brief Taken from the STM32 HAL
 *
 * Set of macros to calculate fractional baud rate which differs for this M4 part compared to the M0 part in the course
 */
#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)            (((_PCLK_)*25U)/(4U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)        (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)        (((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100U)) * 16U + 50U) / 100U)

/* UART BRR = mantissa + overflow + fraction = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU) */
#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4U) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0FU))

#define UART_DIV_SAMPLING8(_PCLK_, _BAUD_)             (((_PCLK_)*25U)/(2U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING8(_PCLK_, _BAUD_)         (UART_DIV_SAMPLING8((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING8(_PCLK_, _BAUD_)         (((UART_DIV_SAMPLING8((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) * 100U)) * 8U + 50U) / 100U)

/* UART BRR = mantissa + overflow + fraction = (UART DIVMANT << 4) + ((UART DIVFRAQ & 0xF8) << 1) + (UART DIVFRAQ & 0x07U) */
#define UART_BRR_SAMPLING8(_PCLK_, _BAUD_)             (((UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) << 4U) + \
                                                        ((UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0xF8U) << 1U)) + \
                                                        (UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0x07U))

/**
 * \brief Flag to indicate the state of the USART
 */
static uint_fast8_t IsOpenFlag = FALSE;

/**
 * \brief Exposes IsOpenFlag
 */
static uint_fast8_t IsOpen(void) {
  return IsOpenFlag;
}

/**
 * \brief Checks to see if USART is writing
 */
static uint_fast8_t IsWriteBusy(void) {
  return !(USART3->SR & USART_SR_TXE);
}

/**
 * \brief Enables and initializes the USART
 */
static uint_fast8_t Open(uint32_t baudrate) {
  if (IsOpenFlag) {
    return FALSE;
  }

  // enable GPIO TX pin
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  GPIOD->MODER &= ~GPIO_MODER_MODER8 | GPIO_MODER_MODER9;
  GPIOD->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
  GPIOD->AFR[1] &= ~(0x0F | (0x0F << 4));
  GPIOD->AFR[1] |= (0x07 | (0x07 << 4));


  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
  USART3->CR1 = 0;
  USART3->CR1 &= ~USART_CR1_M;    // 8 bit data
  USART3->CR2 &= ~USART_CR2_STOP; // 1 stop bit

#ifdef USART_OVER_SAMPLE_16
  USART3->CR1 &= ~USART_CR1_OVER8;
  USART3->BRR = UART_BRR_SAMPLING16(SystemCoreClock, baudrate);
#else
  USART3->CR1 |= USART_CR1_OVER8;
  USART3->BRR = UART_BRR_SAMPLING8(SystemCoreClock, baudrate);
#endif

  USART3->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

  IsOpenFlag = TRUE;
  return TRUE;
}

/**
 * \brief Close USART
 *
 * Disables the device
 */
static void Close(void) {
  USART3->CR1 &= ~(1);
}

/**
 * \brief Send one byte over the USART
 * \param source byte to send
 *
 * \return TRUE = success otherwise port is closed or other error
 */
static uint_fast8_t SendByte(uint8_t source) {
  if (!IsOpenFlag)
  {
    return FALSE;
  }

  while (IsWriteBusy());

  USART3->DR = source;

  return TRUE;
}

/**
 * \brief Checks to see if receive buffer has data
 * \return  1 = have data
 *          0 = no data
 *         -1 = port closed
 */
static int_fast8_t RxBufferHasData(void) {
  if (!IsOpenFlag) {
    return -1;
  }

  return USART3->SR & USART_SR_RXNE;
}

/**
 * \brief Checks to see if receive buffer has data
 * \return  1 = success
 *          0 = no data
 *         -1 = port closed
 */
static int_fast8_t GetByte(uint8_t *destination) {
  if (!IsOpenFlag || !destination) {
    return -1;
  }

  int_fast8_t has_data = RxBufferHasData();
  if (!has_data)
  {
    return FALSE;
  }
  else if (has_data < 0) ///< error
  {
    return -1;
  }

  /// \brief To clear error bits read status register and then the data register
  uint32_t sr = USART3->SR;
  *destination = USART3->DR;

  /// \brief check various error conditions and return the correct error
  if (sr & USART_SR_ORE) {
    return SERIAL_OVERRUN;
  }
  else if (sr & USART_SR_FE) {
    return SERIAL_FRAMING_ERROR;
  }
  else if (sr & USART_SR_PE) {
    return SERIAL_PARITY_ERROR;
  }
  else if (sr & USART_SR_LBD) {
    return SERIAL_LINE_BREAK_ERROR;
  }
  else if (sr & USART_SR_NE) {
    return SERIAL_NOISE_ERROR;
  }

  return TRUE;
}

/**
 * \brief Send string over the USART
 * \param source pointer to array holding string
 *
 * \return TRUE = success else port may be closed or invalid pointer
 */
static uint_fast8_t SendString(const char *source) {
  if (!IsOpenFlag || !source) {
    return FALSE;
  }  

  while(*source) {
    if (!SendByte(*source)) {
      return FALSE;
    }
    source++;
  }

  return TRUE;
}

/**
 * \brief Send byte array over USART
 * \param source pointer to byte array
 * \param length of byte array
 *
 * \return TRUE = success else port may not be open or invalid pointer
 */
static uint_fast8_t SendArray(const uint8_t *source, uint32_t length) {
  if (!IsOpenFlag || !source) {
    return FALSE;
  }  
  
  for ( ; length; length--) {
    if (!SendByte(*source)) {
      return FALSE;
    }
    source++;
  }

  return TRUE;
}

/**
 * \brief Setup SerialInterface struct
 */
SerialInterface SerialPort3 = {
  IsOpen,
  Open,
  Close,
  SendByte,
  SendString,
  SendArray,
  RxBufferHasData,
  GetByte
};
