#include "Definitions.h"

#ifndef __DATAREPORTER_GPRS_1
  #define __DATAREPORTER_GPRS_1
  // The following are used to communicate with and control
  // the shared SD card resoure.
//
// GprsControl task communication and control stuff.
//
enum gprsOperation
  {
    GPRS_POWER_OFF,
    GPRS_POWER_ON, 
    GPRS_CONNECTED, 
    GPRS_DATA_EXCHANGE,
    GPRS_OPERATION_ERROR
  };
enum gprsClientOperation
  {
    GPRS_CLIENT_WAIT,
    GPRS_CLIENT_TRANSFER,
    GPRS_CLIENT_DONE
  };
// Variables associated with the Data Logging queue.
// Data Logging operation control structure.
struct gprsControl
{ 
  gprsOperation         gprsStatus;
  gprsClientOperation   clientStatus;
  char *server;
  int port;
};

#endif

