/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
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
 *
 */

#include "contiki.h"
#include "contiki-net.h"

#include "net/rpl/rpl.h"

#include "simple-rpl.h"

#define DEBUG DEBUG_FULL//NONE
#include "net/uip-debug.h"

#define RPL_DAG_GRACE_PERIOD (CLOCK_SECOND * 60 * 1)

static struct uip_ds6_notification n;
static uint8_t to_become_root;
static struct ctimer c;
/*---------------------------------------------------------------------------*/
static void
create_dag_callback(void *ptr)
{
  if(to_become_root) {
    simple_rpl_init_dag_immediately();
    to_become_root = 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr,
               int numroutes)
{
  if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    if(to_become_root) {
      ctimer_set(&c, 0, create_dag_callback, NULL);
      /*      simple_rpl_init_dag_immediately();
              to_become_root = 0;*/
    }
  }
}
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
void
simple_rpl_init(void)
{
  to_become_root = 0;
  set_global_address();
  uip_ds6_notification_add(&n, route_callback);
}
/*---------------------------------------------------------------------------*/
void
simple_rpl_init_dag_immediately(void)
{
  struct uip_ds6_addr *root_if;
  int i;
  uint8_t state;
  uip_ipaddr_t *ipaddr = NULL;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       state == ADDR_PREFERRED) {
      ipaddr = &uip_ds6_if.addr_list[i].ipaddr;
    }
  }

  if(ipaddr != NULL) {
    root_if = uip_ds6_addr_lookup(ipaddr);
    if(root_if != NULL) {
      rpl_dag_t *dag;
      uip_ipaddr_t prefix;

      rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
      dag = rpl_get_any_dag();

      /* If there are routes in this dag, we remove them all as we are
         from now on the new dag root and the old routes are wrong */
      rpl_remove_routes(dag);
      if(dag->instance != NULL &&
         dag->instance->def_route != NULL) {
	uip_ds6_defrt_rm(dag->instance->def_route);
        dag->instance->def_route = NULL;
      }

      uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
      rpl_set_prefix(dag, &prefix, 64);
      PRINTF("simple_rpl_init_dag: created a new RPL dag\n");
    } else {
      PRINTF("simple_rpl_init_dag: failed to create a new RPL DAG\n");
    }
  } else {
    PRINTF("simple_rpl_init_dag: failed to create a new RPL DAG, no preferred IP address found\n");
  }
}
/*---------------------------------------------------------------------------*/
void
simple_rpl_init_dag(void)
{
  ctimer_set(&c, RPL_DAG_GRACE_PERIOD, create_dag_callback, NULL);
  to_become_root = 1;
}
/*---------------------------------------------------------------------------*/
void
simple_rpl_global_repair(void)
{
  if(rpl_repair_root(RPL_DEFAULT_INSTANCE)) {
    PRINTF("simple_rpl_global_repair: started global repair\n");
  } else {
    PRINTF("simple_rpl_global_repair: failed to start global repair\n");
  }
}
/*---------------------------------------------------------------------------*/
void
simple_rpl_local_repair(void)
{
  rpl_local_repair(rpl_get_instance(RPL_DEFAULT_INSTANCE));
}
/*---------------------------------------------------------------------------*/
