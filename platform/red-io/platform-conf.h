#ifndef PLATFORM_CONF_H
#define PLATFORM_CONF_H

#undef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  (UIP_CONF_BUFFER_SIZE - 60)

#undef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS       576

/* Make HTTP/TCP connection way faster. */
#undef UIP_CONF_TCP_SPLIT
#define UIP_CONF_TCP_SPLIT 0
#undef UIP_SPLIT_CONF_SIZE
#define UIP_SPLIT_CONF_SIZE 8

#undef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE ip64_uip_fallback_interface

#undef NETSTACK_CONF_MAC
#undef NETSTACK_CONF_RDC

#define NETSTACK_CONF_MAC     csma_driver
#define NETSTACK_CONF_RDC     nullrdc_driver

#endif /* PLATFORM_CONF_H */
