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

/* This is header for a simple cookie implementation.
 * For security reasons, if the host in the url changes at all,
 * the cookies will be dumped and we start over.
 */

#ifndef WWW_COOKIES_H_
#define WWW_COOKIES_H_

#include <stdint.h>
#include "contiki-conf.h"

typedef struct www_cookie {
    char *raw;
    char *name;
    char *value;
    char *domain;
    char *path;
    uint8_t secure;
#ifdef WWW_COOKIE_TRACK_EXPIRES
    time_t expires;
#endif
    int max_age;
#ifdef WWW_COOKIE_TRACK_HTTP_ONLY
    utf8_t http_only;
#endif
} www_cookie_t;


// Parse the header string value, make a new www_cookie and add it to the cookie list.
uint8_t set_cookie(const char *setcookie_value, const char *url);

// Delete all cookies from the cookie list, releasing string resources.
uint8_t del_cookies(void);

// Returns a the relevant cookie header value string.
char *get_cookie(const char *url);

#endif /* WWW_COOKIES_H_ */
