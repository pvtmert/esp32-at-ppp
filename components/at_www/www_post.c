/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 Eric S. Pooch
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

/* This is the implementaion of a simple http posting mechanism with encoding,
 * "application/x-www-form-urlencoded".
 */


#include <string.h>
#include <stdio.h>

#if 0
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif

#include "contiki-conf.h"
#include "www_post.h"

#define ISO_space 0x20
#define ISO_ampersand 0x26
#define ISO_plus  0x2b
#define ISO_eq    0x3d

static char post_data[WWW_CONF_MAX_URLLEN*2];

void add_post_query(char *key, char *value)
{
    static char *query;
    
    if (post_data[0])
    {
        *query++ = ISO_ampersand;
    } else {
        query = post_data;
    }
    
    query = urlencode(query, key);
    *query++ = ISO_eq;
    query = urlencode(query, value);
}


void clear_post_query(void)
{
    post_data[0] = '\0';
}


char *post_query(void)
{
    return post_data;
}


char *urlencodechr(char *dst, int c)
{
    if (isalnum(c) || strchr("-._~", c))
    {
        *dst = c;
        return dst+1;
    }
    else if (c == ISO_space)
    {
        *dst = ISO_plus;
        return dst+1;
    }
    snprintf(dst, 4, "%%%02X", c);
    return dst+3;
}

char *urlencode(char *dst, const char *src)
{
    char *d;
    const char *s;
    
    for (d=dst, s=src; *s != '\0'; s++)
    {
        d = urlencodechr(d, (int)*s);
    }
    *d = *s; // '\0'
    
    return d;
}
