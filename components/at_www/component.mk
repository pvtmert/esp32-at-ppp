#
# Esp32 at_www Component Makefile
#

#EXTRA_COMPONENT_DIRS := $(ESP_AT_PROJECT_PATH)/tools/mkfatfs

COMPONENT_SRCDIRS += contiki/core/ctk \
    contiki/core/sys \
    contiki/apps/webbrowser

#COMPONENT_EXTRA_INCLUDES += $(ESP_AT_PROJECT_PATH)/tools/contiki/core \
    $(ESP_AT_PROJECT_PATH)/tools/contiki/core/ctk \
    $(ESP_AT_PROJECT_PATH)/tools/contiki/core/sys

COMPONENT_PRIV_INCLUDEDIRS += webbrowser \
    contiki/core \
    contiki/core/ctk \
    contiki/core/ctk/libconio \
    contiki/apps/webbrowser

