CONTIKI_PROJECT = telnet-server

UIP_CONF_IPV6=1

APPS = telnetd

PROJECTDIRS = $(CONTIKI)/platform/mbxxx/apps
PROJECT_SOURCEFILES = shell-sensors.c

CFLAGS+=-DPROJECT_CONF_H=\"project-conf.h\"

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
include $(CONTIKI)/Makefile.include
