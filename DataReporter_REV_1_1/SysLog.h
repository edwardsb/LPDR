#include "Definitions.h"

#ifndef __DATAREPORTER_SYSLOG_1
  #define __DATAREPORTER_SYSLOG_1
  
  #define SYSLOG_MAX_INTPARMS    2
  #define SYSLOG_1_PARAMETER     1
    enum sysLogDataType
  {
    SYSLOG_INTEGER,
    SYSLOG_LONG,
    SYSLOG_FLOAT  
  };

    union sysLogData
    {
      int intParm[SYSLOG_MAX_INTPARMS];
      long longParm;
      float floatParm;
    };

  struct sysLogControl
  { 
    int msgIdx;      // Index into LogMessages task0MsgPointers[] array
    sysLogDataType type;
    result stat;
    int validParnmCount;
    sysLogData parameter;
  };
  #define SYSLOG_MSGBUF_SIZE    30
#endif
