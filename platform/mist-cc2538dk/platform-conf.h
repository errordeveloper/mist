#ifndef PLATFORM_CONF_H
#define PLATFORM_CONF_H

#include "mist-conf-const.h"

#ifndef MIST_CONF_NETSTACK
//#define MIST_CONF_NETSTACK      MIST_CONF_NULLRDC
//#define MIST_CONF_NETSTACK      MIST_CONF_DROWSIE
#define MIST_CONF_NETSTACK      (MIST_CONF_AES | MIST_CONF_DROWSIE)
#endif /* MIST_CONF_NETSTACK */

#include "mist-default-conf.h"

#define NETSTACK_RADIO_MAX_PAYLOAD_LEN 125

#define NETSTACK_AES_KEY "thingsquare mist"

#endif /* PLATFORM_CONF_H */
