//
// The Modem Simulation task simulates the LIPO battery
// current load contributed by the Quectel M10 Cellular
// Engin (modem) and the periods during which the solar
// panel is exposed to direct sunlight
//
// This task is scheduled by the Monitor task each time
// the MPU is woken up by the watchdog timer, i.e. every
// 8 or 9 seconds and uses the RTC is used to determine th 
//
// Certain assumptions are made about the way the data
// is transmitted by the modem since the TCP and IP parts
// of the OSI stack are located in the modem.The data
// (payload) periodically sent by the DataReporter are
// set by:
#define MDMSIM_PAYLOAD_CHARS        100


#include "Definitions.h"

//
// Local definitions go here.
//
#define MDMSIM_STATE_1    1
#define MDMSIM_STATE_2    2
//
// Local globals go here.
//

// Enter here only from the exec.
void ModemSimulation(void)      
{
  switch(tasksState[MDMSIM_TASK])
  {
    case TASK_INIT_STATE:
    
    break;
    
    case MDMSIM_STATE_1:
    
    break;
    
    case MDMSIM_STATE_2:
    
    break;
    
    default:
    
    break;
  }
  
}

