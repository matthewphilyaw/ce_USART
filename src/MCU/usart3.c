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

static uint_fast8_t IsOpenFlag = FALSE;

static uint_fast8_t IsOpen(void) {
  return IsOpenFlag;
}

static void Close(void) {
  USART3->CR1 &= ~(1);
}

static uint_fast8_t IsWriteBusy(void) {
  return !(USART3->SR & USART_SR_TXE);
}

static uint_fast8_t RxBufferHasData(void) {
  return USART3->SR & USART_SR_RXNE;
}

static SerialResult_t Open(uint32_t baudrate) {
  if (IsOpenFlag) {
    return SERIAL_FAIL;
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
  return SERIAL_SUCCESS;
}

static SerialResult_t SendByte(uint8_t source) {
  if (!IsOpenFlag)
  {
    return SERIAL_CLOSED;
  }

  while (IsWriteBusy());

  USART3->DR = source;

  return SERIAL_SUCCESS;
}

static SerialResult_t GetByte(uint8_t *destination) {
  if (!IsOpenFlag)
  {
    return SERIAL_CLOSED;
  }

  if (!destination) {
    return SERIAL_INVALID_PARAMETER;
  }

  uint_fast8_t hasData = RxBufferHasData();
  /// \brief To clear error bits read status register and then the data register
  uint32_t sr = USART3->SR;
  *destination = USART3->DR;

  if (!hasData)
  {
    return SERIAL_NO_DATA;
  }

  /// \brief check various error conditions and return the correct error
  if (sr & USART_SR_ORE) {
    return SERIAL_OVER_RUN;
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

  return SERIAL_SUCCESS;
}

static SerialResult_t SendString(const char *source) {
  if (!IsOpenFlag) {
    return SERIAL_CLOSED;
  }

  if (!source) {
    return SERIAL_INVALID_PARAMETER;
  }

  while(*source) {
    if (SendByte(*source)) {
      return SERIAL_FAIL;
    }
    source++;
  }

  return SERIAL_SUCCESS;
}

static SerialResult_t SendArray(const uint8_t *source, uint32_t length) {
  if (!IsOpenFlag) {
    return SERIAL_CLOSED;
  }

  if (!source) {
    return SERIAL_INVALID_PARAMETER;
  }
  
  for ( ; length; length--) {
    if (SendByte(*source)) {
      return SERIAL_FAIL;
    }
    source++;
  }

  return SERIAL_SUCCESS;
}

SerialInterface SerialPort3 = {
  IsOpen,
  RxBufferHasData,
  Open,
  Close,
  SendByte,
  SendString,
  SendArray,
  GetByte
};
