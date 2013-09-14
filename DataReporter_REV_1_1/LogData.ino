//
// This task writes a datapoint to the DATA_LOGGING_FILE and
// DATA_HISTORY_FILE files.
//
// A datapoint consists of a structure that identifies the
// the datapoint type, the data, and a status that controls
// the writing of the datapoint.
//
// Data points that are to be logged are placed into the 
// dataLogQueue[] where they are removed by this task and
// written to the DATA_LOGGING_FILE and DATA_HISTORY_FILE
// files as comma delimited strings in the following format
//         datapoint_type, time_stamp, value
// Where datapoint_type is an integer.
//       time_stamp is ASCII mm-dd-yy hh:mm:ss (24 hour local time)
//       value is ASCII integer or floating point.
//
// This task is scheduled by the FIFO queue routine. 
//
// The task deschedules itself when the queue is empty.

#include "Definitions.h"

// The following define the states in task state machine.
#define DATALOG_CAPTURE_RTC        1
#define DATALOG_GET_TIMESTAMP      2
#define DATALOG_FORM_ENTRY         3

int logDataQueueIdx;
String logDataNow; 
void LogDatapoint(void)      
{
  switch(tasksState[SYSTEMLOG_TASK])
  {
       case TASK_INIT_STATE:
      // We are here after being scheduled by the PushDataLog()
      // function or after logginf the last datapoint. See if 
      // there are any moredatapoints to be logged in the 
      // queue.
      logDataQueueIdx = PopDataLog();
      if(logDataQueueIdx == QUEUE_EMPTY)
        // The queue is empty. We are all done. Unschedule
        // this task.
        taskScheduled[DATA_LOGGER_TASK] = false;
      else
        // We have a system log entry in the sysLogQueue 
        // array. Capture the RTC for our exclusive use.
        tasksState[DATA_LOGGER_TASK] = DATALOG_CAPTURE_RTC;
      break;
      
    case DATALOG_CAPTURE_RTC:
      // We are here to wait for exclusive access to the real 
      // time clock.
        cli();
        if(!rtcBusy)
        {
          rtcBusy = true;
          tasksState[DATA_LOGGER_TASK] = DATALOG_GET_TIMESTAMP;
        }
        sei();
        // Continue to wait in this state.
        break;
      
      case DATALOG_GET_TIMESTAMP:
        // We are here to get the timestamp from the DS2434 RTC.
        //logDataNow = ReadTimeDate();
        rtcBusy = false;
        tasksState[DATA_LOGGER_TASK] = DATALOG_FORM_ENTRY;
        break;

// 
      
      
      
      
      
      
      
      
  }  // End of switch(tasksState[SYSTEMLOG_TASK]).
}
