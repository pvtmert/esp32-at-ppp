/*
 *
 */

#ifndef __AT_PPPD_H__
//#define __AT_PPPD_H__

uint8_t at_testCmdPpp(uint8_t *cmd_name);

uint8_t at_setupCmdPpp(uint8_t para_num);

uint8_t at_exeCmdPpp(uint8_t *cmd_name);

#define PPP_AT_COMMAND "+PPPD"
#define PPP_ATD_COMMAND "DTPPPD"
#define PPP_AT2_COMMAND "DTPPPD;"


#define PPP_AT_CUSTOM_COMMANDS	{PPP_AT_COMMAND, at_testCmdPpp, NULL, at_setupCmdPpp, at_exeCmdPpp},\
								{PPP_ATD_COMMAND, at_testCmdPpp, NULL, at_setupCmdPpp, at_exeCmdPpp},\
								{PPP_AT2_COMMAND, at_testCmdPpp, NULL, at_setupCmdPpp, at_exeCmdPpp},

#endif //__AT_PPPD_H__
