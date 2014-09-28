#include "Definitions.h"

#ifndef __DATAREPORTER_DLOG_1
  #define __DATAREPORTER_DLOG_1


// Variables associated with the Data Logging queue.
// Data Logging operation control structure.
struct dataLogControl
{ 
  int idx;      // Index into dataLogQueue[] array
  int type;
  result stat;
  String timeStamp;   // mm-dd-yy hh:mm:ss
  String value;      // Float or integer
};
// End of variables associated with the system log queue.




#endif
