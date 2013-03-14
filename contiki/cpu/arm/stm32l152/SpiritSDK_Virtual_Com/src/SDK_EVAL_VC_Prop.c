/**
 * @file     SDK_EVAL_VC_Prop.c
 * @author   MSH RF/ART Team IMS-Systems Lab
 * @version  V1.0.0
 * @date     August 4, 2011
 * @brief    Insert here the description.
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
#include "usb_conf.h"
#include "SDK_EVAL_VC_Prop.h"
#include "SDK_EVAL_VC_Desc.h"
#include "SDK_EVAL_VC_Pwr.h"
#include "SDK_EVAL_VC_General.h"



/** @addtogroup SDK_EVAL_Virtual_Com
 * @{
 */


/** @addtogroup SDK_EVAL_VC_Properties
 * @{
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_TypesDefinitions   SDK EVAL VC Properties Private TypesDefinitions
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_Defines    SDK EVAL VC Properties Private Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_Macros   SDK EVAL VC Properties Private Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_Variables    SDK EVAL VC Properties Private Variables
 * @{
 */
uint8_t Request = 0;

LineCoding xLinecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* no. of bits 8*/
  };


DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    SdkEvalVCPortInit,
    SdkEvalVCPortReset,
    SdkEvalVCPortStatusIn,
    SdkEvalVCPortStatusOut,
    SdkEvalVCPortDataSetup,
    SdkEvalVCPortNoDataSetup,
    SdkEvalVCPortGetInterfaceSetting,
    SdkEvalVCPortGetDeviceDescriptor,
    SdkEvalVCPortGetConfigDescriptor,
    SdkEvalVCPortGetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    SdkEvalVCPortGetConfiguration,
    SdkEvalVCPortSetConfiguration,
    SdkEvalVCPortGetInterface,
    SdkEvalVCPortSetInterface,
    SdkEvalVCPortGetStatus,
    SdkEvalVCPortClearFeature,
    SdkEvalVCPortSetEndPointFeature,
    SdkEvalVCPortSetDeviceFeature,
    SdkEvalVCPortSetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)g_vectcVCPortDeviceDescriptor,
    VIRTUAL_COM_PORT_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)g_vectcVCPortConfigDescriptor,
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(uint8_t*)g_vectcVCPortStringLangID, VIRTUAL_COM_PORT_SIZ_STRING_LANGID},
    {(uint8_t*)g_vectcVCPortStringVendor, VIRTUAL_COM_PORT_SIZ_STRING_VENDOR},
    {(uint8_t*)g_vectcVCPortStringProduct, VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT},
    {(uint8_t*)g_vectcVCPortStringSerial, VIRTUAL_COM_PORT_SIZ_STRING_SERIAL}
  };

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_FunctionPrototypes   SDK EVAL VC Properties Private FunctionPrototypes
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Private_Functions    SDK EVAL VC Properties Private Functions
 * @{
 */

/**
 * @brief  Virtual Com Port initialization routine at usb level.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCPortInit(void)
{
  /* Update the serial number string descriptor with the data from the unique ID*/
  SdkEvalVCGetSerialNum();

  pInformation->Current_Configuration = 0;

  /* Connect the device */
  SdkEvalVCPowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}


/**
 * @brief  Virtual Com Port reset routine at usb level.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCPortReset(void)
{
  /* Set Virtual_Com_Port DEVICE as not configured */
  pInformation->Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = g_vectcVCPortConfigDescriptor[7];

  /* Set Virtual_Com_Port DEVICE with the default Interface*/
  pInformation->Current_Interface = 0;

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_INTERRUPT);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  SetEPTxStatus(ENDP2, EP_TX_NAK);

  /* Initialize Endpoint 3 */
  SetEPType(ENDP3, EP_BULK);
  SetEPRxAddr(ENDP3, ENDP3_RXADDR);
  SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPRxStatus(ENDP3, EP_RX_VALID);
  SetEPTxStatus(ENDP3, EP_TX_DIS);

  /* Set this device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;
}

/**
 * @brief  Update the device state to configured.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCPortSetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
}


/**
 * @brief  Update the device state to addressed.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCPortSetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}


/**
 * @brief  Virtual COM Port Status In Routine.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCPortStatusIn(void)
{
  if (Request == SET_LINE_CODING)
  {
    Request = 0;
  }
}


/**
 * @brief  Virtual COM Port Status OUT Routine.
 * @param  None
 * @retval None
 */
void SdkEvalVCPortStatusOut(void)
{

}


/**
 * @brief  Handle the data class specific requests.
 * @param  cRequestNo: Number of requests.
 * @retval RESULT: USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT SdkEvalVCPortDataSetup(uint8_t cRequestNo)
{
  uint8_t    *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;

  if (cRequestNo == GET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = SdkEvalVCPortGetLineCoding;
    }
  }
  else if (cRequestNo == SET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = SdkEvalVCPortSetLineCoding;
    }
    Request = SET_LINE_CODING;
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;

}


/**
 * @brief  Handle the no data class specific requests.
 * @param  cRequestNo: Number of requests.
 * @retval RESULT: USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT SdkEvalVCPortNoDataSetup(uint8_t cRequestNo)
{

  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (cRequestNo == SET_COMM_FEATURE)
    {
      return USB_SUCCESS;
    }
    else if (cRequestNo == SET_CONTROL_LINE_STATE)
    {
      return USB_SUCCESS;
    }
  }

  return USB_UNSUPPORT;

}


/**
 * @brief  Gets the device descriptor.
 * @param  nLength: length of the device descriptor.
 * @retval uint8_t: the address of the device descriptor.
 */
uint8_t* SdkEvalVCPortGetDeviceDescriptor(uint16_t nLength)
{
  return Standard_GetDescriptorData(nLength, &Device_Descriptor);

}


/**
 * @brief  Gets the configuration descriptor.
 * @param  nLength: length of the configuration descriptor.
 * @retval uint8_t: the address of the configuration descriptor.
 */
uint8_t* SdkEvalVCPortGetConfigDescriptor(uint16_t nLength)
{
  return Standard_GetDescriptorData(nLength, &Config_Descriptor);

}


/**
 * @brief  Gets the string descriptors according to the needed index.
 * @param  nLength: length of the string descriptor.
 * @retval uint8_t: the address of the string descriptor.
 */
uint8_t* SdkEvalVCPortGetStringDescriptor(uint16_t nLength)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(nLength, &String_Descriptor[wValue0]);
  }
}


/**
 * @brief  Test the interface and the alternate setting according to the supported one.
 * @param  cInterface: interface number.
 * @param  cAlternateSetting: alternate setting number.
 * @retval RESULT: USB_UNSUPPORT or USB_SUCCESS.
 */
RESULT SdkEvalVCPortGetInterfaceSetting(uint8_t cInterface, uint8_t cAlternateSetting)
{
  if (cAlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (cInterface > 1)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}


/**
 * @brief  Send the xLinecoding structure to the PC host.
 * @param  nLength: length of the xLinecoding structure.
 * @retval uint8_t: xLinecoding structure base address.
 */
uint8_t* SdkEvalVCPortGetLineCoding(uint16_t nLength)
{
  if (nLength == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(xLinecoding);
    return NULL;
  }
  return(uint8_t *)&xLinecoding;
}


/**
 * @brief  Set the xLinecoding structure fields.
 * @param  nLength: length of the xLinecoding structure.
 * @retval uint8_t: xLinecoding structure base address.
 */
uint8_t* SdkEvalVCPortSetLineCoding(uint16_t nLength)
{
  if (nLength == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(xLinecoding);
    return NULL;
  }
  return(uint8_t *)&xLinecoding;
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
