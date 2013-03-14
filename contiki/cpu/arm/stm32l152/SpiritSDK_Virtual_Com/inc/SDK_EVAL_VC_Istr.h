/**
 * @file    SDK_EVAL_VC_Istr.h
 * @author  MSH RF/ART Team IMS-Systems Lab
 * @version V1.0.0
 * @date    August 4, 2011
 * @brief   Interrupt Status Register header.
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
#ifndef __SDK_EVAL_VC_ISTR_H
#define __SDK_EVAL_VC_ISTR_H


/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"


#ifdef __cplusplus
extern "C" {
#endif



/** @defgroup SDK_EVAL_Virtual_Com    SDK EVAL Virtual Com
 * @{
 */


/** @defgroup SDK_EVAL_VC_InterruptStatus   SDK EVAL VC InterruptStatus
 * @brief Interrupt status register module for Virtual COM Port Device.
 * @details See the file <i>@ref SDK_EVAL_VC_Istr.h</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_InterruptStatus_Exported_Types    SDK EVAL VC InterruptStatus Exported_Types
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_InterruptStatus_Exported_Constants    SDK EVAL VC InterruptStatus Exported Constants
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_InterruptStatus_Exported_Macros   SDK EVAL VC InterruptStatus Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_InterruptStatus_Exported_Functions    SDK EVAL VC InterruptStatus Exported Functions
 * @{
 */
void SdkEvalVCIntServRoutine(void);

void EP1_IN_Callback(void);
void EP2_IN_Callback(void);
void EP3_IN_Callback(void);
void EP4_IN_Callback(void);
void EP5_IN_Callback(void);
void EP6_IN_Callback(void);
void EP7_IN_Callback(void);

void EP1_OUT_Callback(void);
void EP2_OUT_Callback(void);
void EP3_OUT_Callback(void);
void EP4_OUT_Callback(void);
void EP5_OUT_Callback(void);
void EP6_OUT_Callback(void);
void EP7_OUT_Callback(void);

#ifdef CTR_CALLBACK
void CTR_Callback(void);
#endif

#ifdef DOVR_CALLBACK
void DOVR_Callback(void);
#endif

#ifdef ERR_CALLBACK
void ERR_Callback(void);
#endif

#ifdef WKUP_CALLBACK
void WKUP_Callback(void);
#endif

#ifdef SUSP_CALLBACK
void SUSP_Callback(void);
#endif

#ifdef RESET_CALLBACK
void RESET_Callback(void);
#endif

#ifdef SOF_CALLBACK
void SOF_Callback(void);
#endif

#ifdef ESOF_CALLBACK
void ESOF_Callback(void);
#endif

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
