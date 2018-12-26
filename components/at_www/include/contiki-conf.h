/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * Author: Oliver Schmidt <ol.sc@web.de>
 *
 */


#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#define CCIF
#define CLIF

#include "esp_system.h"
//#include "esp_attr.h"

//#include <stdio.h>
//#include <stdint.h>
#include <ctype.h>
//#include "ctk/ctk.h"

#include "VT100.h"
#include "libconio.h"

#define ctk_arch_isprint(c)  isprint((unsigned char)c)

#define CTK_CONF_MOUSE_SUPPORT  0
#define CTK_CONF_MENUS          0

#define CH_DEL          0x7F
#define CH_ENTER        0x0D
#define CH_ESC          0x1B
#define CH_CURS_UP      0x0B
#define CH_CURS_DOWN    0x0A
#define CH_CURS_LEFT    0x08
#define CH_CURS_RIGHT   0x15

#define CTK_CONF_MENU_KEY         0x05  /* Ctrl-E */
#define CTK_CONF_WINDOWSWITCH_KEY 0x17  /* Ctrl-W */
#define CTK_CONF_WIDGETUP_KEY     0x01  /* Ctrl-A */
#define CTK_CONF_WIDGETDOWN_KEY   '\t'  /* Tab or Ctrl-I */

#define MOUSE_CONF_XTOC(x) ((x) / 4)
#define MOUSE_CONF_YTOC(y) ((y) / 8)

#define BORDERCOLOR       COLOR_BLACK
#define SCREENCOLOR       COLOR_WHITE       // background color of most of screen
//#define BACKGROUNDCOLOR      // Not used?
#define WINDOWCOLOR       COLOR_BLACK       // text in unfocused window
#define WINDOWCOLOR_FOCUS COLOR_BLACK       // text in focused window
#define WIDGETCOLOR       COLOR_MAGENTA     // text of unfocused widget
#define WIDGETCOLOR_FOCUS COLOR_BLUE        // text of focused widget (reversed)
#define WIDGETCOLOR_FWIN  COLOR_BLACK       // text in focused window
#define WIDGETCOLOR_HLINK COLOR_CYAN        // text in url (+UNDERLINE)

#define EMAIL_CONF_WIDTH  ( VT_SCREEN_WIDTH - 1 )
#define EMAIL_CONF_HEIGHT ( VT_SCREEN_HEIGHT - 4)
#define EMAIL_CONF_ERASE   0

#define FTP_CONF_WIDTH  (( VT_SCREEN_WIDTH / 2)- 2)
#define FTP_CONF_HEIGHT ( VT_SCREEN_HEIGHT - 2)

#define IRC_CONF_WIDTH  VT_SCREEN_WIDTH
#define IRC_CONF_HEIGHT VT_SCREEN_HEIGHT

//#define WWW_CONF_HOMEPAGE
#define WWW_CONF_WEBPAGE_WIDTH      VT_SCREEN_WIDTH
#define WWW_CONF_WEBPAGE_HEIGHT     ( VT_SCREEN_HEIGHT - 5)
#define WWW_CONF_HISTORY_SIZE       6
#define WWW_CONF_MAX_URLLEN         255
#define WWW_CONF_MAX_NUMPAGEWIDGETS ( VT_SCREEN_WIDTH / 2)
#define WWW_CONF_FORMS              1
#define WWW_CONF_COOKIES            1
#define WWW_CONF_POST_QUERY         1
#define WWW_CONF_USER_AGENT "Contiki/3.x (LOLIN32; http://www.contiki-os.org/)"
#endif /* __CONTIKI_CONF_H__ */
