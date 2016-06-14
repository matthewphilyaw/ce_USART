/**
 * @file usart3.c
 * @author Matthew Philyaw (matthew.philyaw@gmail.com)
 *
 * @brief USART Driver implementation for SerialPort3
 */
#include "MCU/usart3.h"

#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)            (((_PCLK_)*25U)/(4U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)        (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)        (((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100U)) * 16U + 50U) / 100U)


#define UART_DIV_SAMPLING8(_PCLK_, _BAUD_)             (((_PCLK_)*25U)/(2U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING8(_PCLK_, _BAUD_)         (UART_DIV_SAMPLING8((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING8(_PCLK_, _BAUD_)         (((UART_DIV_SAMPLING8((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) * 100U)) * 8U + 50U) / 100U)

/**
 * @brief These macros set that fractional buad rate this has been taken from the STM provided HAL
 */
/* UART BRR = mantissa + overflow + fraction = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU) */
#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4U) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0FU))

/**
 * @brief These macros set that fractional buad rate this has been taken from the STM provided HAL
 */
/* UART BRR = mantissa + overflow + fraction = (UART DIVMANT << 4) + ((UART DIVFRAQ & 0xF8) << 1) + (UART DIVFRAQ & 0x07U) */
#define UART_BRR_SAMPLING8(_PCLK_, _BAUD_)             (((UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) << 4U) + \
                                                        ((UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0xF8U) << 1U)) + \
                                                        (UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0x07U))
/**
 * @brief Internal flag for open status
 */
static uint_fast8_t IsOpenFlag = FALSE;

/**
 * @brief Return the status of the Interface
 *
 * @return
 * 1 -> Open\n
 * 0 -> Closed
 */
static uint_fast8_t IsOpen(void) {
  return IsOpenFlag;
}

/**
 * @brief Closes open interface
 */
static void Close(void) {
  USART3->CR1 &= ~(1);
}

/**
 * @brief Checks if write is in progress
 *
 * @return
 * 1 -> Write in progress\n
 * 0 -> Interface ready
 */
static uint_fast8_t IsWriteBusy(void) {
  return !(USART3->SR & USART_SR_TXE);
}

/**
 * @brief Checks if there is data to read
 *
 * @return
 * 1 -> Data to read\n
 * 0 -> No data to read
 */
static uint_fast8_t RxBufferHasData(void) {
  return USART3->SR & USART_SR_RXNE;
}

/**
 * @brief Open serial interface with specified baudrate
 * @param baudrate is the desired baudrate
 *
 * @return
 * SERIAL_FAIL    -> interface is open, nothing to do\n
 * SERIAL_SUCCESS -> interface was opened and initialized
 */
static SerialResult_t Open(uint32_t baudrate) {
  if (IsOpenFlag) {
    return SERIAL_FAIL;
  }

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

/**
 * @brief Send one byte over the interface
 * @param source is the byte to send
 *
 * @return
 * SERIAL_CLOSED -> Interface is closed and nothing is done\n
 * SERIAL_SUCCESS -> Byte has been sent
 */
static SerialResult_t SendByte(uint8_t source) {
  if (!IsOpenFlag) {
    return SERIAL_CLOSED;
  }

  while (IsWriteBusy());

  USART3->DR = source;

  return SERIAL_SUCCESS;
}

/**
 * @brief Retrieve byte from interface
 * @param[out] destination is the pointer to put the byte into
 *
 * @return
 * SERIAL_SUCCESS -> Byte successfully read without errors\n
 * SERIAL_CLOSE -> Interface is closed and nothing is done\n
 * SERIAL_INVALID_PARAMETER -> destination is a null pointer\n
 * SERIAL_NO_DATA -> No data to read from interface\n
 * SERIAL_OVER_RUN -> Byte in data register is valid, but next byte to be shifted in is being overwritten (I.E not reading fast enough)\n
 * SERIAL_FRAMING_ERROR -> Byte in data register is likely junk and issue with buadrate or other parameters thus interface is no longer in sync\n
 * SERIAL_PARITY_ERROR -> Byte in data register is likely junk and parity check failed\n
 * SERIAL_NOISE_ERROR -> Byte is data register is likely junk and noise was detected on the line
 */
static SerialResult_t GetByte(uint8_t *destination) {
  if (!IsOpenFlag) {
    return SERIAL_CLOSED;
  }

  if (!destination) {
    return SERIAL_INVALID_PARAMETER;
  }


  uint_fast8_t hasData = RxBufferHasData();

  uint32_t sr = USART3->SR;
  *destination = USART3->DR;

  if (!hasData) {
    return SERIAL_NO_DATA;
  }

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

/**
 * @brief Send string over interface
 * @param source pointer to string
 *
 * @return
 * SERIAL_CLOSED -> Interface is closed and nothing is done\n
 * SERIAL_INVALID_PARAMETER -> source pointer is a null pointer\n
 * SERIAL_FAIL -> SendByte failed\n
 * SERIAL_SUCCESS -> String has been sent
 */
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

/**
 * @brief Send string over interface
 * @param source pointer to byte array
 * @param length of the byte array
 *
 * @return
 * SERIAL_CLOSED -> Interface is closed and nothing is done\n
 * SERIAL_INVALID_PARAMETER -> source pointer is a null pointer\n
 * SERIAL_FAIL -> SendByte failed\n
 * SERIAL_SUCCESS -> String has been sent
 */
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

/**
 * @brief Initializing SerialPort3
 */
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
