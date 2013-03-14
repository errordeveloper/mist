/*
* Copyright (c) 2012, STMicroelectronics.
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
* This file is part of the Contiki operating system.
*
*/

#include "spirit1.h"
#include "spirit1-arch.h"
#include "stm32l1xx.h"

#include "contiki.h"
#include "net/mac/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"

#include "SDK_EVAL_Spirit_Spi_Config.h"
#include "SDK_EVAL_Config.h"
#include "SDK_EVAL_VC_General.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)

/*---------------------------------------------------------------------------*/
#define CLEAR_TXBUF()           (spirit_txbuf[0] = 0)
#define CLEAR_RXBUF()           (spirit_rxbuf[0] = 0)
#define IS_TXBUF_EMPTY()        (spirit_txbuf[0] == 0)
#define IS_RXBUF_EMPTY()        (spirit_rxbuf[0] == 0)
#define IS_RXBUF_FULL()         (spirit_rxbuf[0] != 0)

/* transceiver state. */
#define ON     0
#define OFF    1

#define IRQ_ENABLE()             SdkEvalM2SGpioInterruptCmd(M2S_GPIO_3,0x0F,0x0F,ENABLE);
#define IRQ_DISABLE()            SdkEvalM2SGpioInterruptCmd(M2S_GPIO_3,0x0F,0x0F,DISABLE);
/*---------------------------------------------------------------------------*/
static volatile unsigned int spirit_on = OFF;
static volatile uint8_t receiving_packet = 0;
static uint8_t nTxIndex = 0;
static uint8_t nResidualPayloadLength = 0;

/* 
* The buffers which hold incoming data.
* The +1 because of the first byte, 
* which will contain the length of the packet.
*/
static uint8_t spirit_rxbuf[MAX_PACKET_LEN+1];
static uint8_t spirit_txbuf[MAX_PACKET_LEN+1-SPIRIT_MAX_FIFO_LEN];

#define ACK_LEN 3

static int packet_is_prepared = 0;
/*---------------------------------------------------------------------------*/
PROCESS(spirit_radio_process, "SPIRIT radio driver");
/*---------------------------------------------------------------------------*/
static int spirit_radio_init(void);
static int spirit_radio_prepare(const void *payload, unsigned short payload_len);
static int spirit_radio_transmit(unsigned short payload_len);
static int spirit_radio_send(const void *data, unsigned short len);
static int spirit_radio_read(void *buf, unsigned short bufsize);
static int spirit_radio_channel_clear(void);
static int spirit_radio_receiving_packet(void);
static int spirit_radio_pending_packet(void);
static int spirit_radio_on(void);
static int spirit_radio_off(void);
/*---------------------------------------------------------------------------*/
const struct radio_driver spirit_radio_driver =
{
  spirit_radio_init,
  spirit_radio_prepare,
  spirit_radio_transmit,
  spirit_radio_send,
  spirit_radio_read,
  spirit_radio_channel_clear,
  spirit_radio_receiving_packet,
  spirit_radio_pending_packet,
  spirit_radio_on,
  spirit_radio_off,
};
/*---------------------------------------------------------------------------*/
/* convienience macro for reading the MC_STATE[1] register from Spirit1, to be used like eg
      if(SPIRIT1_STATUS() == SPIRIT1_STATE_READY) {
      }
  or    
      BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, RTIMER_SECOND/1000);
*/
#define SPIRIT1_STATUS()       (spirit1_arch_refresh_status() & SPIRIT1_STATE_STATEBITS)

/*---------------------------------------------------------------------------*/
/* Strobe a command. The rationale for this is to clean up the messy legacy code. */
static void
spirit1_strobe(uint8_t s)
{
  SpiritCmdStrobeCommand(s);
}
/*---------------------------------------------------------------------------*/
/**
* @brief  Puts the SPIRIT1 in READY state.
* @param  None
* @retval None
*/
void SpiritSetReadyState(void)
{
  PRINTF("READY IN\n");

  SpiritIrqClearStatus();
  IRQ_DISABLE();

  if(SPIRIT1_STATUS() == SPIRIT1_STATE_STANDBY) {
    spirit1_strobe(SPIRIT1_STROBE_READY);
/*    SpiritCmdStrobeReady();*/
  } else if(SPIRIT1_STATUS() == SPIRIT1_STATE_RX) {
    spirit1_strobe(SPIRIT1_STROBE_SABORT);
/*    SpiritCmdStrobeSabort();*/
    SpiritIrqClearStatus();
  }

  IRQ_ENABLE();

  PRINTF("READY OUT\n");
}
/*---------------------------------------------------------------------------*/
static void
panic_ra(uint8_t l)
{
  volatile uint16_t i, k;
  if(l > 5) {
    l = 5;
  }
  if(l == 0) {
    l++;
  }
  while(1) {
    SdkEvalLedOn(l);
    for(i = 0; i < 0xffff; i += 1) {
      k++;
    }
    SdkEvalLedOff(l);
    for(i = 0; i < 0xffff; i += 1) {
      k++;
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_init(void)
{  
  PRINTF("RADIO INIT IN\n");

  SpiritSpiInit();
  
  /* Configure radio shut-down (SDN) pin and activate radio */
  SdkEvalM2SGpioInit(M2S_GPIO_SDN, M2S_MODE_GPIO_OUT);
  
  /* Configures the SPIRIT1 library */
  SpiritRadioSetXtalFrequency(XTAL_FREQUENCY);
  SpiritManagementSetVersion(SPIRIT1_VERSION);
  SpiritGeneralSetSpiritVersion(SPIRIT1_VERSION);
  SpiritManagementSetXtalFrequency(XTAL_FREQUENCY);
  
  /* wake up to READY state */
  /* weirdly enough, this *should* actually *set* the pin, not clear it! The pins is declared as GPIO_pin13 == 0x2000 */
  M2S_GPIO_SDN_PORT->BSRRL = M2S_GPIO_SDN_PIN;
/*  GPIOC->BSRRL = M2S_GPIO_SDN_PIN;*/
  GPIO_ResetBits(M2S_GPIO_SDN_PORT, M2S_GPIO_SDN_PIN);

  /* wait minimum 1.5 ms to allow SPIRIT1 a proper boot-up sequence */
  BUSYWAIT_UNTIL(0, 3 * RTIMER_SECOND/2000);
  
  /* Soft reset of core */
  spirit1_strobe(SPIRIT1_STROBE_SRES);
/*  SpiritCmdStrobeSres();*/
  
  /* Configures the SPIRIT1 radio part */
  SRadioInit xRadioInit = {
    XTAL_FREQUENCY,
    XTAL_OFFSET_PPM,
    BASE_FREQUENCY,
    CHANNEL_SPACE,
    CHANNEL_NUMBER,
    MODULATION_SELECT,
    DATARATE,
    FREQ_DEVIATION,
    BANDWIDTH
  };
  SpiritRadioInit(&xRadioInit);
  SpiritRadioSetPALeveldBm(0,POWER_DBM);
  SpiritRadioSetPALevelMaxIndex(0);
  
  /* Configures the SPIRIT1 packet handler part*/
  PktBasicInit xBasicInit = {
    PREAMBLE_LENGTH,
    SYNC_LENGTH,
    SYNC_WORD,
    LENGTH_TYPE,
    LENGTH_WIDTH,
    CRC_MODE,
    CONTROL_LENGTH,
    EN_ADDRESS,
    EN_FEC,
    EN_WHITENING
  };
  SpiritPktBasicInit(&xBasicInit);
  
  /* Enable the following interrupt sources, routed to GPIO */
  SpiritIrqDeInit(NULL);
  SpiritIrqClearStatus();
  SpiritIrq(TX_DATA_SENT, S_ENABLE);
  SpiritIrq(RX_DATA_READY,S_ENABLE);
  SpiritIrq(VALID_SYNC,S_ENABLE);
  SpiritIrq(RX_DATA_DISC, S_ENABLE);
  
  /* Configure Spirit1 */
  SpiritRadioPersistenRx(S_ENABLE);
  SpiritQiSetSqiThreshold(SQI_TH_0);
  SpiritQiSqiCheck(S_ENABLE);
  SpiritQiSetRssiThresholddBm(CCA_THRESHOLD);
  SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);
  SET_INFINITE_RX_TIMEOUT();
  SpiritRadioAFCFreezeOnSync(S_ENABLE);
  
  /* Puts the SPIRIT1 in STANDBY mode (125us -> rx/tx) */
  spirit1_strobe(SPIRIT1_STROBE_STANDBY);
/*  SpiritCmdStrobeCommand(SPIRIT1_STROBE_STANDBY);*/
/*  SpiritCmdStrobeStandby();*/
  spirit_on = OFF;
  CLEAR_RXBUF();
  CLEAR_TXBUF();
  
  /* Initializes the mcu pin as input, used for IRQ */
  SdkEvalM2SGpioInit(M2S_GPIO_0, M2S_MODE_EXTI_IN);
  SdkEvalM2SGpioInit(M2S_GPIO_1, M2S_MODE_EXTI_IN);
  SdkEvalM2SGpioInit(M2S_GPIO_2, M2S_MODE_EXTI_IN);
  SdkEvalM2SGpioInit(M2S_GPIO_3, M2S_MODE_EXTI_IN);
  
  /* Configure the radio to route the IRQ signal to its GPIO 3 */
  SpiritGpioInit(&(SGpioInit){SPIRIT_GPIO_3, SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP, SPIRIT_GPIO_DIG_OUT_IRQ});

  process_start(&spirit_radio_process, NULL);

  PRINTF("Spirit1 init done\n");

  return 0;
}

/*---------------------------------------------------------------------------*/
static int
spirit_radio_prepare(const void *payload, unsigned short payload_len)
{
  PRINTF("Spirit1: prep %u\n", payload_len);
  packet_is_prepared = 0;

  /* Checks if the payload length is supported */
  if(payload_len > MAX_PACKET_LEN) {
    return RADIO_TX_ERR;
  }
  
  /* Waits the end of a still running spirit_radio_transmit */
  BUSYWAIT_UNTIL(IS_TXBUF_EMPTY(), TX_WAIT_PCKT_PERIOD * RTIMER_SECOND/1000);
  if(!IS_TXBUF_EMPTY()) {
    PRINTF("PREPARE OUT ERR\n");
    return RADIO_TX_ERR;
  }
  
  /* Sets the length of the packet to send */
  SpiritPktBasicSetPayloadLength(payload_len);
  SpiritSpiWriteLinearFifo(payload_len, (uint8_t *)payload);
  
  PRINTF("PREPARE OUT\n");

  packet_is_prepared = 1;
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_transmit(unsigned short payload_len)
{ 
  PRINTF("TRANSMIT IN\n");
  if(!packet_is_prepared) {
    return RADIO_TX_ERR;
  }

  /* Stores the length of the packet to send */
  /* Others spirit_radio_prepare will be in hold */
  spirit_txbuf[0] = payload_len;
  
  /* Puts the SPIRIT1 in TX state */
  SpiritSetReadyState(); 
  spirit1_strobe(SPIRIT1_STROBE_TX);
/*  SpiritCmdStrobeTx();*/

  /* Busywait until tx done */
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_TX, 5 * RTIMER_SECOND/1000);
  BUSYWAIT_UNTIL(SPIRIT1_STATUS() != SPIRIT1_STATE_TX, 50 * RTIMER_SECOND/1000);
  BUSYWAIT_UNTIL(0, RX_WAIT_ACK_PERIOD * RTIMER_SECOND/1000)

  PRINTF("TRANSMIT OUT\n");

  CLEAR_TXBUF();
  packet_is_prepared = 0;
  return RADIO_TX_OK;
}
/*---------------------------------------------------------------------------*/
static int spirit_radio_send(const void *payload, unsigned short payload_len)
{
  if(spirit_radio_prepare(payload, payload_len) == RADIO_TX_ERR) {
    return RADIO_TX_ERR;
  } 
  return spirit_radio_transmit(payload_len);
  
}
/*---------------------------------------------------------------------------*/
static int spirit_radio_read(void *buf, unsigned short bufsize)
{  
  PRINTF("READ IN\n");
  
  /* Checks if the RX buffer is empty */
  if(IS_RXBUF_EMPTY()) { 
    PRINTF("READ OUT RX BUF EMPTY\n");
    return 0;
  }
  
  if(bufsize < spirit_rxbuf[0]) { 
    /* If buf has the correct size */
    PRINTF("TOO SMALL BUF\n");
    return 0;
  } else {
    /* Copies the packet received */
    memcpy(buf, spirit_rxbuf + 1, spirit_rxbuf[0]);

    bufsize = spirit_rxbuf[0];
    CLEAR_RXBUF();
    
    PRINTF("READ OUT\n");
    
    return bufsize;
  }  
  
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_channel_clear(void)
{
  float rssi_value;
  /* Local variable used to memorize the SPIRIT1 state */
  uint8_t spirit_state = ON;
  
  PRINTF("CHANNEL CLEAR IN\n");
  
  if(spirit_on == OFF) {
    /* Wakes up the SPIRIT1 */
    spirit_radio_on();
    spirit_state = OFF;
  }
  
  /*  */
  IRQ_DISABLE();
  spirit1_strobe(SPIRIT1_STROBE_SABORT);
/*  SpiritCmdStrobeSabort();*/
  SpiritIrqClearStatus();
  IRQ_ENABLE();
  {
    rtimer_clock_t timeout = RTIMER_NOW() + 5 * RTIMER_SECOND/1000;
    do {
      SpiritRefreshStatus();
    } while((g_xStatus.MC_STATE != MC_STATE_READY) && (RTIMER_NOW() < timeout));
    if(RTIMER_NOW() < timeout) {
      return 1;
    }
  }

  /* Stores the RSSI value */
  rssi_value = SpiritQiGetRssidBm();
  
  /* Puts the SPIRIT1 in its previous state */
  if(spirit_state==OFF) {
    spirit_radio_off();
  } else {
    spirit1_strobe(SPIRIT1_STROBE_RX);
/*    SpiritCmdStrobeRx();*/
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 5 * RTIMER_SECOND/1000);
  }
  
  PRINTF("CHANNEL CLEAR OUT\n");
  
  /* Checks the RSSI value with the threshold */
  if(rssi_value<CCA_THRESHOLD) {
    return 0;
  } else {
    return 1;
  }
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_receiving_packet(void)
{
  return receiving_packet;
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_pending_packet(void)
{
  PRINTF("PENDING PACKET\n");
  return !IS_RXBUF_EMPTY();
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_off(void)
{  
  PRINTF("Spirit1: ->off\n");
  if(spirit_on == ON) {
    /* Disables the mcu to get IRQ from the SPIRIT1 */
    IRQ_DISABLE();

    /* first stop rx/tx */
    spirit1_strobe(SPIRIT1_STROBE_SABORT);

    /* Clear any pending irqs */
    SpiritIrqClearStatus();
    
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 5 * RTIMER_SECOND/1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_READY) {
      PRINTF("Spirit1: failed off->ready\n");
      return 1;
    }

    /* Puts the SPIRIT1 in STANDBY */
    spirit1_strobe(SPIRIT1_STROBE_STANDBY);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_STANDBY, 5 * RTIMER_SECOND/1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_STANDBY) {
      PRINTF("Spirit1: failed off->stdby\n");
      return 1;
    }

    spirit_on = OFF;
    CLEAR_TXBUF();
    CLEAR_RXBUF(); 
  }
  PRINTF("Spirit1: off.\n");
  return 0; 
}
/*---------------------------------------------------------------------------*/
static int
spirit_radio_on(void)
{
  PRINTF("Spirit1: on\n");
  if(spirit_on == OFF) {
    IRQ_DISABLE();

    /* ensure we are in READY state as we go from there to Rx */
    spirit1_strobe(SPIRIT1_STROBE_READY);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_READY, 5 * RTIMER_SECOND/1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_READY) {
      PRINTF("Spirit1: failed to turn on\n");
      panic_ra(4);
      return 1;
    }

    /* now we go to Rx */
    spirit1_strobe(SPIRIT1_STROBE_RX);
    BUSYWAIT_UNTIL(SPIRIT1_STATUS() == SPIRIT1_STATE_RX, 5 * RTIMER_SECOND/1000);
    if(SPIRIT1_STATUS() != SPIRIT1_STATE_RX) {
      PRINTF("Spirit1: failed to enter rx\n");
      panic_ra(5);
      return 1;
    }
    
    /* Enables the mcu to get IRQ from the SPIRIT1 */
    IRQ_ENABLE();
    spirit_on = ON;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int interrupt_callback_in_progress = 0;
static int interrupt_callback_wants_poll = 0;
PROCESS_THREAD(spirit_radio_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Spirit1: process started\n");
  
  while(1) {
    int len;

    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);    
    PRINTF("Spirit1: polled\n");

    packetbuf_clear();
    len = spirit_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);

    if(len > 0) {
      packetbuf_set_datalen(len);   
      NETSTACK_RDC.input();
    }
    if(!IS_RXBUF_EMPTY()){
      process_poll(&spirit_radio_process);
    }

    if(interrupt_callback_wants_poll) {
      spirit1_interrupt_callback();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
spirit1_interrupt_callback(void)
{
#define INTPRINTF // PRINTF
  SpiritIrqs xIrqStatus;

  if (SpiritSPIBusy() || interrupt_callback_in_progress) {
    process_poll(&spirit_radio_process);
    interrupt_callback_wants_poll = 1;
    return;
  }
  interrupt_callback_wants_poll = 0;
  interrupt_callback_in_progress = 1;

  /* get interrupt source from radio */
  SpiritIrqGetStatus(&xIrqStatus);
  SpiritIrqClearStatus();

  /* The IRQ_VALID_SYNC is used to notify a new packet is coming */
  if(xIrqStatus.IRQ_VALID_SYNC) {
    INTPRINTF("SYNC\n");
    receiving_packet = 1;
  }

  /* The IRQ_TX_DATA_SENT notifies the packet received. Puts the SPIRIT1 in RX */
  if(xIrqStatus.IRQ_TX_DATA_SENT) {
    spirit1_strobe(SPIRIT1_STROBE_RX);
/*    SpiritCmdStrobeRx();*/
    INTPRINTF("SENT\n");
    CLEAR_TXBUF();
    interrupt_callback_in_progress = 0;
    return;
  }

  /* The IRQ_RX_DATA_READY notifies a new packet arrived */
  if(xIrqStatus.IRQ_RX_DATA_READY) {
    SpiritSpiReadLinearFifo(SpiritLinearFifoReadNumElementsRxFifo(), &spirit_rxbuf[1]);
    spirit_rxbuf[0] = SpiritPktBasicGetReceivedPktLength();

    spirit1_strobe(SPIRIT1_STROBE_FRX);
/*    SpiritCmdStrobeFlushRxFifo();*/
    INTPRINTF("RECEIVED\n");

   	process_poll(&spirit_radio_process);

    receiving_packet = 0;
    
    /* Checks if the packet has an ACK request */
#if NULLRDC_CONF_802154_AUTOACK
    frame802154_t info154;
    if(spirit_rxbuf[0] > ACK_LEN &&
        frame802154_parse(&spirit_rxbuf[1], spirit_rxbuf[0], &info154) != 0) {
      if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&
          info154.fcf.ack_required != 0 &&
          rimeaddr_cmp((rimeaddr_t *)&info154.dest_addr,
                       &rimeaddr_node_addr)) {

        /* Sends the ACK packet */
        uint8_t ack_frame[ACK_LEN] = {
            FRAME802154_ACKFRAME,
            0x00,
            info154.seq
        };
        /* XXX Verify ack timing */
        spirit1_strobe(SPIRIT1_STROBE_FTX);
        packet_is_prepared = 0;
        SpiritPktBasicSetPayloadLength(ACK_LEN);
        SpiritSpiWriteLinearFifo(ACK_LEN, ack_frame);
        SpiritSetReadyState();
        spirit1_strobe(SPIRIT1_STROBE_TX);
      }
    }
#endif

    interrupt_callback_in_progress = 0;
    return;
  }
  
  if(xIrqStatus.IRQ_RX_DATA_DISC) {
    spirit1_strobe(SPIRIT1_STROBE_FRX);
    CLEAR_RXBUF();
    INTPRINTF("DISC\n");
    receiving_packet = 0;
  }

  interrupt_callback_in_progress = 0;
}

/*---------------------------------------------------------------------------*/
