#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include "MCU/device.h"

#define TRUE 1
#define FALSE 0

#define FIRMWARE_VERSION "00.001D"
#define HARDWARE_VERSION "01"
#define COMPILED_DATA_TIME "[" __DATE__ " " __TIME__ "]"

#define EN_DEBUG_INTERFACE
#define USART_OVER_SAMPLE_16

#define FIFO_UINT8_T

#define USART_MAX_BUFFER 20

#endif
