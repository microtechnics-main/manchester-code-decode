/**
  ******************************************************************************
  * @file           : manchester_code.h
  * @brief          : Manchester code driver interface
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */

#ifndef MANCH_H
#define MANCH_H



/* Includes ------------------------------------------------------------------*/

#include "stm32f1xx_hal.h"
#include <string.h>



/* Declarations and definitions ----------------------------------------------*/

#define MANCH_OUTPUT_PORT                                       GPIOA
#define MANCH_OUTPUT_PIN                                        GPIO_PIN_3

#define MANCH_INPUT_PORT                                        GPIOA
#define MANCH_INPUT_PIN                                         GPIO_PIN_4

#define MANCH_ENCODE_TIMER_PERIOD_US                            10
#define MANCH_BIT_TIME_US                                       100

#define MANCH_BYTES_NUM                                         10
#define MANCH_SYNC_BYTES_NUM                                    2
#define MANCH_DATA_BYTES_NUM                                    MANCH_BYTES_NUM - MANCH_SYNC_BYTES_NUM

#define MANCH_BITS_IN_BYTE_NUM                                  8

#define MANCH_SYNC_FIELD                                        0xAA55

#define MANCH_DECODE_TIMER_PERIOD_US                            10

#define MANCH_ENCODE_TIMER_MAX                                  MANCH_BIT_TIME_US / MANCH_ENCODE_TIMER_PERIOD_US

#define MANCH_DECODE_TIMER_MAX                                  MANCH_BIT_TIME_US / MANCH_DECODE_TIMER_PERIOD_US
#define MANCH_DECODE_TIMER_THRESHOLD                            MANCH_DECODE_TIMER_MAX * 3 / 4



typedef enum
{
  NOT_SYNC,
  BIT_SYNC,
  DATA_SYNC,
  DATA_READY
} MANCH_DecodeState;

typedef enum
{
  NONE,
  RAISING_EDGE,
  FALLING_EDGE
} MANCH_DecodeEdge;

typedef struct MANCH_Data
{
  uint8_t data[MANCH_BYTES_NUM];
  uint16_t bitStream;
  uint16_t byteIdx;
  uint16_t bytesNum;
  uint8_t bitIdx;
  uint8_t active;
} MANCH_Data;



/* Functions -----------------------------------------------------------------*/

extern void MANCH_Encode(uint8_t* data, uint8_t size);
extern void MANCH_DecodeReset();



#endif // #ifndef MANCH_H
