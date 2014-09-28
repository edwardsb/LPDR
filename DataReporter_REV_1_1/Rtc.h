#include "Definitions.h"

#ifndef __DATAREPORTER_RTC_1
  #define __DATAREPORTER_RTC_1
  // The following are used to communicate with and control
  // the shared RTC resoure.
  enum rtcOp
  {
    RTC_SET_DATETIME,
    RTC_READ_DATE_TIME,
    RTC_DEC_SECONDS,
    RTC_INC_SECONDS
  };

  // Variables associated with the RTC queue.
  // rtcData is used in the shared RTC resource control.
    union rtcData
    {
      // Assumes-->( 2 * DATE_TIME_ARGUMENTS) <= DATE_TIME_LENGTH
      int dateTime[DATE_TIME_ARGUMENTS];
      // Use char[] array pointers as a type confuse the IDE sometimes.
      //char dateTimeString[DATE_TIME_LENGTH + 1]; // +1 for null terminator.
      String *rtcSetString;
    };
    
  // rtcControl is used to control shared RTC resource.
    struct rtcControl
    {
       rtcOp op;
       result stat;
       int errNo;
       rtcData data;
    };
    // Date time format: mm-dd-yy hh:mi:ss
    #define DATE_TIME_LENGTH    17 
#endif

