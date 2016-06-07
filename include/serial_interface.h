#ifndef __SERIAL_INTERFACE__
#define __SERIAL_INTERFACE__

#include "common.h"

typedef struct {
  uint_fast8_t (*IsOpen)(void);
  uint_fast8_t (*Open)(uint32_t baudrate);
  void         (*Close)(void);
  uint_fast8_t (*SendByte)(uint8_t);
  uint_fast8_t (*SendString)(const uint8_t *source);
  uint_fast8_t (*SendArray)(const uint8_t *source, uint32_t length);
  int_fast8_t  (*RxBufferHasData)(void);
  int_fast8_t  (*GetByte)(uint8_t *destination);
} SerialInterface;

#endif