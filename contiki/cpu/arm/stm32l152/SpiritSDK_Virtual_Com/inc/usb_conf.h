/**
 * @file    usb_conf.h
 * @author  MSH RF/ART Team IMS-Systems Lab
 * @version V1.0.0
 * @date    August 4, 2011
 * @brief   Virtual Com Configuration Header.
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
#ifndef __USB_CONF_H
#define __USB_CONF_H


#ifdef __cplusplus
extern "C" {
#endif




/** @defgroup SDK_EVAL_Virtual_Com    SDK EVAL Virtual Com
 * @{
 */


/** @defgroup SDK_EVAL_VC_Configuration   SDK EVAL VC Configuration
 * @brief Configuration Header for Virtual COM Port Device.
 * @details See the file <i>@ref usb_conf.h</i> for more details.
 * @{
 */


/** @defgroup SDK_EVAL_VC_Configuration_Exported_Types    SDK EVAL VC Configuration Exported Types
 * @{
 */

/**
 * @brief  EP_NUM: defines how many endpoints are used by the device.
 */
#define EP_NUM              (4)


/**
 * @brief  Buffer table base address.
 */
#define BTABLE_ADDRESS      (0x00)


/**
 * @brief  EP0: rx/tx buffer base address.
 */
#define ENDP0_RXADDR        (0x40)
#define ENDP0_TXADDR        (0x80)


/**
 * @brief  EP1: tx buffer base address.
 */
#define ENDP1_TXADDR        (0xC0)


/**
 * @brief  EP2: tx buffer base address.
 */
#define ENDP2_TXADDR        (0x100)


/**
 * @brief  EP3: rx buffer base address.
 */
#define ENDP3_RXADDR        (0x110)


/**
 * @brief  IMR_MSK: mask defining which events has to be handled by the device
 *         application software.
 */
#define IMR_MSK (CNTR_CTRM  | CNTR_SOFM  | CNTR_RESETM )


/**
 * @brief  Callback used in application
 */
/*#define CTR_CALLBACK*/
/*#define DOVR_CALLBACK*/
/*#define ERR_CALLBACK*/
/*#define WKUP_CALLBACK*/
/*#define SUSP_CALLBACK*/
/*#define RESET_CALLBACK*/
#define SOF_CALLBACK
/*#define ESOF_CALLBACK*/


/**
 * @brief  CTR service routines associated to defined endpoints.
 */
//#define  EP1_IN_Callback   NOP_Process
#define  EP2_IN_Callback   NOP_Process
#define  EP3_IN_Callback   NOP_Process
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

#define  EP1_OUT_Callback   NOP_Process
#define  EP2_OUT_Callback   NOP_Process
/*#define  EP3_OUT_Callback   NOP_Process*/
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process


/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Configuration_Exported_Constants    SDK EVAL VC Configuration Exported Constants
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Configuration_Exported_Macros   SDK EVAL VC Configuration Exported Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Configuration_Exported_Functions    SDK EVAL VC Configuration Exported Functions
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
