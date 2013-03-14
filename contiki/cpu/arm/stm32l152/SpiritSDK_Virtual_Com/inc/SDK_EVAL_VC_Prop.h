/**
 * @file    SDK_EVAL_VC_Prop.h
 * @author  MSH RF/ART Team IMS-Systems Lab
 * @version V1.0.0
 * @date    August 4, 2011
 * @brief   Header file for all processing related to Virtual COM Port.
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
#ifndef __SDK_EVAL_VC_PROP_H
#define __SDK_EVAL_VC_PROP_H


#ifdef __cplusplus
extern "C" {
#endif




/** @defgroup SDK_EVAL_Virtual_Com    SDK EVAL Virtual Com
 * @{
 */


/** @defgroup SDK_EVAL_VC_Properties    SDK EVAL VC Properties
 * @brief Properties and configuration module for Virtual COM Port Device.
 * @details See the file <i>@ref SDK_EVAL_VC_Prop.h</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_Properties_Exported_Types   SDK EVAL VC Properties Exported Types
 * @{
 */

/**
 *@brief USB line coding structure
 */
typedef struct
{
  uint32_t bitrate;
  uint8_t format;
  uint8_t paritytype;
  uint8_t datatype;
}LineCoding;


#define SdkEvalVCPortGetConfiguration          NOP_Process
/*      #define SdkEvalVCPortSetConfiguration          NOP_Process    */
#define SdkEvalVCPortGetInterface              NOP_Process
#define SdkEvalVCPortSetInterface              NOP_Process
#define SdkEvalVCPortGetStatus                 NOP_Process
#define SdkEvalVCPortClearFeature              NOP_Process
#define SdkEvalVCPortSetEndPointFeature        NOP_Process
#define SdkEvalVCPortSetDeviceFeature          NOP_Process
/*      #define SdkEvalVCPortSetDeviceAddress          NOP_Process      */

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Exported_Constants   SDK EVAL VC Properties Exported Constants
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Exported_Macros    SDK EVAL VC Properties Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Properties_Exported_Functions   SDK EVAL VC Properties Exported Functions
 * @{
 */
void SdkEvalVCPortInit(void);
void SdkEvalVCPortReset(void);
void SdkEvalVCPortSetConfiguration(void);
void SdkEvalVCPortSetDeviceAddress (void);
void SdkEvalVCPortStatusIn (void);
void SdkEvalVCPortStatusOut (void);
RESULT SdkEvalVCPortDataSetup(uint8_t);
RESULT SdkEvalVCPortNoDataSetup(uint8_t);
RESULT SdkEvalVCPortGetInterfaceSetting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t* SdkEvalVCPortGetDeviceDescriptor(uint16_t );
uint8_t* SdkEvalVCPortGetConfigDescriptor(uint16_t);
uint8_t* SdkEvalVCPortGetStringDescriptor(uint16_t);
uint8_t* SdkEvalVCPortGetLineCoding(uint16_t Length);
uint8_t* SdkEvalVCPortSetLineCoding(uint16_t Length);

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
