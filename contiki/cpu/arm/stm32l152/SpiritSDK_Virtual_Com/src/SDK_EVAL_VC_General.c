/**
* @file     SDK_EVAL_VC_General.c
* @author   MSH RF/ART Team IMS-Systems Lab
* @version  V1.0.0
* @date     August 4, 2011
* @brief    SDK EVAL Virtual Com Setup & API.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
*
*/

/* Includes ------------------------------------------------------------------*/
#include <yfuns.h>
#include "SDK_EVAL_VC_General.h"
#include "SDK_EVAL_Config.h"


/** @addtogroup SDK_EVAL_Virtual_Com
* @{
*/


/** @addtogroup SDK_EVAL_VC_General
* @{
*/


/** @defgroup SDK_EVAL_VC_General_Private_TypesDefinitions    SDK EVAL VC General Private TypesDefinitions
* @{
*/


/**
* @}
*/


/** @defgroup SDK_EVAL_VC_General_Private_Macros    SDK EVAL VC General Private Macros
* @{
*/


#define USB_ENTER_CRITICAL()              {NVIC_InitTypeDef NVIC_InitStructure = {USB_LP_IRQn, 1, 0,  DISABLE}; \
  NVIC_Init(&NVIC_InitStructure);} \
    
#define USB_EXIT_CRITICAL()              {NVIC_InitTypeDef NVIC_InitStructure = {USB_LP_IRQn, 1, 0,  ENABLE}; \
  NVIC_Init(&NVIC_InitStructure);} \
    
#define USB_OUT_BUFFER_IS_FULL() (s_lVCTxBufferInIndex == s_lVCTxBufferOutIndex - 1 || (s_lVCTxBufferInIndex == VC_TX_BUFFER_DATA_SIZE && s_lVCTxBufferOutIndex == 0))
#define USB_OUT_BUFFER_IS_EMPTY() (s_lVCTxBufferInIndex == s_lVCTxBufferOutIndex)

/**
* @}
*/


/** @defgroup SDK_EVAL_VC_General_Private_Variables     SDK EVAL VC General Private Variables
* @{
*/

uint8_t  s_vectcVCTxBuffer [VC_TX_BUFFER_DATA_SIZE];
uint32_t s_lVCTxBufferInIndex = 0;
uint32_t s_lVCTxBufferOutIndex = 0;
uint32_t s_lVCTxDataLength  = 0;

uint8_t  USB_Tx_State = 0;


#define RECEIVE_QUEUE_SIZE VC_TX_BUFFER_DATA_SIZE 

uint8_t rxQ[RECEIVE_QUEUE_SIZE];
uint16_t rxHead = 0;
uint16_t rxTail = 0;
volatile uint16_t rxUsed = 0;


/**
* @}
*/


/** @defgroup SDK_EVAL_VC_General_Private_FunctionPrototypes      SDK EVAL VC General Private FunctionPrototypes
* @{
*/
static void IntToUnicode (uint32_t lValue , uint8_t* pcBuffer , uint8_t cLength);
/**
* @}
*/


/** @defgroup SDK_EVAL_VC_General_Private_Functions     SDK EVAL VC General Private Functions
* @{
*/


/**
* @brief  Configures main system clocks, USB clock, USB interrupt and GPIO pin for USB pull-up if defined.
* @param  None.
* @retval None.
*/
void SdkEvalVCInit(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
#if defined(USB_USE_EXTERNAL_PULLUP)
  GPIO_InitTypeDef  GPIO_InitStructure;
#endif /* USB_USE_EXTERNAL_PULLUP */
  
  /* At this stage the microcontroller clock setting is already configured,
  this is done through SystemInit() function which is called from startup
  file (startup_stm32l1x_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32l1xx.c file */
  
  /* Enable the SYSCFG module clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  
#if defined(USB_USE_EXTERNAL_PULLUP)
  /* Enable the USB disconnect GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIO_DISCONNECT, ENABLE);
  
  /* USB_DISCONNECT used as USB pull-up */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
#endif /* USB_USE_EXTERNAL_PULLUP */

  /* Enables USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  
  /* Configures USB interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Initializes USB system */
  USB_Init();
}


/**
* @brief  Power-off system clocks and power while entering suspend mode.
* @param  None.
* @retval None.
*/
void SdkEvalVCEnterLowPowerMode(void)
{
  /* Sets the device state to suspend */
  bDeviceState = SUSPENDED;
  
}


/**
* @brief  Restores system clocks and power while exiting suspend mode.
* @param  None.
* @retval None.
*/
void SdkEvalVCLeaveLowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;
  
  /* Sets the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }
  
}


/**
* @brief  Software Connection/Disconnection of USB Cable.
* @param  xNewState new state for cable connection.
*         This parameter can be ENABLE or DISABLE.
* @retval None.
*/
void SdkEvalVCCableConfig (FunctionalState xNewState)
{
  if (xNewState != DISABLE)
  {
    STM32L15_USB_CONNECT;
  }
  else
  {
    STM32L15_USB_DISCONNECT;
  }
  
}

/**
* @brief  Puts in the circular buffer the data to transfer through USB.
* @param  pcDataBuffer pointer to the buffer containing the bytes to put in the circular buffer.
* @param  nNbBytes the number of bytes to write in the buffer.
* @retval None.
*/
void SdkEvalVCWriteTxBuffer(uint8_t* pcDataBuffer, uint16_t nNbBytes)
{
  uint32_t inIndex = s_lVCTxBufferInIndex;
  
  for(uint16_t i=0; i<nNbBytes; i++)
  {
    s_vectcVCTxBuffer[inIndex] = *pcDataBuffer;
    inIndex++;
    /* To avoid buffer overflow */
    if(inIndex == VC_TX_BUFFER_DATA_SIZE)
    {
      inIndex = 0;
    }
    pcDataBuffer++;
  }
  s_lVCTxBufferInIndex = inIndex;
}

//////////////////////////////////////////////////////////////////////////////
// Transmit

// IAR Standard library hook for serial output
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  
  /* This template only writes to "standard out" and "standard err",
  * for all other file handles it returns failure. */
  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR) {
    return _LLIO_ERROR;
  }
  if (buffer == 0) {
    
    return 0;
  }
  
  
  
  for(uint16_t i=0; i<size; i++)
  {
    s_vectcVCTxBuffer[s_lVCTxBufferInIndex++] = *buffer;
    
    while(USB_OUT_BUFFER_IS_FULL()) ;
    
    /* To avoid buffer overflow */
    if(s_lVCTxBufferInIndex == VC_TX_BUFFER_DATA_SIZE)
    {
      s_lVCTxBufferInIndex = 0;
    }
    buffer++;
  }
  
  return size;
}

size_t fflush(int handle)
{
  return __write(_LLIO_STDOUT, NULL, 0);
}


size_t __read(int handle, unsigned char * buffer, size_t size)
{
  int nChars = 0;
  
  /* This template only reads from "standard in", for all other file
  * handles it returns failure. */
  if (handle != _LLIO_STDIN)
  {
    return _LLIO_ERROR;
  }
  
  USB_ENTER_CRITICAL();
  
  for(nChars = 0; (rxUsed>0) && (nChars < size); nChars++) {
    *buffer++ = rxQ[rxTail];
    rxTail = (rxTail+1) % RECEIVE_QUEUE_SIZE;
    rxUsed--;
    
  }
  
  USB_EXIT_CRITICAL();     
  
  return nChars;
}

uint8_t __io_getcharNonBlocking(uint8_t *data)
{
  if (__read(_LLIO_STDIN,data,1))
    return TRUE;
  else
    return FALSE;
}/* end serialReadByte() */

void __io_putchar( char c )
{
  __write(_LLIO_STDOUT, (unsigned char *)&c, 1);
}

int __io_getchar()
{
  unsigned char c;
  
  __read(_LLIO_STDIN, &c, 1);
  return (int)(c);
}

void __io_flush( void )
{
  __write(_LLIO_STDOUT, NULL, 0);
}

/**
* @brief  Transfers available data through USB.
* @param  None.
* @retval None.
*/
void SdkEvalVCSendData(void)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;
  
  if(USB_Tx_State != 1)
  {
    if (s_lVCTxBufferOutIndex == VC_TX_BUFFER_DATA_SIZE)
    {
      s_lVCTxBufferOutIndex = 0;
    }
    
    if(s_lVCTxBufferOutIndex == s_lVCTxBufferInIndex)
    {
      USB_Tx_State = 0; 
      return;
    }
    if(s_lVCTxBufferOutIndex > s_lVCTxBufferInIndex) /* rollback */
    {
      s_lVCTxDataLength = VC_TX_BUFFER_DATA_SIZE - s_lVCTxBufferOutIndex;
    }
    else
    {
      s_lVCTxDataLength = s_lVCTxBufferInIndex - s_lVCTxBufferOutIndex;
    }
    if (s_lVCTxDataLength > VIRTUAL_COM_PORT_DATA_SIZE)
    {
      USB_Tx_ptr = s_lVCTxBufferOutIndex;
      USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;
      
      s_lVCTxBufferOutIndex += VIRTUAL_COM_PORT_DATA_SIZE;
      s_lVCTxDataLength -= VIRTUAL_COM_PORT_DATA_SIZE;
      
    }
    else
    {
      USB_Tx_ptr = s_lVCTxBufferOutIndex;
      USB_Tx_length = s_lVCTxDataLength;
      
      s_lVCTxBufferOutIndex += s_lVCTxDataLength;
      s_lVCTxDataLength = 0;
      
    }
    
    USB_Tx_State = 1;
    UserToPMABufferCopy(&s_vectcVCTxBuffer[USB_Tx_ptr], ENDP1_TXADDR, USB_Tx_length);
    SetEPTxCount(ENDP1, USB_Tx_length);
    SetEPTxValid(ENDP1);
    
  }
  
}


/**
* @brief  Creates the serial number string descriptor.
* @param  None.
* @retval None.
*/
void SdkEvalVCGetSerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
  
  Device_Serial0 = *(uint32_t*)(0x1FF80050);
  Device_Serial1 = *(uint32_t*)(0x1FF80054);
  Device_Serial2 = *(uint32_t*)(0x1FF80064);
  
  Device_Serial0 += Device_Serial2;
  
  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &g_vectcVCPortStringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &g_vectcVCPortStringSerial[18], 4);
  }
  
}


/**
* @brief  Converts Hex 32Bits value into char.
* @param  lValue the HEX words to convert.
* @param  pcBuffer the buffer in whic put the converted values.
* @param  cLength the number of byte to convert.
* @retval None.
*/
static void IntToUnicode (uint32_t lValue , uint8_t* pcBuffer , uint8_t cLength)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < cLength ; idx ++)
  {
    if( ((lValue >> 28)) < 0xA )
    {
      pcBuffer[ 2* idx] = (lValue >> 28) + '0';
    }
    else
    {
      pcBuffer[2* idx] = (lValue >> 28) + 'A' - 10;
    }
    
    lValue = lValue << 4;
    
    pcBuffer[ 2* idx + 1] = 0;
  }
  
}


/**
* @brief  Resets the static variable only for test purpose.
* @param  None.
* @retval None.
*/
void SdkEvalVCResetCounter(void)
{
  s_lVCTxBufferInIndex = 0;
  s_lVCTxBufferOutIndex = 0;
}


/**
* @brief  Sets the static variable only for test purpose.
* @param  None.
* @retval None.
*/
void SdkEvalVCSetCounter(uint32_t lInIndex, uint32_t lOutIndex)
{
  s_lVCTxBufferInIndex = lInIndex;
  s_lVCTxBufferOutIndex = lOutIndex;
}


void enqueueRxChars(unsigned char * buffer, uint16_t size)
{
  // Enqueue chars received from USB into a RX buffer
  while (( size > 0 ) && (rxUsed < (RECEIVE_QUEUE_SIZE-1))) {
    rxQ[rxHead] = *buffer++;
    rxHead = (rxHead+1) % RECEIVE_QUEUE_SIZE;
    rxUsed++;
    size--;
  }
}

/**
* @}
*/


/**
* @}
*/


/**
* @}
*/




/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
