/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 Eric S. Pooch
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* This creates a simple vt52 terminal emulator (tolerant of vt100)
 * for simple communication with retro terminal programs.
 */
#include <string.h>
#include <stdarg.h>
//#include <stdint.h>

#include "esp_system.h"
#include "esp_attr.h"
#include "esp_at.h"

#include "esp_log.h"

#include "driver/uart.h"

#include "contiki.h"

#include "VT100.h"

#include "libconio.h"

static const char *TAG = "ctk-vt_at";

#define NUL '\0'

#define VT_AT_SEND_CODE(x) at_write_byte(ESC_KEY); \
at_write_byte(x);

#define ESC_SEQ_TO 1000

struct vt_info vt_at_info = { };

static unsigned char curx=0, cury=0, rvrs=0, colr=0;

/*----------------------------------------------------------------------------*/

uint8_t at_read_bytes(char *dataptr, uint8_t len);
uint8_t at_read_byte(char *byteptr);
uint8_t at_write_byte(char byte);
uint8_t at_write_str(char *str);

char do_esc(char major, char minor);
char process_esc(void);


uint8_t vt_init(void)
{
    if (vt_at_info.inited == 0)
    {
        char *id_cmd_str[3] = { VT52_ID_REQ, VT100_ID_REQ, NULL };
        int i, timeout;
        char *key_buf = vt_at_info.key_buf;

        ESP_LOGE(TAG, "Identifying VT");

        //VT_AT_SEND_CODE(ID_REQ);
        for (i=0; id_cmd_str[i] != NULL; i++)
        {
            at_write_str(id_cmd_str[i]);
            for (timeout = 0;
                 *key_buf != (char)ESC_KEY && timeout < ESC_SEQ_TO;
                 timeout++)
            {
                at_read_byte(key_buf);
            }
            if (timeout >= ESC_SEQ_TO)
            {
                ESP_LOGE(TAG, "Timeout on IDENTIFY: %s", id_cmd_str[i]);
            }
            else
            {
                *key_buf = process_esc();
                // FIXME check vt_at_info.ourtype
                vt_at_info.inited = 1;
                break;
            }
        }

        ESP_LOGE(TAG, "VT Type = %c", vt_at_info.our_type);
        *key_buf = NUL;
    }
    return vt_at_info.inited;
}

/*void clrscr(void)
{
    at_write_str(VT52_CMD_STR(CURS_HOME_KEY));
    at_write_str(VT52_CMD_STR(CLR_END_SCRN));
}*/


uint8_t at_read_bytes(char *dataptr, uint8_t len)
{
    return esp_at_port_read_data((unsigned char *)dataptr, len);
}

uint8_t at_read_byte(char *byteptr)
{
    return esp_at_port_read_data((unsigned char *)byteptr, 1);
}

uint8_t at_write_byte(char byte)
{
    return esp_at_port_write_data((unsigned char *)&byte, 1);
}

uint8_t at_write_str(char *str)
{
    return esp_at_port_write_data((unsigned char *)str, strlen(str));
}
/*----------------------------------------------------------------------------*/

char process_esc(void)
{
    /* only call this if terminal is already in escape sequence! */

    int timeout = 0;
    char major = NUL;
    char *buf_loc = vt_at_info.esc_buf;
    
    memset(vt_at_info.esc_buf, NUL, MAX_VT_CODE);
    
    *buf_loc = ESC_KEY;
    buf_loc++;
    
    while (at_read_byte(buf_loc) != 1) {};
    ESP_LOGE(TAG, "processing esc:");
    printf("%c ", *buf_loc);
    //ESP_LOGE(TAG, "processing esc1: %c", *buf_loc);

    if ( strchr(ANSI_START_SEQ, *buf_loc) )
    {
        major = *buf_loc;
        // We are in a continuing esc sequence, read until we get a final byte.
        do
        {
            buf_loc++;
            if (at_read_byte(buf_loc) != 1)
                buf_loc--;
            printf(" %c", *buf_loc);
        }
        while (*buf_loc < ENDBYTE_RANGE_START || *buf_loc > ENDBYTE_RANGE_END);
        // We have a complete code or timeout.
        printf("\r\n");

    }
    // Pass along the major and minor codes, which are good enough for us.
    // If VT52, major will be '\0' (NUL).  */
    if (timeout < 1000)
        return do_esc(major, *buf_loc);
    else
        ESP_LOGE(TAG, "process_esc timeout");

    return NUL;
}

char do_esc(char major, char minor)
{
    ESP_LOGE(TAG, "doing esc: %c %c", major, minor);

    switch (major){
        case NUL:
            // If no major (most of VT52), fall through to each major VT100
            // code, checking the minor until an unconditional break.
            
        case SS3_START:
            switch (minor)
        {
            case F1_KEY:            return CTK_CONF_MENU_KEY;
            case F2_KEY:            return CTK_CONF_WINDOWSWITCH_KEY;
            case F3_KEY:            return CTK_CONF_WIDGETUP_KEY;
            case F4_KEY:            return CTK_CONF_WIDGETDOWN_KEY;
        }
            
        case CSI_START:
            switch (minor)
        {
            case CURS_UP_KEY:       return CH_CURS_UP;
            case CURS_DOWN_KEY:     return CH_CURS_DOWN;
            case CURS_RIGHT_KEY:    return CH_CURS_RIGHT;
            case CURS_LEFT_KEY:     return CH_CURS_LEFT;
                
            case KEY_END:           return CH_DEL;
                
            case VT100:   vt_at_info.our_type = VT100; break;
            case VT102:   vt_at_info.our_type = VT100; break; // close enough
        }
            break; // End of fall through for VT52 with no major.
            
        case ID_RESP_START:
            switch (minor)
        {
            case VT100_EM:  VT_AT_SEND_CODE(ANSI_CMD);  // enter ANSI mode
            default:        vt_at_info.our_type = VT52; break;// close enough
                
        }
        default: ESP_LOGE(TAG, "Unhandled esc: %c%c", major, minor);
    }
    return NUL;
}

/*----------------------------------------------------------------------------*/
unsigned char
ctk_arch_keyavail(void)
{
    if (vt_at_info.inited == 0)
        vt_init();
    
    char *key_buf = vt_at_info.key_buf;
    
    // If no char in buf yet, check if one is available.
    if (*key_buf == NUL && at_read_byte(key_buf) > 0)
    {
        // process all of the esc sequences that don't return a key press.
        while (*key_buf == (char)ESC_KEY && (*key_buf = process_esc()) == NUL)
        {
            at_read_byte(key_buf);
        }
    }
    return *key_buf;
}


/*  character or complete escape code, return the character. */

char
ctk_arch_getkey(void)
{
    char *key_buf = vt_at_info.key_buf;
    if (key_buf == NUL)
        at_read_byte(key_buf);
    
    if (*key_buf == (char)ESC_KEY)
    {
        *key_buf = process_esc();
    }
    
    key_buf[1] = *key_buf;
    *key_buf = NUL;
    
    return key_buf[1];
}


void ctk_arch_draw_char(char c,
                        unsigned char xpos,
                        unsigned char ypos,
                        unsigned char reversedflag,
                        unsigned char color)
{
    if (xpos!=curx+1 || ypos!=cury)
    {
        if (vt_at_info.our_type == VT100)
        {
            
            VT100_GXY_STR(xpos, ypos, vt_at_info.esc_buf);
            at_write_str(vt_at_info.esc_buf);

            if (reversedflag!=rvrs)
            {
                if (reversedflag)
                    at_write_str(REVERSE);
                else
                    at_write_str(OFF);
            }
            if (color!=colr)
            {
                if (color)
                    at_write_str(REVERSE);
                else
                    at_write_str(OFF);
            }
        }
        else if (vt_at_info.our_type == VT52)
        {
            at_write_str(VT52_GXY_STR(xpos, ypos));
        }
    }
    at_write_byte(c);
    
    curx=xpos; cury=ypos; rvrs=reversedflag; colr=color;
}

/*----------------------------------------------------------------------------*/

