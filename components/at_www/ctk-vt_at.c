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
//#include <stdarg.h>

#include "esp_system.h"
#include "esp_attr.h"
#include "esp_at.h"

#include "esp_log.h"

#include "driver/uart.h"

#include "contiki.h"
#include "VT100.h" /* before ctk-conio.h */

#include "ctk-conio.h"
#undef ctk_arch_keyavail
#undef ctk_arch_getkey


static const char *TAG = "ctk-vt_at";

#define NUL '\0'

#define ESC_SEQ_TO 1000

struct vt_info vt_at_info = { };

/*----------------------------------------------------------------------------*/

uint8_t vt_at_init(void);
void vt_at_exit(void);

char do_esc(char major, char minor);
char process_esc(void);

/*----------------------------------------------------------------------------*/

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
uint8_t vt_at_init(void)
{
    if (vt_at_info.inited == 0)
    {
        char *id_cmd_str[3] = { VT52_ID_REQ, VT100_ID_REQ, NULL };
        int i, timeout;
        char *key_buf = vt_at_info.key_buf;
        
        ESP_LOGI(TAG, "Identifying VT");
        
        /* Send each terminal ID request string until we get an answer. */
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
                
                vt_at_info.inited = 1;
                break;
            }
        }
        
        ESP_LOGE(TAG, "VT Type = %c", vt_at_info.type);
        *key_buf = NUL;
    }

    return vt_at_info.inited;
}
/*----------------------------------------------------------------------------*/
void
vt_at_exit(void)
{
    ESP_LOGI(TAG, "vt_at_exit");
    /* restore graphics */
    GET_VT100_CSI1_CMD( SGR_END, 0, vt_at_info.esc_buf);
    at_write_str(vt_at_info.esc_buf);
    
    clrscr();
}
/*----------------------------------------------------------------------------*/
unsigned char
vt_at_resize(void)
{
    return 0;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
unsigned char
wherex(void)
{
    return vt_at_info.cursx;
}
/*----------------------------------------------------------------------------*/
unsigned char
wherey(void)
{
    return vt_at_info.cursy;
}
/*----------------------------------------------------------------------------*/
void
clrscr(void)
{
    if (vt_at_info.type == DUMB)
    {
        for ( unsigned int i = 0; i<=VT_SCREEN_HEIGHT; i++)
        {
            at_write_str("\r\n");
        }
    }
    else if (vt_at_info.type == VT100)
    {
        GET_VT100_GXY_CMD( 0, 0, vt_at_info.esc_buf);
        at_write_str(vt_at_info.esc_buf);
        at_write_str(VT100_CSI_STR(CLR_END_SCRN));
    }
    else if (vt_at_info.type <= VT100_EM)
    {
        at_write_str(VT52_CMD_STR(CURS_HOME_KEY));
        at_write_str(VT52_CMD_STR(CLR_END_SCRN));
    }
    vt_at_info.cursx = 0;
    vt_at_info.cursy = 0;
;
    
}
/*----------------------------------------------------------------------------*/
void
cputc(char c)
{
    at_write_byte(c);
    vt_at_info.cursx++;
}
/*----------------------------------------------------------------------------*/
void
cputs(char *str)
{
    at_write_str(str);
    vt_at_info.cursx += strlen(str);
}
/*----------------------------------------------------------------------------*/
void
cclear(unsigned char length)
{
    int i;
    if (vt_at_info.cursx+length >= VT_SCREEN_WIDTH)
    {
        if (vt_at_info.type == VT100)
        {
            at_write_str(VT100_CSI_STR(CLR_END_LINE));
        }
        else if (vt_at_info.type <= VT100_EM)
        {
            at_write_str(VT52_CMD_STR(CLR_END_LINE));
        }
    }
    else
    {
        for(i = 0; i < length; ++i) {
            cputc(' ');
        }
    }
    
}
/*----------------------------------------------------------------------------*/
void
chline(unsigned char length)
{
    int i;
    for(i = 0; i < length; ++i) {
        cputc('-');
    }
}
/*----------------------------------------------------------------------------*/
void
cvline(unsigned char length)
{
    int i;
    for(i = 0; i < length; ++i) {
        cputc('|');
        gotoxy(vt_at_info.cursx - 1, vt_at_info.cursy + 1);
    }
}
/*----------------------------------------------------------------------------*/
void
gotoxy(unsigned char x, unsigned char y)
{
    if (vt_at_info.cursx == x && vt_at_info.cursy == y) return;
    
    if (vt_at_info.type == DUMB)
    {
    }
    else if (vt_at_info.type == VT100)
    {
        GET_VT100_GXY_CMD( x, y, vt_at_info.esc_buf);
        at_write_str(vt_at_info.esc_buf);
    }
    else if (vt_at_info.type <= VT100_EM)
    {
        at_write_str( VT52_GXY_STR(x, y) );
    }
    vt_at_info.cursx = x;
    vt_at_info.cursy = y;

}
/*----------------------------------------------------------------------------*/
void
cclearxy(unsigned char x, unsigned char y, unsigned char length)
{
    gotoxy(x, y);
    cclear(length);
}
/*----------------------------------------------------------------------------*/
void
chlinexy(unsigned char x, unsigned char y, unsigned char length)
{
    gotoxy(x, y);
    chline(length);
}
/*----------------------------------------------------------------------------*/
void
cvlinexy(unsigned char x, unsigned char y, unsigned char length)
{
    gotoxy(x, y);
    cvline(length);
}
/*----------------------------------------------------------------------------*/
void
cputsxy(unsigned char x, unsigned char y, char *str)
{
    gotoxy(x, y);
    cputs(str);
}
/*----------------------------------------------------------------------------*/
void
cputcxy(unsigned char x, unsigned char y, char c)
{
    gotoxy(x, y);
    cputc(c);
}
/*----------------------------------------------------------------------------*/
void
revers(unsigned char c)
{
    // c is 1 or 0. convert to ANSI REVERSE attribute 7 or 27 (7+20).
    int tmp = (c) ? ATTR_REVERSE : (ATTR_REVERSE+ATTR_OFF);
    
    if (  vt_at_info.type == VT100 && tmp != vt_at_info.txt_attrib )
    {
        vt_at_info.txt_attrib = tmp;
        
        GET_VT100_CSI1_CMD( SGR_END, tmp, vt_at_info.esc_buf);
        at_write_str(vt_at_info.esc_buf);
    }
}
/*----------------------------------------------------------------------------*/
void
textcolor(unsigned char c)
{
    int tmp = c + FRGND;
    if ( vt_at_info.type == VT100 && tmp != vt_at_info.txt_color )
    {
        vt_at_info.txt_color = tmp;

#ifdef WIDGETCOLOR_HLINK
        // Underline hyperlinks.
        if (c == WIDGETCOLOR_HLINK)
            GET_VT100_CSI2_CMD( SGR_END, ATTR_UNDERLINE, tmp, vt_at_info.esc_buf);
        else
#endif
            GET_VT100_CSI2_CMD( SGR_END, ATTR_UNDERLINE+ATTR_OFF, tmp, vt_at_info.esc_buf);
        
        at_write_str(vt_at_info.esc_buf);
    }
}
/*----------------------------------------------------------------------------*/
void
bgcolor(unsigned char c)
{
    /* Presume this to be one of the first calls. */
    vt_at_init();
    
    int tmp = c + BKGND;
    if ( vt_at_info.type == VT100 )
    {
        /* reset all graphics */
        GET_VT100_CSI1_CMD( SGR_END, 0, vt_at_info.esc_buf);
        at_write_str(vt_at_info.esc_buf);
        
        /* set the background color */
        vt_at_info.bg_color = tmp;
        
        GET_VT100_CSI1_CMD( SGR_END, tmp, vt_at_info.esc_buf);
        at_write_str(vt_at_info.esc_buf);
    }
}
/*----------------------------------------------------------------------------*/
void
bordercolor(unsigned char c)
{
    /* Presume this to be one of the first calls. */
    vt_at_init();

    /* Not sure what to do here. FIXME ??*/
}
/*----------------------------------------------------------------------------*/
void
screensize(unsigned char *x, unsigned char *y)
{
    /* FIXME try set cursor way out, then inquire location. */
    *x = VT_SCREEN_WIDTH;
    *y = VT_SCREEN_HEIGHT;
}
/*----------------------------------------------------------------------------*/

char process_esc(void)
{
    /* only call this if terminal is already in escape sequence! */

    char major = NUL;
    char *buf_loc = vt_at_info.esc_buf;
    
    memset(vt_at_info.esc_buf, NUL, MAX_VT_CODE);
    
    *buf_loc = ESC_KEY;
    buf_loc++;
    
    while (at_read_byte(buf_loc) != 1) {};
    //ESP_LOGE(TAG, "processing esc:");
    //printf("%c ", *buf_loc);

    if ( strchr(ANSI_START_SEQ, *buf_loc) )
    {
        // We are in a continuing esc sequence, read until we get a final byte.
        major = *buf_loc;
        do
        {
            buf_loc++;
            if (at_read_byte(buf_loc) != 1)
                buf_loc--;
            //printf(" %c", *buf_loc);
        }
        while (*buf_loc < ENDBYTE_RANGE_START || *buf_loc > ENDBYTE_RANGE_END);
        
        // We have a code or timeout.
        *(buf_loc+1) = NUL;
        ESP_LOGE(TAG, "Received esc sequence: %s",  vt_at_info.esc_buf+1);

    }
    // Pass along the major and minor codes, which are good enough for us.
    // For most VT52, major will be '\0' (NUL).  */
    return do_esc(major, *buf_loc);
}

char do_esc(char major, char minor)
{
    ESP_LOGI(TAG, "doing esc: %c %c", major, minor);

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
                
            case VT100:   vt_at_info.type = VT100; break;
            case VT102:   vt_at_info.type = VT100; break; // close enough
        }
            break; // End of fall through for VT52 with no major.
            
        case ID_RESP_START:
            switch (minor)
        {
            case VT100_EM:  at_write_str(VT52_CMD_STR(ANSI_CMD));  // enter ANSI mode
            default:        vt_at_info.type = VT52; break;// close enough
                
        }
        default: ESP_LOGE(TAG, "Unhandled esc: %c%c", major, minor);
    }
    return NUL;
}
/*----------------------------------------------------------------------------*/
unsigned char
ctk_arch_keyavail(void)
{
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


/*  get character or complete escape code, return the character only. */
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
    else if (*key_buf == (char)3) // ctrl-c
    {
        vt_at_exit();
    }
    
    key_buf[1] = *key_buf;
    *key_buf = NUL;
    
    return key_buf[1];
}
/*----------------------------------------------------------------------------*/
/*
void ctk_arch_draw_char(char c,
                        unsigned char xpos,
                        unsigned char ypos,
                        unsigned char reversedflag,
                        unsigned char color)
{
    if (xpos != vt_at_info.cursx+1 || ypos != vt_at_info.cursy)
    {
        if (vt_at_info.type == VT100)
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
        else if (vt_at_info.type == VT52)
        {
            at_write_str(VT52_GXY_STR(xpos, ypos));
        }
    }
    at_write_byte(c);
    
    vt_at_info.cursx=xpos; vt_at_info.cursy=ypos;
    vt_at_info.txt_attrib=reversedflag; vt_at_info.txt_color=color;
}
*/
/*----------------------------------------------------------------------------*/

