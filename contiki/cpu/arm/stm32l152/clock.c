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
 *      clock.c
 *   \author
 *      Marcus Lunden <marcus@thingsquare.com>>
 *   \desc
*       Contiki slow clock implementation.
 *      Note that it uses the Cortex-M3 SysTick, which runs on the processor
 *      clock so entering any LPM where CPU is stopped, so will clock be, which
 *      is bad.
 */

#include <stdio.h>
#include "contiki.h"
#include "platform-conf.h"
#include "contiki-conf.h"
#include "dev/leds.h"
#include "stm32l1xx.h"
#include "stm32l1xx_systick.h"
#include "stm32l1xx_rcc.h"

/*---------------------------------------------------------------------------*/
#define RELOAD_VALUE        ((F_CPU/CLOCK_CONF_SECOND) - 1)

static volatile unsigned long seconds = 0;
static volatile clock_time_t ticks;
/*---------------------------------------------------------------------------*/
void
SysTick_Handler(void)
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ticks++;
  if((ticks % CLOCK_SECOND) == 0) {
    seconds++;
    energest_flush();
  }
  if(etimer_pending()) {
    etimer_request_poll();
  }

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/
void
clock_init(void)
{ 
  ticks = 0;
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  SysTick_SetReload(RELOAD_VALUE);
  SysTick_ITConfig(ENABLE);
  SysTick_CounterCmd(SysTick_Counter_Enable);
}
/*---------------------------------------------------------------------------*/
unsigned long 
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
void
clock_set_seconds(unsigned long sec)
{
  seconds = sec;
}
/*---------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  return ticks;
}
/*---------------------------------------------------------------------------*/
/**
 * busy-wait the CPU for a duration depending on CPU speed.
 */
void
clock_delay(unsigned int i)
{
  for(; i > 0; i--) {
    unsigned int j;
    for(j = 50; j > 0; j--) {
      asm ("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of clock ticks (7.8 ms at 128 Hz).
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;
  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
