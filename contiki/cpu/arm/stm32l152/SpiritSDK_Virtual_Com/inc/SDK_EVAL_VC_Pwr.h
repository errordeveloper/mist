/**
 * @file    SDK_EVAL_VC_Pwr.h
 * @author  MSH RF/ART Team IMS-Systems Lab
 * @version V1.0.0
 * @date    August 4, 2011
 * @brief   Connection/disconnection & power management header.
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
#ifndef __SDK_EVAL_VC_PWR_H
#define __SDK_EVAL_VC_PWR_H


#ifdef __cplusplus
extern "C" {
#endif




/** @defgroup SDK_EVAL_Virtual_Com    SDK EVAL Virtual Com
 * @{
 */


/** @defgroup SDK_EVAL_VC_Power   SDK EVAL VC Power
 * @brief Connection/disconnection & power management module for Virtual COM Port Device.
 * @details See the file <i>@ref SDK_EVAL_VC_Pwr.h</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_Power_Exported_Types    SDK EVAL VC Power Exported Types
 * @{
 */
typedef enum _RESUME_STATE
{
  RESUME_EXTERNAL,
  RESUME_INTERNAL,
  RESUME_LATER,
  RESUME_WAIT,
  RESUME_START,
  RESUME_ON,
  RESUME_OFF,
  RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE
{
  UNCONNECTED,
  ATTACHED,
  POWERED,
  SUSPENDED,
  ADDRESSED,
  CONFIGURED
} DEVICE_STATE;

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Exported_Constants    SDK EVAL VC Power Exported Constants
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Exported_Macros   SDK EVAL VC Power Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_External_Variables    SDK EVAL VC Power External Variables
 * @{
 */
extern  __IO uint32_t bDeviceState; /*!< USB device status */
extern __IO bool fSuspendEnabled;  /*!< true when suspend is possible */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Exported_Functions    SDK EVAL VC Power Exported Functions
 * @{
 */
void SdkEvalVCSuspend(void);
void SdkEvalVCResumeInit(void);
void SdkEvalVCResume(RESUME_STATE eResumeSetVal);
RESULT SdkEvalVCPowerOn(void);
RESULT SdkEvalVCPowerOff(void);

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
