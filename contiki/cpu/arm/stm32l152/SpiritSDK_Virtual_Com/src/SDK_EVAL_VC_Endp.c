/**
 * @file     SDK_EVAL_VC_Endp.c
 * @author   MSH RF/ART Team IMS-Systems Lab
 * @version  V1.0.0
 * @date     August 4, 2011
 * @brief    Endpoint routines.
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
#include "usb_lib.h"
#include "SDK_EVAL_VC_Desc.h"
#include "usb_mem.h"
#include "SDK_EVAL_VC_General.h"
#include "SDK_EVAL_VC_Istr.h"
#include "SDK_EVAL_VC_Pwr.h"



/** @addtogroup SDK_EVAL_Virtual_Com
 * @{
 */


/** @addtogroup SDK_EVAL_VC_Endpoint	SDK EVAL VC Endpoint
 * @brief Implementation of endpoint routines for Virtual COM Port Device.
 * @details See the file <i>@ref SDK_EVAL_VC_Endp.c</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_TypesDefinitions   SDK EVAL VC Endpoint Private TypesDefinitions
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_Defines    SDK EVAL VC Endpoint Private Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_Macros   SDK EVAL VC Endpoint Private Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_Variables    SDK EVAL VC Endpoint Private Variables
 * @{
 */

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5
extern uint8_t  s_vectcVCTxBuffer [];


extern uint32_t s_lVCTxDataLength;
extern uint32_t s_lVCTxBufferOutIndex;

extern uint8_t  USB_Tx_State;

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_FunctionPrototypes   SDK EVAL VC Endpoint Private FunctionPrototypes
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Endpoint_Private_Functions    SDK EVAL VC Endpoint Private Functions
 * @{
 */

/**
 * @brief  Endpoint3_Output Callback.
 * @param  None
 * @retval None
 */

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback (void)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;
  
  if (USB_Tx_State == 1)
  {
    if (s_lVCTxDataLength == 0) 
    {
      USB_Tx_State = 0;
      USB_Tx_length = 0;
    }
    else 
    {
      if (s_lVCTxDataLength > VIRTUAL_COM_PORT_DATA_SIZE){
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
      
      UserToPMABufferCopy(&s_vectcVCTxBuffer[USB_Tx_ptr], ENDP1_TXADDR, USB_Tx_length);
//      SetEPTxCount(ENDP1, USB_Tx_length);
//      SetEPTxValid(ENDP1); 
    }
    SetEPTxCount(ENDP1, USB_Tx_length);
    SetEPTxValid(ENDP1); 
  }
}

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
extern void enqueueRxChars(unsigned char * buffer, uint16_t size);
void EP3_OUT_Callback(void)
{
  static uint8_t cVCRxBuffer[VIRTUAL_COM_PORT_DATA_SIZE];
  uint16_t USB_Rx_Cnt;
  
  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = USB_SIL_Read(EP3_OUT, cVCRxBuffer);
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the USART Xfer */
  
  enqueueRxChars(cVCRxBuffer, USB_Rx_Cnt);
  
  /* Enable the receive of data on EP3 */
  SetEPRxValid(ENDP3);
}

/*******************************************************************************
* Function Name  : SOF_Callback 
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void)
{
  static uint32_t FrameCount = 0;
  
  if(bDeviceState == CONFIGURED)
  {
    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;
      
      /* Check the data to be sent through IN pipe */
      SdkEvalVCSendData();
    }
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
