#include "MCU/usart2.h"

static IsOpenFlag = FALSE;

static uint_fast8_t IsOpen(void) {
  return IsOpenFlag;
}

static uint_fast8_t IsWriteBusy(void) {
  // TODO: Implement
  return TRUE; 
}

static void SetBaudrate(uint32_t baud) {
  // TODO: Implement
}

static uint_fast8_t Open(uint32_t baudrate) {
  // TODO: Implement
  return FALSE;
}

static void Close(void) {
  // TODO: Implement
}

static uint_fast8_t SendByte(uint8_t source) {
  // TODO: Implement
  return FALSE;
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
  if (!IsOpen || !source) {
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

SerialInterface SerialPort2 = {
  IsOpen,
  Open,
  Close,
  SendByte,
  SendString,
  SendArray,
  RxBufferHasData,
  GetByte
};
