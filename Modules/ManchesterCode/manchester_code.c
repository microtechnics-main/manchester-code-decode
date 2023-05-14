/**
  ******************************************************************************
  * @file           : manchester_code.c
  * @brief          : Manchester code driver
  * @author         : MicroTechnics (microtechnics.ru)
  ******************************************************************************
  */



/* Includes ------------------------------------------------------------------*/

#include "manchester_code.h"



/* Declarations and definitions ----------------------------------------------*/

static uint8_t virtTact = 1;

static MANCH_Data encodeData;
static MANCH_Data decodeData;

static MANCH_DecodeEdge curEdge = NONE;
static MANCH_DecodeEdge prevEdge = NONE;

static MANCH_DecodeState decodeState = NOT_SYNC;

static uint16_t encodeTimerCnt = 0;
static uint16_t decodeTimerCnt = 0;

extern TIM_HandleTypeDef htim2;



/* Functions -----------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static void SetOutput(uint8_t state)
{
  HAL_GPIO_WritePin(MANCH_OUTPUT_PORT, MANCH_OUTPUT_PIN, (GPIO_PinState)state);
}



/*----------------------------------------------------------------------------*/
static uint8_t GetInput()
{
  uint8_t state = (uint8_t)HAL_GPIO_ReadPin(MANCH_INPUT_PORT, MANCH_INPUT_PIN);
  return state;
}



/*----------------------------------------------------------------------------*/
void MANCH_Encode(uint8_t* data, uint8_t size)
{
  encodeData.bitIdx = 0;
  encodeData.byteIdx = 0;
  
  if (size > MANCH_DATA_BYTES_NUM)
  {
    encodeData.bytesNum = MANCH_DATA_BYTES_NUM + MANCH_SYNC_BYTES_NUM;
  }
  else
  {
    encodeData.bytesNum = size + MANCH_SYNC_BYTES_NUM;
  }
  
  memcpy(&encodeData.data[MANCH_SYNC_BYTES_NUM], data, encodeData.bytesNum - MANCH_SYNC_BYTES_NUM);
  encodeData.data[0] = MANCH_SYNC_FIELD & 0xFF;
  encodeData.data[1] = (MANCH_SYNC_FIELD & 0xFF00) >> 8;
  
  encodeTimerCnt = 0;
  virtTact = 1;
  encodeData.active = 1;
}



/*----------------------------------------------------------------------------*/
static uint8_t GetDataBit(MANCH_Data* manchData)
{
  uint8_t res;
  
  uint8_t curByte = manchData->data[manchData->byteIdx];
  uint8_t curBitIdx = manchData->bitIdx;
  
  res = (curByte >> curBitIdx) & 0x01;
  
  return res;
}



/*----------------------------------------------------------------------------*/
static void SetDataBit(MANCH_Data* manchData, uint8_t bit)
{
  uint8_t curByteIdx = manchData->byteIdx;
  uint8_t curBitIdx = manchData->bitIdx;
  
  if (bit == 1)
  {
    manchData->data[curByteIdx] |= (1 << curBitIdx);
  }
}



/*----------------------------------------------------------------------------*/
__weak void MANCH_DataReadyCallback()
{
}



/*----------------------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    // Encoding process
    if (encodeData.active == 1)
    {
      if ((encodeTimerCnt == (MANCH_ENCODE_TIMER_MAX / 2)) ||
          (encodeTimerCnt == MANCH_ENCODE_TIMER_MAX))
      {
        uint8_t curCodeBit = GetDataBit(&encodeData);
        uint8_t curOutputBit = curCodeBit ^ virtTact;
        SetOutput(curOutputBit);
        virtTact ^= 0x01;
      }
            
      if (encodeTimerCnt == MANCH_ENCODE_TIMER_MAX)
      {        
        encodeData.bitIdx++;
        
        if (encodeData.bitIdx == (MANCH_BITS_IN_BYTE_NUM))
        {
          encodeData.bitIdx = 0;
          
          encodeData.byteIdx++;
          if (encodeData.byteIdx == encodeData.bytesNum)
          {
            encodeData.active = 0;
          }
        }
        
        encodeTimerCnt = 0;
      }
    
      encodeTimerCnt++;
    }
    
    // Decoding process
    if (decodeData.active == 1)
    {
      if (decodeState == DATA_SYNC)
      {
        if (decodeTimerCnt >= (3 * MANCH_DECODE_TIMER_MAX))
        {
          decodeTimerCnt = 0;
          
          decodeData.active = 0;
          curEdge = NONE;
          prevEdge = NONE;
          
          decodeState = DATA_READY;
        
          // Data is ready
          MANCH_DataReadyCallback();
        }
      }
      
      decodeTimerCnt++;
    }
  }
}



/*----------------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == MANCH_INPUT_PIN)
  {
    uint8_t inputState = GetInput();
    
    if (inputState == 0)
    {
      curEdge = FALLING_EDGE;
    }
    else
    {
      curEdge = RAISING_EDGE;
    }
    
    switch (decodeState)
    {
      case NOT_SYNC:
        if (decodeData.active == 0)
        {
          decodeData.active = 1;
          decodeTimerCnt = 0;
        }
        else
        {
          if (((curEdge == FALLING_EDGE) && (prevEdge == RAISING_EDGE)) ||
              ((curEdge == RAISING_EDGE) && (prevEdge == FALLING_EDGE)))
          {
            if (decodeTimerCnt >= MANCH_DECODE_TIMER_THRESHOLD)
            {
              if (curEdge == FALLING_EDGE)
              {
                decodeData.bitStream = 0x4000;
                decodeData.bitStream >>= 1;
              }
              else
              {
                decodeData.bitStream = 0x8000;
                decodeData.bitStream >>= 1;
              }
  
              for (uint8_t i = 0; i < MANCH_BYTES_NUM; i++)
              {
                decodeData.data[i] = 0x00;
              }
  
              decodeState = BIT_SYNC;
            }
            
            decodeTimerCnt = 0;
          }
        }
        break;
        
      case BIT_SYNC:
        if (decodeTimerCnt >= MANCH_DECODE_TIMER_THRESHOLD)
        {         
          if (curEdge == RAISING_EDGE)
          {
            decodeData.bitStream |= 0x8000;
          }
                    
          if (decodeData.bitStream == MANCH_SYNC_FIELD)
          {          
            decodeState = DATA_SYNC;
            decodeData.data[0] = decodeData.bitStream & 0xFF;
            decodeData.data[1] = (decodeData.bitStream & 0xFF00) >> 8;
            decodeData.bitIdx = 0;
            decodeData.byteIdx = MANCH_SYNC_BYTES_NUM;
            decodeData.bytesNum = MANCH_DATA_BYTES_NUM + MANCH_SYNC_BYTES_NUM;
          }
          else
          {
              decodeData.bitStream >>= 1;
          }
          
          decodeTimerCnt = 0;
        }
        break;
        
      case DATA_SYNC:
        if (decodeTimerCnt >= MANCH_DECODE_TIMER_THRESHOLD)
        {
          if (curEdge == RAISING_EDGE)
          {
            SetDataBit(&decodeData, 1);
          }
          
          decodeData.bitIdx++;
          
          if (decodeData.bitIdx == (MANCH_BITS_IN_BYTE_NUM))
          {
            decodeData.bitIdx = 0;
            
            decodeData.byteIdx++;
            if (decodeData.byteIdx == decodeData.bytesNum)
            {     
              decodeData.active = 0;
              curEdge = NONE;
              prevEdge = NONE;
              
              decodeState = DATA_READY;
              
              // Data is ready
              MANCH_DataReadyCallback();
            }
          }
          
          decodeTimerCnt = 0;
        }
        break;
        
      case DATA_READY:
        break;
        
      default:
        break;
    }
    
    prevEdge = curEdge;
  }
}



/*----------------------------------------------------------------------------*/
void MANCH_DecodeReset()
{
  decodeState = NOT_SYNC;
}



/*----------------------------------------------------------------------------*/
