#include "Definitions.h"

#ifndef __DATAREPORTER_SDRW_1
  #define __DATAREPORTER_SDRW_1
  // The following are used to communicate with and control
  // the shared SD card resoure.
// Enumerations.
enum sdOpeation
{
  SD_READ,
  SD_WRITE
};

struct sdrwControl
{ 
   sdOpeation op;
   result stat;
   int errNo;
   File *filePtr;
   String data;
};
#endif

