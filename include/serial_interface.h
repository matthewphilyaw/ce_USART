#ifndef __SERIAL_INTERFACE__
#define __SERIAL_INTERFACE__

#include "common.h"

typedef enum {
  SERIAL_SUCCESS = 0,
  SERIAL_FAIL = -1,
  SERIAL_CLOSED = -2,
  SERIAL_NO_DATA = -3,

  SERIAL_OVER_RUN = -4,
  SERIAL_FRAMING_ERROR = -5,
  SERIAL_PARITY_ERROR = -6,
  SERIAL_NOISE_ERROR = - 7,
  SERIAL_LINE_BREAK_ERROR = -8,
  SERIAL_INVALID_PARAMETER = -9
} SerialResult_t;

typedef struct {
  uint_fast8_t   (*IsOpen)(void);
  uint_fast8_t   (*RxBufferHasData)(void);
  SerialResult_t (*Open)(uint32_t baudrate);
  void           (*Close)(void);
  SerialResult_t (*SendByte)(uint8_t);
  SerialResult_t (*SendString)(const char *source);
  SerialResult_t (*SendArray)(const uint8_t *source, uint32_t length);
  SerialResult_t (*GetByte)(uint8_t *destination);
} SerialInterface;

#endif
