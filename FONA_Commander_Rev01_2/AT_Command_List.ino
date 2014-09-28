//
//
//
#include <avr/pgmspace.h>

#define AT_AT                 0
#define AT_ECHO_OFF           1
#define AT_ECHO_ON            2

// Also see ATCMD_DISPLAY_MAX.
prog_char AT_Command_0[] PROGMEM = "AT\r\n";
prog_char AT_Command_1[] PROGMEM = "ATE0\r\n";
prog_char AT_Command_2[] PROGMEM = "AT+CBC\r\n";
// The following commands 3 --> 13 enable GPRS, get a web page from 
// 'http://www.adafruit.com' and disable GPRS.
//      Enable GPRS
prog_char AT_Command_3[] PROGMEM = "AT+CGATT=1\r\n";
prog_char AT_Command_4[] PROGMEM = "AT+SAPBR=3,1,""CONTYPE"",""GPRS""\r\n";
prog_char AT_Command_5[] PROGMEM = "AT+SAPBR=3,1,""APN"",""FONAnet""\r\n";
prog_char AT_Command_6[] PROGMEM = "AT+SAPBR=1,1\r\n";
//********************************************************************************************
//********************************************************************************************
//      GET (read) a web page
//********************************************************************************************
//********************************************************************************************
prog_char AT_Command_7[] PROGMEM = "AT+HTTPTERM\r\n";
prog_char AT_Command_8[] PROGMEM = "AT+HTTPINIT\r\n";
//prog_char AT_Command_9[] PROGMEM = "AT+HTTPPARA=""CONTENT"",""application/json""\r\n";
prog_char AT_Command_9[] PROGMEM = "AT+HTTPPARA=""CID"",1\r\n";
//prog_char AT_Command_10[] PROGMEM = 
//  "AT+HTTPPARA=""URL"",""http://diggu.asuscomm.com:1337/user""\r\n";
prog_char AT_Command_10[] PROGMEM = 
  "AT+HTTPPARA=""URL"",""http://adafruit.com/testwifi/index.html""\r\n";
//prog_char AT_Command_10[] PROGMEM = "AT+HTTPDATA=nn,10000,
prog_char AT_Command_11[] PROGMEM = "AT+HTTPACTION=0\r\n";
prog_char AT_Command_12[] PROGMEM = "AT+HTTPREAD\r\n";
prog_char AT_Command_13[] PROGMEM = "AT+HTTPTERM\r\n";
//          Disable GPRS
prog_char AT_Command_14[] PROGMEM = "AT+SAPBR=0,1\r\n";
prog_char AT_Command_15[] PROGMEM = "AT+CGATT=0\r\n";
//****************************************************
//****************************************************
//      End of read a web page
//****************************************************
//****************************************************
prog_char AT_Command_16[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_17[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_18[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_19[] PROGMEM = "Reserved\r\n";
//********************************************************************************************
//********************************************************************************************
//      POST Send data to Ben's server.
//********************************************************************************************
//********************************************************************************************
prog_char AT_Command_20[] PROGMEM = "AT+HTTPTERM\r\n";// <---Will respond ERROR
prog_char AT_Command_21[] PROGMEM = "AT+HTTPINIT\r\n";
prog_char AT_Command_22[] PROGMEM = "AT+HTTPPARA=""CONTENT"",""application/json""\r\n";
prog_char AT_Command_23[] PROGMEM = "AT+HTTPPARA=""URL"",""http://diggu.asuscomm.com:1337/user""\r\n";
prog_char AT_Command_24[] PROGMEM = "AT+HTTPDATA=18,10000\r\n";
prog_char AT_Command_25[] PROGMEM = "{\"name\":\"arduino\"}";
prog_char AT_Command_26[] PROGMEM = "AT+HTTPACTION=1\r\n";
prog_char AT_Command_27[] PROGMEM = "AT+HTTPTERM\r\n";
prog_char AT_Command_28[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_29[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_30[] PROGMEM = "AT+CMGF=?\r\n";
prog_char AT_Command_31[] PROGMEM = "AT+CMGF?\r\n";
prog_char AT_Command_32[] PROGMEM = "AT+CPMS=?\r\n";
prog_char AT_Command_33[] PROGMEM = "AT+CPMS?\r\n";
prog_char AT_Command_34[] PROGMEM = "AT+CMGF=1\r\n";
prog_char AT_Command_35[] PROGMEM = "AT+CSDH=1\r\n";
prog_char AT_Command_36[] PROGMEM = "AT+CMGR=1\r\n";
prog_char AT_Command_37[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_38[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_39[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_40[] PROGMEM = "AT+CBC\r\n";
prog_char AT_Command_41[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_42[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_43[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_44[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_45[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_46[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_47[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_48[] PROGMEM = "Reserved\r\n";
prog_char AT_Command_49[] PROGMEM = "Reserved\r\n";
//****************************************************
//****************************************************
//      End of Send data to Ben's server.
//****************************************************
//****************************************************


PROGMEM const char *atCmdPointers [] =
{
  AT_Command_0,
  AT_Command_1,
  AT_Command_2,
  AT_Command_3,
  AT_Command_4,
  AT_Command_5,
  AT_Command_6,
  AT_Command_7,
  AT_Command_8,
  AT_Command_9,
  AT_Command_10,
  AT_Command_11,
  AT_Command_12,
  AT_Command_13,
  AT_Command_14,
  AT_Command_15,
  AT_Command_16,
  AT_Command_17,
  AT_Command_18,
  AT_Command_19,
  AT_Command_20,
  AT_Command_21,
  AT_Command_22,
  AT_Command_23,
  AT_Command_24,
  AT_Command_25,
  AT_Command_26,
  AT_Command_27,
  AT_Command_28,
  AT_Command_29,
  AT_Command_30,
  AT_Command_31,
  AT_Command_32,
  AT_Command_33,
  AT_Command_34,
  AT_Command_35,
  AT_Command_36,
  AT_Command_37,
  AT_Command_38,
  AT_Command_39,
  AT_Command_40,
  AT_Command_41,
  AT_Command_42,
  AT_Command_43,
  AT_Command_44,
  AT_Command_45,
  AT_Command_46,
  AT_Command_47,
  AT_Command_48,
  AT_Command_49
};
