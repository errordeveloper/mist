/*
 * Copyright (c) 2012, Thingsquare, www.thingsquare.com.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of Thingsquare Mist.
 */

/*
 *  \file
 *      rtimer-arch.c
 *   \author
 *      Marcus Lunden <marcus@thingsquare.com>>
 *   \desc
 *      rtimer arch implementation for STM32L+spirit1, using Timer X
 */

#include "contiki.h"
#include "platform-conf.h"

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "sys/process.h"
#include "dev/watchdog.h"

#include "stm32l1xx.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_tim.h"
#include "stm32l1xx_it.h"
#include "misc.h"
/*---------------------------------------------------------------------------*/
/* some debug features */
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...)     printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/* XXX NB: rtimer now works with busywait_until() macro, but not setting any
  rtimer interrupts, and it's not super-precise either (on 500ms it takes 500.07ms
  and on 1 ms it takes 0.9777 ms. */

/* the prescaler assumes the timer is sourced with same clock speed as the CPU*/
#define PRESCALER       ((F_CPU / (RTIMER_SECOND*2)))

volatile uint32_t rtimer_clock = 0uL;
/*---------------------------------------------------------------------------*/
void
TIM2_IRQHandler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  
  /* clear interrupt pending flag */
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

  rtimer_clock++;
  
  /* check for, and run, any pending rtimers */
  // XXX no, later, when we have implemented rtimer timer expiration properly
  //rtimer_run_next();

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);

}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  /* to init, we use structures that are filled in and passed to the HAL */
  TIM_TimeBaseInitTypeDef tim_init_s;
  TIM_OCInitTypeDef       tim_ocinit_s;
  NVIC_InitTypeDef        nvic_init_s;

  /* Enable TIM2 peripheral clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* configure TIM2 */
  tim_init_s.TIM_Prescaler = PRESCALER;
  tim_init_s.TIM_CounterMode = TIM_CounterMode_Up;
  tim_init_s.TIM_ClockDivision = 0;
  tim_init_s.TIM_Period = 1;
/*  tim_init_s.TIM_Period = RTIMER_SECOND/2;*/
  TIM_TimeBaseInit(TIM2, &tim_init_s);

  /* Configure the timer for output compare interrupt */
  TIM_OCStructInit(&tim_ocinit_s);
  tim_ocinit_s.TIM_OCMode = TIM_OCMode_Timing;
  tim_ocinit_s.TIM_Pulse = 0;
/*  tim_ocinit_s.TIM_OutputState = TIM_OutputState_Enable;*/
/*  tim_ocinit_s.TIM_OCPolarity = TIM_OCPolarity_High;*/
  TIM_OC1Init(TIM2, &tim_ocinit_s);

  /* Immediate load of TIM2 Precaler value */
/*  TIM_PrescalerConfig(TIM2, (F_CPU/RTIMER_SECOND), TIM_PSCReloadMode_Immediate);*/
/*  TIM_PrescalerConfig(TIM2, ((SystemCoreClock/1200) - 1), TIM_PSCReloadMode_Immediate);*/

  /* Clear TIM2 update pending flag */
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);

  /* Enable TIM2 Update interrupt */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  TIM_Cmd(TIM2, ENABLE);

  /* Enable the TIM2 Interrupt in the NVIC */
  nvic_init_s.NVIC_IRQChannel = TIM2_IRQn;
  nvic_init_s.NVIC_IRQChannelPreemptionPriority = 0;
  nvic_init_s.NVIC_IRQChannelSubPriority = 0;
  nvic_init_s.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic_init_s);
}
/*---------------------------------------------------------------------------*/
rtimer_clock_t
rtimer_arch_now(void)
{
#if 0
  rtimer_clock_t t1, t2;
  do {
    t1 = rtimer_clock;
    t2 = rtimer_clock;
  } while(t1 != t2);
#endif    /* if 0; code commented out */
  return rtimer_clock;
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  PRINTF("rtimer_arch_schedule time %u; not yet implemented\n", t);
}
/*---------------------------------------------------------------------------*/
