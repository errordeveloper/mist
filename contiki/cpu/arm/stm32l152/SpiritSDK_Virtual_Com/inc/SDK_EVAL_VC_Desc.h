/**
 * @file    SDK_EVAL_VC_Desc.h
 * @author  MSH RF/ART Team IMS-Systems Lab
 * @version V1.0.0
 * @date    August 4, 2011
 * @brief   Descriptor Header for Virtual COM Port Device.
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
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDK_EVAL_VC_DESC_H
#define __SDK_EVAL_VC_DESC_H


#ifdef __cplusplus
extern "C" {
#endif



/** @defgroup SDK_EVAL_Virtual_Com    SDK EVAL Virtual Com
 * @{
 */


/** @defgroup SDK_EVAL_VC_Descriptor    SDK EVAL VC Descriptor
 * @brief Descriptor module for Virtual COM Port Device.
 * @details See the file <i>@ref SDK_EVAL_VC_Desc.h</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_Descriptor_Exported_Types   SDK EVAL VC Descriptor Exported Types
 * @{
 */

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define VIRTUAL_COM_PORT_DATA_SIZE              64
#define VIRTUAL_COM_PORT_INT_SIZE               8

#define VIRTUAL_COM_PORT_SIZ_DEVICE_DESC        18
#define VIRTUAL_COM_PORT_SIZ_CONFIG_DESC        67
#define VIRTUAL_COM_PORT_SIZ_STRING_LANGID      4
#define VIRTUAL_COM_PORT_SIZ_STRING_VENDOR      38
#define VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT     112
#define VIRTUAL_COM_PORT_SIZ_STRING_SERIAL      26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09


/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Descriptor_Exported_Constants   SDK EVAL VC Descriptor Exported Constants
 * @{
 */
extern const uint8_t g_vectcVCPortDeviceDescriptor[VIRTUAL_COM_PORT_SIZ_DEVICE_DESC];
extern const uint8_t g_vectcVCPortConfigDescriptor[VIRTUAL_COM_PORT_SIZ_CONFIG_DESC];

extern const uint8_t g_vectcVCPortStringLangID[VIRTUAL_COM_PORT_SIZ_STRING_LANGID];
extern const uint8_t g_vectcVCPortStringVendor[VIRTUAL_COM_PORT_SIZ_STRING_VENDOR];
extern const uint8_t g_vectcVCPortStringProduct[VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT];
extern uint8_t g_vectcVCPortStringSerial[VIRTUAL_COM_PORT_SIZ_STRING_SERIAL];

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Descriptor_Exported_Macros    SDK EVAL VC Descriptor Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Descriptor_Exported_Functions   SDK EVAL VC Descriptor Exported Functions
 * @{
 */

/**
 * @}
 */


/**
 * @}
 */


/**
 * @}
 */




#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
