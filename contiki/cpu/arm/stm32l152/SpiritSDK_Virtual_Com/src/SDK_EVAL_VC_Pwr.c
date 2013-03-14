/**
 * @file     SDK_EVAL_VC_Pwr.c
 * @author   MSH RF/ART Team IMS-Systems Lab
 * @version  V1.0.0
 * @date     August 4, 2011
 * @brief    Connection/disconnection & power management.
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
#include "stm32l1xx.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "SDK_EVAL_VC_Pwr.h"
#include "SDK_EVAL_VC_General.h"



/** @addtogroup SDK_EVAL_Virtual_Com
 * @{
 */


/** @addtogroup SDK_EVAL_VC_Power
 * @{
 */


/** @defgroup SDK_EVAL_VC_Power_Private_TypesDefinitions    SDK EVAL VC Power Private TypesDefinitions
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Private_Defines   SDK EVAL VC Power Private Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Private_Macros    SDK EVAL VC Power Private Macros
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Private_Variables   SDK EVAL VC Power Private Variables
 * @{
 */

__IO uint32_t bDeviceState = UNCONNECTED; /*!< USB device status */
__IO bool fSuspendEnabled = TRUE;  /*!< true when suspend is possible */

struct
{
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
}ResumeS;

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Private_FunctionPrototypes    SDK EVAL VC Power Private FunctionPrototypes
 * @{
 */

/**
 * @}
 */


/** @defgroup SDK_EVAL_VC_Power_Private_Functions   SDK EVAL VC Power Private Functions
 * @{
 */

/**
 * @brief  Handles switch-on conditions.
 * @param  None.
 * @retval RESULT: USB_SUCCESS.
 */
RESULT SdkEvalVCPowerOn(void)
{
  uint16_t wRegVal;

  /*** cable plugged-in ? ***/
  SdkEvalVCCableConfig(ENABLE);

  /*** CNTR_PWDN = 0 ***/
  wRegVal = CNTR_FRES;
  _SetCNTR(wRegVal);

  /*** CNTR_FRES = 0 ***/
  wInterrupt_Mask = 0;
  _SetCNTR(wInterrupt_Mask);

  /*** Clear pending interrupts ***/
  _SetISTR(0);

  /*** Set interrupt mask ***/
  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
  _SetCNTR(wInterrupt_Mask);
  
  return USB_SUCCESS;

}


/**
 * @brief  Handles switch-off conditions.
 * @param  None.
 * @retval RESULT: USB_SUCCESS.
 */
RESULT SdkEvalVCPowerOff(void)
{
  /* disable all interrupts and force USB reset */
  _SetCNTR(CNTR_FRES);
  /* clear interrupt status register */
  _SetISTR(0);
  /* Disable the Pull-Up*/
  SdkEvalVCCableConfig(DISABLE);
  /* switch-off device */
  _SetCNTR(CNTR_FRES + CNTR_PDWN);

  /* sw variables reset */
  /* ... */

  return USB_SUCCESS;

}


/**
 * @brief  Sets suspend mode operating conditions.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCSuspend(void)
{
  uint16_t wCNTR;
  /* suspend preparation */
  /* ... */

  /* macrocell enters suspend mode */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_FSUSP;
  _SetCNTR(wCNTR);

  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* power reduction */
  /* ... on connected devices */

  /* force low-power mode in the macrocell */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_LPMODE;
  _SetCNTR(wCNTR);

  /* switch-off the clocks */
  /* ... */
  SdkEvalVCEnterLowPowerMode();

}


/**
 * @brief  Handles wake-up restoring normal operations.
 * @param  None.
 * @retval None.
 */
void SdkEvalVCResumeInit(void)
{
  uint16_t wCNTR;

  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* restart the clocks */
  /* ...  */

  /* CNTR_LPMODE = 0 */
  wCNTR = _GetCNTR();
  wCNTR &= (~CNTR_LPMODE);
  _SetCNTR(wCNTR);

  /* restore full power */
  /* ... on connected devices */
  SdkEvalVCLeaveLowPowerMode();

  /* reset FSUSP bit */
  _SetCNTR(IMR_MSK);

  /* reverse suspend preparation */
  /* ... */

}


/**
 * @brief  This is the state machine handling resume operations and
 *         timing sequence. The control is based on the SdkEvalVCResume structure
 *         variables and on the ESOF interrupt calling this subroutine
 *         without changing machine state.
 * @param  xResumeSetVal: a state machine value (RESUME_STATE)
 *         RESUME_ESOF doesn't change ResumeS.eState allowing
 *         decrementing of the ESOF counter in different states.
 * @retval None.
 */
void SdkEvalVCResume(RESUME_STATE xResumeSetVal)
{
  uint16_t wCNTR;

  if (xResumeSetVal != RESUME_ESOF)
    ResumeS.eState = xResumeSetVal;

  switch (ResumeS.eState)
  {
  case RESUME_EXTERNAL:
    SdkEvalVCResumeInit();
    ResumeS.eState = RESUME_OFF;
    break;
  case RESUME_INTERNAL:
    SdkEvalVCResumeInit();
    ResumeS.eState = RESUME_START;
    break;
  case RESUME_LATER:
    ResumeS.bESOFcnt = 2;
    ResumeS.eState = RESUME_WAIT;
    break;
  case RESUME_WAIT:
    ResumeS.bESOFcnt--;
    if (ResumeS.bESOFcnt == 0)
      ResumeS.eState = RESUME_START;
    break;
  case RESUME_START:
    wCNTR = _GetCNTR();
    wCNTR |= CNTR_RESUME;
    _SetCNTR(wCNTR);
    ResumeS.eState = RESUME_ON;
    ResumeS.bESOFcnt = 10;
    break;
  case RESUME_ON:
    ResumeS.bESOFcnt--;
    if (ResumeS.bESOFcnt == 0)
    {
      wCNTR = _GetCNTR();
      wCNTR &= (~CNTR_RESUME);
      _SetCNTR(wCNTR);
      ResumeS.eState = RESUME_OFF;
    }
    break;
  case RESUME_OFF:
  case RESUME_ESOF:
  default:
    ResumeS.eState = RESUME_OFF;
    break;
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
