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
#define WWW_CONF_MAX_COOKIES 10
#endif

#ifndef WWW_CONF_MAX_COOKIELEN
#define WWW_CONF_MAX_COOKIELEN ( WWW_CONF_MAX_URLLEN * 2 )
#endif

#ifndef WWW_CONF_MAX_HOSTLEN
#define WWW_CONF_MAX_HOSTLEN ( WWW_CONF_MAX_URLLEN / 2 )
#endif

struct host_cookies {
    char host[WWW_CONF_MAX_HOSTLEN];
    www_cookie_t *cookies[WWW_CONF_MAX_COOKIES];
};

struct host_cookies cookie_list = {0};

www_cookie_t *next_cookie(void)
{
    static www_cookie_t **cur_c = cookie_list.cookies;
    static www_cookie_t **max_c = &cookie_list.cookies[WWW_CONF_MAX_COOKIES];
    
    if (cur_c < max_c && cur_c != NULL && *cur_c!= NULL)
    {
        return *cur_c++;
    }
    cur_c = cookie_list.cookies;
    return NULL;
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
    www_cookie_t *new_cookie;
    
    int len = strlen(setcookie_value)+1;
    if (len > WWW_CONF_MAX_COOKIELEN)
    {
        PRINTF(("Cookie longer than WWW_CONF_MAX_COOKIELEN\n  %s", setcookie_value));
        return NULL;
    }
    
    if (!(new_cookie = malloc(sizeof(www_cookie_t))))
    {
        PRINTF(("Cookie memory allocation error"));
        return new_cookie;
    }
    memset(new_cookie, 0, sizeof(www_cookie_t));
    
    //PRINTF(("Cookie name: %p , NULL: %p", (void *)new_cookie->name, (void *)NULL));
    new_cookie->name =  NULL;
    
    //raw = strdup(setcookie_value);
    new_cookie->raw = malloc(len);
    strncpy(new_cookie->raw, setcookie_value, len);
    
    new_cookie->max_age = 100;
    
    for (attrib = strtok(new_cookie->raw, ";"); attrib; attrib = strtok(NULL, ";" ))
    {
        char *max_str = &attrib[strlen(attrib)];
        while ( attrib != '\0' && isspace((int)*attrib) )
            attrib++;
        
        value = strchr(attrib, '=' );
        
        if (value != NULL)
        {
            *value = '\0';
            value++; // go past the '=';
            while (value < max_str && isspace((int)*value))
                value++;
            
            PRINTF(("Cookie attribute: %s value: %s\n", attrib, value));
        } else
        {
            PRINTF(("Cookie attribute: %s \n", attrib));
        }
        
        if (strncasecmp(attrib, "domain", 6) == 0)
            new_cookie->domain = value;
        
        else if (strncasecmp(attrib, "path", 4) == 0)
            new_cookie->path = value;
        
        else if (strncasecmp(attrib, "secure", 6) == 0)
            new_cookie->secure = 1;
#ifdef WWW_COOKIE_TRACK_EXPIRES
        else if (strncasecmp(attrib, "expires", 7) == 0)
            new_cookie->expires = value;
#endif
        else if (strncasecmp(attrib, "max-age", 7) == 0)
            new_cookie->max_age = (int)strtol(value, NULL, 10);
#ifdef WWW_COOKIE_TRACK_HTTP_ONLY
        else if (strncasecmp(attrib, "httponly", 8) == 0)
            new_cookie->http_only = 1;
#endif
        else if (new_cookie->name ==  NULL && attrib && *attrib)
        {
            new_cookie->name  = attrib;
            new_cookie->value = value;
        }
        else
            PRINTF(("Cookie parse unhandled attribute: %s\n", attrib));
    }
    
    return new_cookie;
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
        // They are different, so update the host name...
        memcpy(curr_host, start_ptr, newlen);
        curr_host[newlen] = '\0';
        // and dump the old cookies.
        del_cookies();
    }
    return dif;
}

// Delete the indicated cookie, releasing string resources.
void del_cookie(www_cookie_t *c)
{
    free(c->raw);
    free(c);
    c = NULL;
}

// Parse the header string value, make a new www_cookie and add it to the cookie list.
uint8_t set_cookie(const char *setcookie_value, const char *url)
{
    /* Get the host of the url. If it doesnt match current host, dump the
     * cookies, and start over.
     */
    static int old_loc = 0;
    int i = 0, found = 0;
    PRINTF(("Cookie got set-cookie header: %s\n", setcookie_value));
    
    // Set cookie host and delete the cookie list, if different.
    set_cookie_host(cookie_list.host, url);
    
    www_cookie_t *new_cook;
    if ((new_cook = parse_cookie(setcookie_value)))
    {
        www_cookie_t *c;
        while ((c = next_cookie()))
        {
            if (c->name == new_cook->name)
            {
                PRINTF(("Cookie list replacing same name\n"));
                c = new_cook;
                found = 1;
            }
            i++;
        }
        
        if (!found)
        {
            if (i >= WWW_CONF_MAX_COOKIES)
            {
                PRINTF(("Cookie list at WWW_CONF_MAX_COOKIES, over-writing oldest\n"));
                del_cookie(cookie_list.cookies[old_loc]);
                cookie_list.cookies[old_loc] = new_cook;
                i = old_loc++;
            }
            else
            {
                cookie_list.cookies[i] = new_cook;
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
        i++;
    }
    return i;
}


// Sets the relevant cookie header value string to send.
char *get_cookie(const char *url)
{
    static char cookie_buf[WWW_CONF_MAX_URLLEN*4];
    cookie_buf[0] = '\0';
    
    //char *domain = (c->domain) ? c->domain : c->inf_domain;
    //int dlen = strlen(domain);
    // check if different host.
    set_cookie_host(cookie_list.host, url);
    
    www_cookie_t *c;
    while ((c = next_cookie()))
    {
        int field_len;
        const char *start_ptr;
        int avail;
        
        if (c->max_age == 0)
        {
            PRINTF(("Cookie skipping 0 Max-Age\n"));
            break;
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
                break;
            }
        }
#ifdef WWW_COOKIE_TRACK_HTTP_ONLY
        if (c->http_only)
        {
            field_len = get_url_field(url, UF_SCHEMA, &start_ptr);
            if ( strncmp(start_ptr, http_http, 4) != 0)
            {
                PRINTF(("Cookie Skipping Http-Only for non http url.\n"));
                break;
            }
        }
#endif
        avail = sizeof(cookie_buf) - strlen(cookie_buf);
        if ( avail > strlen(c->raw))
            strncat(cookie_buf, c->raw, avail);
        else
            PRINTF(("Cookie ran out of output space: %s\n", cookie_buf));
    }
    if (cookie_buf[0])
    {
        PRINTF(("Set-Cookie buffer: \n%s\n",cookie_buf));
        return cookie_buf;
    }
    return NULL; // http_header_set() will delete the header.
}
