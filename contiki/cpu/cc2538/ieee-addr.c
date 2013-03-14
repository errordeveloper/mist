/*
 * Copyright (c) 2013, Texas Instruments Incorporated - http://www.ti.com/
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup cc2538-ieee-addr
 * @{
 *
 * \file
 * Driver for the cc2538 IEEE addresses
 */
#include "contiki-conf.h"
#include "net/rime/rimeaddr.h"
#include "ieee-addr.h"

#include <stdint.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
#if IEEE_CONF_ADDR_LOCATION==IEEE_ADDR_USE_HARDCODED
#define IEEE_ADDR (ieee_addr_hc)
#elif IEEE_CONF_ADDR_LOCATION==IEEE_ADDR_USE_SECONDARY
#define IEEE_ADDR ((uint8_t *)IEEE_ADDR_LOCATION_SECONDARY)
#else
#define IEEE_ADDR ((uint8_t *)IEEE_ADDR_LOCATION_PRIMARY)
#endif
/*---------------------------------------------------------------------------*/
void
ieee_addr_cpy_to(uint8_t *dst, uint8_t len)
{
#if IEEE_CONF_ADDR_LOCATION==IEEE_ADDR_USE_HARDCODED
  uint8_t ieee_addr_hc[8] = { 0x00, 0x12, 0x4B, 0x00, 0x89, 0xAB, 0xCD, 0xEF };
#endif

  memcpy(dst, &IEEE_ADDR[8 - len], len);

#if IEEE_ADDR_NODE_ID
  dst[len - 1] = IEEE_ADDR_NODE_ID & 0xFF;
  dst[len - 2] = IEEE_ADDR_NODE_ID >> 8;
#endif
}

/** @} */
