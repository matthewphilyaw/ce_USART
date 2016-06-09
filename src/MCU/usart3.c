#include <MCU/usart3.h>

// TAKEN FROM HAL LAYER
#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)            (((_PCLK_)*25U)/(4U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)        (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)        (((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100U)) * 16U + 50U) / 100U)
/* UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU) */
#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4U) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0FU))

#define UART_DIV_SAMPLING8(_PCLK_, _BAUD_)             (((_PCLK_)*25U)/(2U*(_BAUD_)))
#define UART_DIVMANT_SAMPLING8(_PCLK_, _BAUD_)         (UART_DIV_SAMPLING8((_PCLK_), (_BAUD_))/100U)
#define UART_DIVFRAQ_SAMPLING8(_PCLK_, _BAUD_)         (((UART_DIV_SAMPLING8((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) * 100U)) * 8U + 50U) / 100U)
/* UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + ((UART DIVFRAQ & 0xF8) << 1) + (UART DIVFRAQ & 0x07U) */
#define UART_BRR_SAMPLING8(_PCLK_, _BAUD_)             (((UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) << 4U) + \
                                                        ((UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0xF8U) << 1U)) + \
                                                        (UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0x07U))

static uint_fast8_t IsOpenFlag = FALSE;

static uint_fast8_t IsOpen(void) {
  return IsOpenFlag;
}

static uint_fast8_t IsWriteBusy(void) {
  return !(USART3->SR & USART_SR_TXE);
}


static uint_fast8_t Open(uint32_t baudrate) {
  if (IsOpenFlag) {
    return FALSE;
  }

  // enable GPIO TX pin
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  GPIOD->MODER &= ~GPIO_MODER_MODER8;
  GPIOD->MODER |= GPIO_MODER_MODER8_1;
  GPIOD->AFR[1] &= ~0x0F;
  GPIOD->AFR[1] |= 0x07;

  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
  USART3->CR1 = 0;
  USART3->CR1 &= ~USART_CR1_M;    // 8 bit data
  USART3->CR2 &= ~USART_CR2_STOP; // 1 stop bit

#ifdef USART_OVER_SAMPLE_16
  USART3->BRR = UART_BRR_SAMPLING16(SystemCoreClock, baudrate);
#else
  USART3->BRR = UART_BRR_SAMPLING8(SystemCoreClock, baudrate);
#endif

  USART3->CR1 |= USART_CR1_UE;    // enable USART
  USART3->CR1 |= USART_CR1_TE;

  IsOpenFlag = TRUE;
  return TRUE;
}

static void Close(void) {
  USART3->CR1 &= ~(1);
}

static uint_fast8_t SendByte(uint8_t source) {
  if (!IsOpenFlag)
  {
    return FALSE;
  }

  while (IsWriteBusy());

  USART3->DR = source;

  return TRUE;
}

/*
 *  1 = have data
 *  0 = no data
 * -1 = port closed 
 */
static int_fast8_t RxBufferHasData(void) {
  // TODO: Implement
  return -1;
}

/*
 *  1 = success
 *  0 = no data
 * -1 = port closed or destination pointer invalid 
 */
static int_fast8_t GetByte(uint8_t *destination) {
  // TODO: Implement
  return -1;
}

static uint_fast8_t SendString(const uint8_t *source) {
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

static uint_fast8_t SendArray(const uint8_t *source, uint32_t length) {
  if (!IsOpen || !source) {
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
