/*
 * Contiki BSD-stye License
 *
 * Copyright (c) 2018 Eric S. Pooch.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of esp32_at_ppp module.
 *
 *
 */

#include <string.h>
#include <stdio.h>

#if 0
#define PRINTF(x)
#else
#include <stdio.h>
#define PRINTF(x) printf x
#endif

#include "http_parser.h"
#include "http-strings.h"

#include "www_cookies.h"

typedef enum http_parser_url_fields www_url_field;

#ifndef WWW_CONF_MAX_COOKIES
#define WWW_CONF_MAX_COOKIES 12
#endif

#ifndef WWW_CONF_MAX_COOKIELEN
#define WWW_CONF_MAX_COOKIELEN ( WWW_CONF_MAX_URLLEN * 2 )
#endif

#ifndef WWW_CONF_MAX_HOSTLEN
#define WWW_CONF_MAX_HOSTLEN 254
#endif

struct host_cookies {
    char host[WWW_CONF_MAX_HOSTLEN];
    www_cookie_t *cookies[WWW_CONF_MAX_COOKIES];
};

struct host_cookies cookie_list = {0};
static int oldest_cookie_idx = 0;


// Call this function until it returns NULL, so that next caller has a fresh list.
www_cookie_t **next_list_cookie(void)
{
    static www_cookie_t **cur_c = cookie_list.cookies;
    static www_cookie_t **max_c = &cookie_list.cookies[WWW_CONF_MAX_COOKIES];
        
    if (cur_c < max_c && *cur_c != NULL)
    {
        return cur_c++;
    }
    cur_c = cookie_list.cookies;
    return NULL;
}

www_cookie_t *next_cookie(void)
{
    www_cookie_t **c = next_list_cookie();
    if (c)
        return *c;
    return NULL;
}

//Return a newly allocated cookie.
www_cookie_t *new_cookie(unsigned int raw_len)
{
    www_cookie_t *c;
    
    if (!(c = malloc(sizeof(www_cookie_t))))
    {
        PRINTF(("Cookie memory allocation error"));
        return c;
    }
    memset(c, 0, sizeof(www_cookie_t));
    
    c->name =  NULL;
    c->raw = malloc(raw_len);
    
    return c;
}


uint8_t get_url_field(const char *url, www_url_field field, const char **start_ptr)
{
    int len = 0;
    struct http_parser_url purl;
    *start_ptr = NULL;
    
    http_parser_url_init(&purl);
    len = http_parser_parse_url(url, strlen(url), 0, &purl);
    
    if (len != 0) return 0;
    
    len = purl.field_data[field].len;
    
    *start_ptr = (url + purl.field_data[field].off);
    
    return len;
}

int8_t url_field_compare(const char *url, www_url_field field, const char *compare)
{
    // Parse a url.
    int8_t dif = 0;
    const char *start_ptr;
    
    int newlen = get_url_field(url, field, &start_ptr);
    int curlen = strlen(compare);
    
    if (newlen == curlen)
        dif = (int8_t)memcmp(start_ptr, compare, curlen);
    else
        dif = (newlen > curlen) ? 1 : -1;
    
    return dif;
}

// Called by set_cookie to parse header value into a new www_cookie.
www_cookie_t *parse_cookie(const char *setcookie_value)
{
    char *attrib, *value = NULL;
    www_cookie_t *c;
    
    int len = strlen(setcookie_value)+1;
    if (len > WWW_CONF_MAX_COOKIELEN)
    {
        PRINTF(("Cookie longer than WWW_CONF_MAX_COOKIELEN\n  %s", setcookie_value));
        return NULL;
    }
    
    c = new_cookie(len);
    strncpy(c->raw, setcookie_value, len);
    
    c->max_age = 100;
    
    for (attrib = strtok(c->raw, ";"); attrib; attrib = strtok(NULL, ";" ))
    {
        while ( attrib != '\0' && isspace((int)*attrib) )
            attrib++;
        
        if ((value = strchr(attrib, '=' )))
        {
            *value = '\0';  // split the string.
            value++;        // go the start of the value;
            
            /*PRINTF(("Cookie parsed: %s  =  %s\n", attrib, value));
        }
        else
        {
            PRINTF(("Cookie parsed: %s \n", attrib));*/
        }
        
        if (strncasecmp(attrib, "domain", 6) == 0)
            c->domain = value;
        
        else if (strncasecmp(attrib, "path", 4) == 0)
            c->path = value;
        
        else if (strncasecmp(attrib, "secure", 6) == 0)
            c->secure = 1;
#ifdef WWW_COOKIE_TRACK_EXPIRES
        else if (strncasecmp(attrib, "expires", 7) == 0)
            c->expires = value;
#endif
        else if (strncasecmp(attrib, "max-age", 7) == 0)
        {
            if (value)
                c->max_age = (int)strtol(value, NULL, 10);
        }
#ifdef WWW_COOKIE_TRACK_HTTP_ONLY
        else if (strncasecmp(attrib, "httponly", 8) == 0)
            c->http_only = 1;
#endif
        else if (c->name ==  NULL && attrib && *attrib)
        {
            c->name  = attrib;
            c->value = value;
        }
        else
            PRINTF(("Cookie parsed unhandled attribute: %s\n", attrib));
    }
    
    return c;
}

// Conditionally set the cookie host name and return a memcmp() type difference.
int8_t set_cookie_host(char *curr_host, const char *url)
{
    // Parse a url.
    int8_t dif = 0;
    const char *start_ptr;
    
    int newlen = get_url_field(url, UF_HOST, &start_ptr);
    int curlen = strlen(curr_host);
    
    if (newlen >= WWW_CONF_MAX_HOSTLEN)
        newlen = WWW_CONF_MAX_HOSTLEN-1;
    
    if (newlen == curlen)
        dif = (int8_t)memcmp(start_ptr, curr_host, curlen);
    else
        dif = (newlen > curlen) ? 1 : -1;
    
    if (dif)
    {
        // Hosts are different, so update the host name...
        memcpy(curr_host, start_ptr, newlen);
        curr_host[newlen] = '\0';
        PRINTF(("Cookie setting new host: %s\n", curr_host));
        // and dump the cookies from the old host.
        del_cookies();
        oldest_cookie_idx = 0;
        //PRINTF(("Cookies empty?: %s\n", get_cookie(url)));
    }
    return dif;
}

// Delete the indicated cookie, releasing string resources.
void del_cookie(www_cookie_t *c)
{
    PRINTF(("del_cookie\n"));

    if (c)
        free(c->raw);
    else
        PRINTF(("Cookie attempt to delete NULL cookie!!\n"));

    free(c);
}

// Parse the header string value, make a new www_cookie and add it to the cookie list.
uint8_t set_cookie(const char *setcookie_value, const char *url)
{
    /* Get the host of the url. If it doesnt match current host, dump the
     * cookies, and start over.
     */
    int i = 0, found = 0;
    PRINTF(("Cookie got set-cookie header: %s\n", setcookie_value));
    
    // Set cookie host and delete the cookie list, if different.
    set_cookie_host(cookie_list.host, url);
    
    www_cookie_t *n_cookie;
    if ((n_cookie = parse_cookie(setcookie_value)))
    {
        www_cookie_t *c;
        while ((c = next_cookie()))
        {
            if (strncmp(c->name, n_cookie->name, WWW_CONF_MAX_COOKIELEN) == 0)
            {
                PRINTF(("Cookies replacing same name\n"));
                del_cookie(cookie_list.cookies[i]);
                cookie_list.cookies[i] = n_cookie;
                found = 1;
            }
            i++;
        }
        
        if (!found)
        {
            if (i >= WWW_CONF_MAX_COOKIES)
            {
                PRINTF(("Cookies at WWW_CONF_MAX_COOKIES, over-writing oldest: %d\n", oldest_cookie_idx));
                del_cookie(cookie_list.cookies[oldest_cookie_idx]);
                cookie_list.cookies[oldest_cookie_idx] = n_cookie;
                if (++oldest_cookie_idx >= WWW_CONF_MAX_COOKIES)
                    oldest_cookie_idx = 0;
                i = oldest_cookie_idx;
            }
            else
            {
                cookie_list.cookies[i] = n_cookie;
            }
        }
        return i;
    }
    return 0;
}

// Delete all cookies from the cookie list, releasing string resources.
uint8_t del_cookies(void)
{
    www_cookie_t *c;
    int i = 0;
    
    while ((c = next_cookie()))
    {
        del_cookie(c);
        c = NULL;
        cookie_list.cookies[i] = NULL;
        i++;
    }
    return i;
}


// Sets the relevant cookie header value string to send.
char *get_cookie(const char *url)
{
    PRINTF(("Cookie request\n"));
    static char cookie_buf[WWW_CONF_MAX_COOKIELEN*4];

    cookie_buf[0] = '\0';
    cookie_buf[(WWW_CONF_MAX_COOKIELEN*4)-1] = '\0';
    
    // check if different host.
    set_cookie_host(cookie_list.host, url);
    
    www_cookie_t *c;
    while ((c = next_cookie()) && c)
    {
        PRINTF(("get_cookie\n"));

        int field_len;
        const char *start_ptr;
        
        if (c->max_age == 0)
        {
            PRINTF(("Cookie skipping %d Max-Age\n", c->max_age));
            continue;
        }
        if (c->domain)
        {
            //field_len = get_url_field(url, UF_HOST, start_ptr);
        }
        if (c->path)
        {
            //field_len = get_url_field(url, UF_PATH, start_ptr);
        }
        if (c->secure)
        {
            field_len = get_url_field(url, UF_SCHEMA, &start_ptr);
            if (strncmp(start_ptr, http_https, 5) != 0)
            {
                PRINTF(("Cookie skipping secure for insecure url: %s\n", c->name));
                continue;
            }
        }
#ifdef WWW_COOKIE_TRACK_HTTP_ONLY
        if (c->http_only)
        {
            field_len = get_url_field(url, UF_SCHEMA, &start_ptr);
            if ( strncmp(start_ptr, http_http, 4) != 0)
            {
                PRINTF(("Cookie Skipping Http-Only for non http url.\n"));
                continue;
            }
        }
#endif
        char *loc = cookie_buf + strlen(cookie_buf);

        if (loc != cookie_buf)
        {
            // There is already something in cookie buff, so add cookie delimiter.
            loc = stpncpy(loc, "; ", sizeof(cookie_buf)-(loc-cookie_buf)-1);
        }
        loc = stpncpy(loc, c->name, sizeof(cookie_buf)-(loc-cookie_buf)-1);
        
        if (c->value)
        {
            *loc = '=';
            loc++;
            *loc = '\0';
            loc = stpncpy(loc, c->value, sizeof(cookie_buf)-(loc-cookie_buf)-1);
        }
    }
    
    if (*cookie_buf)
    {
        PRINTF(("Set-Cookie buffer: \n%s\n", cookie_buf));
        return cookie_buf;
    }
    return NULL; // http_header_set() will delete the header.
}
