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
/*  
 try:
 
AT+WEB="http://retro.hackaday.com"
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_attr.h"

#include "esp_log.h"

#ifdef CONFIG_AT_BROWSE_SUPPORT

#include "driver/uart.h"



#include "htmlparser.h"
#include "http-strings.h"

#include "at_browse.h"


#include "esp_http_client.h"

char wait_for_input(char a);

void more(unsigned int read);

#define BROWSE_AT_PARAMS "\"<URI>\""

#define BUFF_SIZE 256

static char buff[BUFF_SIZE];

#define _ARGS1(...) __VA_ARGS__
#define STRIP_PARENS(x) x
#define PRINTF(x) snprintf(buff, BUFF_SIZE, STRIP_PARENS( _ARGS1 x ))


static const char *TAG = "webbrowser";

static esp_at_cmd_struct at_web_cmd[] = {
    {"+WEB", at_testCmdBrowse, NULL, at_setupCmdBrowse, at_exeCmdBrowse},
};

static esp_http_client_config_t browse_config;




esp_err_t browse_event_handle(esp_http_client_event_t *evt);

bool esp_at_web_cmd_regist(void)
{
    return esp_at_custom_cmd_array_regist (at_web_cmd, 1);
}

uint8_t at_testCmdBrowse(uint8_t *cmd_name)
{
	char response[((int)strlen((char *)cmd_name) + (int)strlen(BROWSE_AT_PARAMS))+2];
	
	sprintf(response, "%s:%s", cmd_name, BROWSE_AT_PARAMS);
	uart_write_bytes(CONFIG_AT_UART_PORT, response, strlen(response));
	
	return ESP_AT_RESULT_CODE_OK;
}

uint8_t at_setupCmdBrowse(uint8_t para_num)
{
	int32_t para_idx = 0;
	
	char *para_str = NULL;

	if (para_num != 1 ) return ESP_AT_RESULT_CODE_ERROR;
	
	if (esp_at_get_para_as_str(para_idx++, (uint8_t**)&para_str) == ESP_AT_PARA_PARSE_RESULT_FAIL)
	{
		ESP_LOGE(TAG, "Parse error on parameter: %d.", para_idx-1);
		return ESP_AT_RESULT_CODE_ERROR;
	}
	else
	{
        ESP_LOGE(TAG, "Raw URL: %s", para_str);

        htmlparser_init();

        browse_config.url = para_str;
        browse_config.event_handler = browse_event_handle;
        
        esp_http_client_handle_t browse_client = esp_http_client_init(&browse_config);
        esp_err_t err = esp_http_client_perform(browse_client);
        
        if (err == ESP_OK)
        {
            int len = esp_http_client_get_content_length(browse_client);
            int stat = esp_http_client_get_status_code(browse_client);

            ESP_LOGE(TAG, "\nStatus = %d, content_length = %d\n", stat, len);
            
            return ESP_AT_RESULT_CODE_OK;
        }
        esp_http_client_cleanup(browse_client);

    }
    return ESP_AT_RESULT_CODE_ERROR;
}

esp_err_t browse_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
        case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
        case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
        case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
        printf("%.*s", evt->data_len, (char*)evt->data);
        break;
        case HTTP_EVENT_ON_DATA:
        ESP_LOGE(TAG, "HTTP_EVENT_ON_DAT, len=%d", evt->data_len);

        if (!esp_http_client_is_chunked_response(evt->client))
        {
            //printf("%.*s", evt->data_len, (char*)evt->data);
            htmlparser_parse((char*)evt->data, evt->data_len);
        }
            more(evt->data_len);

        break;
        case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
        case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}


void more(unsigned int read)
{
    static int count = 0;
    const char more[] = "\e[1m| Press Space Key for Next Page |\e[m\r";
    count+=read;
    
    if (count%2)
    {
        htmlparser_newline();
        uart_write_bytes(CONFIG_AT_UART_PORT, more, strlen(more));
        wait_for_input(' ');
    }
    count++;
}


char wait_for_input(char a)
{
    uint8_t data;
    int len;
    while  ((char)data != a)
    {
        len = uart_read_bytes(CONFIG_AT_UART_PORT, &data, 1, portMAX_DELAY);
    }
    return (char)data;
}


uint8_t at_exeCmdBrowse(uint8_t *cmd_name)
{
    return ESP_AT_RESULT_CODE_OK;
}


void htmlparser_link(char *text, unsigned char textlen, char *url)
{
    PRINTF(("[%.*s](%s) ", textlen, text, url));
    uart_write_bytes(CONFIG_AT_UART_PORT, buff, strlen(buff));
}

void htmlparser_form(char *action)
{
}

void htmlparser_submitbutton(char *value,
                             char *name)
{
}

void htmlparser_inputfield(unsigned char type, unsigned char size,
                           char *value, char *name)
{
    PRINTF(("[   %.*s    ]", size, value));
    uart_write_bytes(CONFIG_AT_UART_PORT, buff, strlen(buff));
    
}

void htmlparser_newline(void)
{
    PRINTF(("\r\n"));
    uart_write_bytes(CONFIG_AT_UART_PORT, buff, strlen(buff));
    
}

void htmlparser_word(char *word, unsigned char wordlen)
{
    PRINTF(("%.*s ", wordlen, word));
    uart_write_bytes(CONFIG_AT_UART_PORT, buff, strlen(buff));
}

void htmlparser_tag(char *tag)
{
    //PRINTF(("%s", tag));
}

#else
#warning CONFIG_AT_BROWSE_SUPPORT is disabled!
#endif