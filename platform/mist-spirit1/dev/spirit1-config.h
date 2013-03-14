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
/*
 *  \file
 *      spirit1-config.h
 *   \author
 *      Marcus Lunden <marcus@thingsquare.com>>
 *   \desc
 *      configuration for the Spirit1 radio transceiver
 *      
 */

#ifndef __SPIRIT1_CONFIG_H__
#define __SPIRIT1_CONFIG_H__

#include "radio.h"
#include "SPIRIT_Config.h"
#include "spirit1-const.h"

   
/**    
 * The TX_WAIT_PCKT_PERIOD should equal the max packet tx time.
 */   
#define TX_WAIT_PCKT_PERIOD         50  
   
/**    
 * The RX_WAIT_ACK_PERIOD is the period within which the ACK packet must be received. 
 * ca PACKET_LENGTH / DATA_RATE + ELABORATION_TIME + SPI_OPERATIONS_USED. 
 */      
#define RX_WAIT_ACK_PERIOD          50

#define CCA_THRESHOLD               -90.0   /* dBm */
#define XTAL_FREQUENCY              50000000    /* Hz */
#define XTAL_OFFSET_PPM             0
#define BASE_FREQUENCY              868.0e6
#define CHANNEL_SPACE               20e3
#define CHANNEL_NUMBER              0
#define MODULATION_SELECT           FSK
#define DATARATE                    38400
#define FREQ_DEVIATION              20e3
#define BANDWIDTH                   100.5E3
#define POWER_DBM                   10.0
#define PREAMBLE_LENGTH             PKT_PREAMBLE_LENGTH_04BYTES
#define SYNC_LENGTH                 PKT_SYNC_LENGTH_4BYTES
#define SYNC_WORD                   0x88888888
#define LENGTH_TYPE                 PKT_LENGTH_VAR
#define LENGTH_WIDTH                8
#define CRC_MODE                    PKT_CRC_MODE_16BITS_2
#define CONTROL_LENGTH              PKT_CONTROL_LENGTH_0BYTES
#define EN_ADDRESS                  S_DISABLE
#define EN_FEC                      S_DISABLE
#define EN_WHITENING                S_DISABLE
#define SPIRIT_MAX_FIFO_LEN         96
   
/**    
 * The MAX_PACKET_LEN is an arbitrary value used to define the two array
 * spirit_txbuf and spirit_rxbuf.
 * The SPIRIT1 supports with its packet handler a length of 65,535 bytes,
 * and in direct mode (without packet handler) there is no limit of data.
 */  
#define MAX_PACKET_LEN              SPIRIT_MAX_FIFO_LEN

/**    
 * Spirit1 IC version
 */
#define SPIRIT1_VERSION             SPIRIT_VERSION_3_0


#endif /* __SPIRIT1_CONFIG_H__ */

