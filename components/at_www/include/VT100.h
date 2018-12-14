//
//    FILE: VT100.h
//  ORIGINAL AUTHOR: Rob Tillaart
// VERSION: 0.2.00
// PURPOSE: VT100 codes, to be sent by serial.print() et al.
//    DATE: 2013-09-30
//     URL:
//
//

// Good source of VT52 and VT100 codes for reference.
// https://en.wikipedia.org/wiki/VT52
// https://en.wikipedia.org/wiki/ANSI_escape_code

#ifndef _VT100_h
#define _VT100_h

typedef enum
{
    DUMB,
    VT50            = 'A',
    VT55            = 'C',
    VT50H           = 'H',
    VT50H_PRINTER   = 'J',
    VT52,
    VT52_PRINTER,
    VT100_EM        = 'Z',
    VT100           = 'c',
    VT102           = 'v',
} vt_type;

typedef enum
{
    ESC_KEY         = 0x1B,
    ID_RESP_START   = '/',
    ALT_KEY_START   = '?',
    CURS_UP_KEY     = 'A',
    CURS_DOWN_KEY,
    CURS_RIGHT_KEY,
    CURS_LEFT_KEY,
    MODE_GRAPHICS   = 'F',
    CURS_HOME_KEY   = 'H',
    LINE_REV_LF,
    CLR_END_SCRN,
    CLR_END_LINE,
    LINE_INSERT,
    LINE_DELETE,
    F1_KEY,
    F2_KEY,
    F3_KEY,
    F4_KEY,
    CURS_SET_CMD    = 'Y',
    ID_REQ          = 'Z',
    ANSI_CMD        = '<',
} vt52_esc_code;

typedef enum
{
    SS2_START       = 'N',
    SS3_START,
    DSC_START,
    SOS_START       = 'X',
    CSI_START       = '[',
    ST_END,
    OSC_START,
    PM_START,
    APC_START,
    RIS_CMD         = 'c',
    SGR_END         = 'm',
    KEY_END         = '~',
    PARAMBYTE_RANGE_START   = 0x30,
    PARAMBYTE_RANGE_END     = 0x3F, /* 0–9:;<=>? */
    INTERBYTE_RANGE_START   = 0x20,
    INTERBYTE_RANGE_END     = 0x3F, /* !"#$%&'()*+,-./ */
    ENDBYTE_RANGE_START     = 0x40,
    ENDBYTE_RANGE_END       = 0x7E, /* @A–Z[\]^_`a–z{|}~ */
} vt100_esc_code;

typedef enum
{
    NOT_ESC = 0,
    IN_ESC,
    IN_O            = IN_ESC<<1,
    IN_ID           = IN_ESC<<2,
    IN_CSI          = IN_ESC<<4,
    IN_PARAM_BYTE   = IN_ESC<<5,
    IN_INTER_BYTE   = IN_ESC<<6,
    COMPLETED       = IN_ESC<<7,
} vt_esc_loc;


// ANSI COLORS
typedef enum
{
    ATTR_RESET,
    ATTR_BOLD,
    ATTR_FAINT,
    ATTR_ITALIC,
    ATTR_UNDERLINE,
    ATTR_BLINK,
    ATTR_F_BLINK,
    ATTR_REVERSE,
    ATTR_INVISIBLE,
    ATTR_CROSSOUT,
    ATTR_OFF = 20,
} ansi_textattrib;

typedef enum
{
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    FRGND = 30,
    BKGND = 40,
    BRIGHT = 60,
} ansi_color;

#define VTBUF_SIZE 3
#define MAX_VT_CODE 16

struct vt_info
{
    unsigned char   inited;
    vt_type         type;
    vt_esc_loc      esc_stat;
    unsigned char   cursx, cursy;
    ansi_color      bg_color, txt_color;
    ansi_textattrib txt_attrib;
    char            key_buf[VTBUF_SIZE];
    char            esc_buf[MAX_VT_CODE];
};

#define ANSI_START_SEQ "NOPX[/]^_"

/* convert to 1 based location with printable characters */
#define VT52_LOC(l) (l+33)

/* Temporary VT52 string with <ESC> and the command. */
#define VT52_CMD_STR(c) (char[3]) {(char)ESC_KEY, (char)c, '\0'}

/* Temporary VT52 string for moving VT55 cursor. */
#define VT52_GXY_STR(x, y) (char[5]) {\
(char)ESC_KEY, \
(char)CURS_SET_CMD, \
y + 33, x + 33, \
'\0'}

#define VT100_CMD_STR(c) VT52_CMD_STR( c )

/* Temporary string with <ESC>, [, and the command. */
#define VT100_CSI_STR(c) \
(char[4]) {(char)ESC_KEY, (char)CSI_START, (char)c, '\0'}

/* User declared string for various 1 value VT100 functions. */
#define GET_VT100_CSI1_CMD(c, value, buff) \
snprintf( buff , MAX_VT_CODE , \
"%c%c%u%c", \
(char)ESC_KEY, (char)CSI_START, \
value , \
c )

/* User declared string for various 2 value VT100 functions. */
#define GET_VT100_CSI2_CMD(c, value1, value2, buff) \
snprintf( buff , MAX_VT_CODE , \
"%c%c%u;%u%c", \
(char)ESC_KEY, (char)CSI_START, \
value1 , value2 , \
c )

/* User declared string for moving VT100 cursor. */
#define GET_VT100_GXY_CMD(x, y, buff) \
GET_VT100_CSI2_CMD( CURS_HOME_KEY , y + 1, x + 1, buff )

#define ASCII_ESC       "\033"

/* Standard default terminal size */
#define VT_SCREEN_HEIGHT   24
#define VT_SCREEN_WIDTH    80

//Identify Requests
#define VT52_ID_REQ     "\033Z"
#define VT100_ID_REQ    "\033[c"

/*
// CHAR MODES
//#define OFF             "\033[0m"
#define BOLD            "\033[1m"
#define LOWINTENS       "\033[2m"
#define UNDERLINE       "\033[4m"
#define BLINK           "\033[5m"
#define REVERSE         "\033[7m"



// POSITIONING
#define CLS             "\033[2J"       // Esc[2J Clear entire screen
#define HOME            "\033[H"

#define CUD(x)          "\033[xB"      // Move cursor up n lines

// Esc[ValueA 	Move cursor up n lines 	    CUU
// Esc[ValueB 	Move cursor down n lines 	CUD
// Esc[ValueC 	Move cursor right n lines 	CUF
// Esc[ValueD 	Move cursor left n lines 	CUB

*/


// ADDITIONAL CODES depend on terminal if supported.
/*
 Esc[20h 	Set new line mode 	LMN
 Esc[?1h 	Set cursor key to application 	DECCKM
 none 	Set ANSI (versus VT52) 	DECANM
 Esc[?3h 	Set number of columns to 132 	DECCOLM
 Esc[?4h 	Set smooth scrolling 	DECSCLM
 Esc[?5h 	Set reverse video on screen 	DECSCNM
 Esc[?6h 	Set origin to relative 	DECOM
 Esc[?7h 	Set auto-wrap mode 	DECAWM
 Esc[?8h 	Set auto-repeat mode 	DECARM
 Esc[?9h 	Set interlacing mode 	DECINLM
 
 Esc[20l 	Set line feed mode 	LMN
 Esc[?1l 	Set cursor key to cursor 	DECCKM
 Esc[?2l 	Set VT52 (versus ANSI) 	DECANM
 Esc[?3l 	Set number of columns to 80 	DECCOLM
 Esc[?4l 	Set jump scrolling 	DECSCLM
 Esc[?5l 	Set normal video on screen 	DECSCNM
 Esc[?6l 	Set origin to absolute 	DECOM
 Esc[?7l 	Reset auto-wrap mode 	DECAWM
 Esc[?8l 	Reset auto-repeat mode 	DECARM
 Esc[?9l 	Reset interlacing mode 	DECINLM
 
 Esc= 	Set alternate keypad mode 	DECKPAM
 Esc> 	Set numeric keypad mode 	DECKPNM
 
 Esc(A 	Set United Kingdom G0 character set 	setukg0
 Esc)A 	Set United Kingdom G1 character set 	setukg1
 Esc(B 	Set United States G0 character set 	setusg0
 Esc)B 	Set United States G1 character set 	setusg1
 Esc(0 	Set G0 special chars. & line set 	setspecg0
 Esc)0 	Set G1 special chars. & line set 	setspecg1
 Esc(1 	Set G0 alternate character ROM 	setaltg0
 Esc)1 	Set G1 alternate character ROM 	setaltg1
 Esc(2 	Set G0 alt char ROM and spec. graphics 	setaltspecg0
 Esc)2 	Set G1 alt char ROM and spec. graphics 	setaltspecg1
 
 EscN 	Set single shift 2 	SS2
 EscO 	Set single shift 3 	SS3
 
 Esc[m 	Turn off character attributes 	SGR0
 Esc[0m 	Turn off character attributes 	SGR0
 Esc[1m 	Turn bold mode on 	SGR1
 Esc[2m 	Turn low intensity mode on 	SGR2
 Esc[4m 	Turn underline mode on 	SGR4
 Esc[5m 	Turn blinking mode on 	SGR5
 Esc[7m 	Turn reverse video on 	SGR7
 Esc[8m 	Turn invisible text mode on 	SGR8
 
 Esc[Line;Liner 	Set top and bottom lines of a window 	DECSTBM
 
 Esc[ValueA 	Move cursor up n lines 	CUU
 Esc[ValueB 	Move cursor down n lines 	CUD
 Esc[ValueC 	Move cursor right n lines 	CUF
 Esc[ValueD 	Move cursor left n lines 	CUB
 Esc[H 	Move cursor to upper left corner 	cursorhome
 Esc[;H 	Move cursor to upper left corner 	cursorhome
 Esc[Line;ColumnH 	Move cursor to screen location v,h 	CUP
 Esc[f 	Move cursor to upper left corner 	hvhome
 Esc[;f 	Move cursor to upper left corner 	hvhome
 Esc[Line;Columnf 	Move cursor to screen location v,h 	CUP
 EscD 	Move/scroll window up one line 	IND
 EscM 	Move/scroll window down one line 	RI
 EscE 	Move to next line 	NEL
 Esc7 	Save cursor position and attributes 	DECSC
 Esc8 	Restore cursor position and attributes 	DECSC
 
 EscH 	Set a tab at the current column 	HTS
 Esc[g 	Clear a tab at the current column 	TBC
 Esc[0g 	Clear a tab at the current column 	TBC
 Esc[3g 	Clear all tabs 	TBC
 
 Esc#3 	Double-height letters, top half 	DECDHL
 Esc#4 	Double-height letters, bottom half 	DECDHL
 Esc#5 	Single width, single height letters 	DECSWL
 Esc#6 	Double width, single height letters 	DECDWL
 
 Esc[K 	Clear line from cursor right 	EL0
 Esc[0K 	Clear line from cursor right 	EL0
 Esc[1K 	Clear line from cursor left 	EL1
 Esc[2K 	Clear entire line 	EL2
 
 Esc[J 	Clear screen from cursor down 	ED0
 Esc[0J 	Clear screen from cursor down 	ED0
 Esc[1J 	Clear screen from cursor up 	ED1
 Esc[2J 	Clear entire screen 	ED2
 
 Esc5n 	Device status report 	DSR
 Esc0n 	Response: terminal is OK 	DSR
 Esc3n 	Response: terminal is not OK 	DSR
 
 Esc6n 	Get cursor position 	DSR
 EscLine;ColumnR 	Response: cursor is at v,h 	CPR
 
 Esc[c 	Identify what terminal type 	DA
 Esc[0c 	Identify what terminal type (another) 	DA
 Esc[?1;Value0c 	Response: terminal type code n 	DA
 
 Escc 	Reset terminal to initial state 	RIS
 
 Esc#8 	Screen alignment display 	DECALN
 Esc[2;1y 	Confidence power up test 	DECTST
 Esc[2;2y 	Confidence loopback test 	DECTST
 Esc[2;9y 	Repeat power up test 	DECTST
 Esc[2;10y 	Repeat loopback test 	DECTST
 
 Esc[0q 	Turn off all four leds 	DECLL0
 Esc[1q 	Turn on LED #1 	DECLL1
 Esc[2q 	Turn on LED #2 	DECLL2
 Esc[3q 	Turn on LED #3 	DECLL3
 Esc[4q 	Turn on LED #4 	DECLL4
 
 
 Codes for use in VT52 compatibility mode
 Esc< 	Enter/exit ANSI mode (VT52) 	setansi
 
 Esc= 	Enter alternate keypad mode 	altkeypad
 Esc> 	Exit alternate keypad mode 	numkeypad
 
 EscF 	Use special graphics character set 	setgr
 EscG 	Use normal US/UK character set 	resetgr
 
 EscA 	Move cursor up one line 	cursorup
 EscB 	Move cursor down one line 	cursordn
 EscC 	Move cursor right one char 	cursorrt
 EscD 	Move cursor left one char 	cursorlf
 EscH 	Move cursor to upper left corner 	cursorhome
 EscLineColumn 	Move cursor to v,h location 	cursorpos(v,h)
 EscI 	Generate a reverse line-feed 	revindex
 
 EscK 	Erase to end of current line 	cleareol
 EscJ 	Erase to end of screen 	cleareos
 
 EscZ 	Identify what the terminal is 	ident
 Esc/Z 	Correct response to ident 	identresp
 
 
 
 
 VT100 Special Key Codes
 
 These are sent from the terminal back to the computer when the particular key is pressed.
 Note that the numeric keypad keys send different codes in numeric mode than in alternate mode.
 See escape codes above to change keypad mode.
 
 
 
 Function Keys:
 EscOP	PF1
 EscOQ	PF2
 EscOR	PF3
 EscOS	PF4
 
 
 
 Arrow Keys:
 Reset 	Set
 up 	EscA	EscOA
 down 	EscB	EscOB
 right 	EscC	EscOC
 left 	EscD	EscOD
 
 
 
 Numeric Keypad Keys:
 EscOp	0
 EscOq	1
 EscOr	2
 EscOs	3
 EscOt	4
 EscOu	5
 EscOv	6
 EscOw	7
 EscOx	8
 EscOy	9
 EscOm	-(minus)
 EscOl	,(comma)
 EscOn	.(period)
 EscOM	^M
 
 
 
 Printing:
 Esc[i 	Print Screen 	Print the current screen
 Esc[1i	Print Line 	Print the current line
 Esc[4i	Stop Print Log 	Disable log
 Esc[5i	Start Print Log	Start log; all received text is echoed to a printer
 
 */

#endif
