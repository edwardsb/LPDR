
#include "Definitions.h"

//extern File sysLogFile; 
//extern char *sysLogFileName;// System Log variables.


/*
  This file is used to define the text strings to be written
  to the system log. These strings are placed in program
  memory to conserve dara memory.
  
  The following steps are required to place a string in the
  program memory:
    1.  Add the char array, containing the text of the message
        at the end of the MESSAGE_TABLE below.
        Note that the char array has the task name of the owner
        to insure uniqueness. Messages start with a '\n' char.
    2.  Add a pointer to the message as the next iten in the
        task0MsgPointers[] array. Don't forget the comma.
    3.  Add an index to the string in the task0MsgPointers[]
        array at SYSTEM_LOG_MESSAGES in LpcsDefinitions.h

*/

#include <avr/pgmspace.h>
//
// The following creates an array of message strings, 
// pointers to the strings in program memory. A RAM 
// buffer is also created for accessing the messages.
// Create some message strings in program memory to
// conserve RAM memory.
//******************************************************
// NOTE BENE: All strings must be <= SYSLOG_MSGBUF_SIZE = 30
//            and be terminated by a null '\0'
//******************************************************
// ***************MESSAGE _TABLE************************
prog_char setupString_0[] PROGMEM = "Setup task error\0";
prog_char setupString_1[] PROGMEM = "Setup task\0";
prog_char setupString_2[] PROGMEM = "Relay Parameter err\0";
prog_char setupString_3[] PROGMEM = "Relay operate failed\0";
prog_char setupString_4[] PROGMEM = "Relay state error\0";
prog_char setupString_5[] PROGMEM = "Monitor SD Write\0";
prog_char setupString_6[] PROGMEM = "Monitor Pi on fail\0";
prog_char setupString_7[] PROGMEM = "Monitor Pi Start Msg\0";
prog_char setupString_8[] PROGMEM = "Monitor Datapoint\0";
prog_char setupString_9[] PROGMEM = "Monitor Cleanup\0";
prog_char setupString_10[] PROGMEM = "Monitor no Pi zzz\0";
prog_char setupString_11[] PROGMEM = "Monitor Pi off fail\0";
//prog_char setupString_12[] PROGMEM = "Setup\0";
//prog_char setupString_13[] PROGMEM = "Setup\0";
//prog_char setupString_14[] PROGMEM = "Setup\0";
//prog_char setupString_15[] PROGMEM = "Setup\0";

// Cheate an array to the messages in program
// memory.
PROGMEM const char *sysLogPointers[] =
{   
  setupString_0,
  setupString_1,
  setupString_2,
  setupString_3,
  setupString_4,
  setupString_5,
  setupString_6,
  setupString_7,
  setupString_8,
  setupString_9,
  setupString_10,
  setupString_11
//  setupString_12
//  setupString_13
//  setupString_14
//  setupString_15
  };
 

//
  extern File sysLogFile; 
  extern char *sysLogFileName;
//  extern RTC_DS1307 rtc;
//  extern DateTime rtcNow;
//  extern String unixTimeString;
//  extern unsigned long unixTime;
  extern bool sdBusy;
  extern char sysLogMsgBuff[];

  // The following define the states of the SystemLogTask
  // state machine.
  #define SYSLOG_CAPTURE_RTC          1
  #define SYSLOG_GET_TIMESTAMP        2
  #define SYSLOG_FORM_TIMESTAMP       3
  #define SYSLOG_FORM_ENTRY           4
  #define SYSLOG_CAPTURE_SD           5
  #define SYSLOG_WRITE_SD             6
  
  int sysLogQueueIdx;
  // The number of parameters to include in the log message.
  int parmCount;
  // Create a string object to assemple the log message.
  String sysLogString;

void SystemLogTask(void)      
{
  int msgIdx;
  int copyIdx;
//  Serial.print("SystemLog Entered tasksState = ");
//  Serial.println(tasksState[SYSTEMLOG_TASK], DEC);
//  delay(3000);
  switch(tasksState[SYSTEMLOG_TASK])
  {
    case TASK_INIT_STATE:
      // We are here after being scheduled by the PushSysLog()
      // function or after completing the last system log entry.
      // See if there are any more system log entries in the 
      // queue.
      sysLogQueueIdx = PopSysLog();
      if(sysLogQueueIdx == QUEUE_EMPTY)
        // The queue is empty. We are all done. Unschedule
        // this task.
        taskScheduled[SYSTEMLOG_TASK] = false;
      else
        // We have a system log entry in the sysLogQueue 
        // array. Capture the RTC for our exclusive use.
        tasksState[SYSTEMLOG_TASK] = SYSLOG_CAPTURE_RTC;
      break;
      
    case SYSLOG_CAPTURE_RTC:
      // We are here to wait for exclusive access to the real 
      // time clock.
        cli();
        if(!rtcBusy)
        {
          rtcBusy = true;
          tasksState[SYSTEMLOG_TASK] = SYSLOG_GET_TIMESTAMP;
        }
        sei();
        // Continue to wait in this state.
        break;
      
      case SYSLOG_GET_TIMESTAMP:
        // We are here to get the timestamp from the DS1307 RTC.
//        rtcNow = rtc.now();
//        rtcBusy = false;
        tasksState[SYSTEMLOG_TASK] = SYSLOG_FORM_ENTRY;
        break;
        
      case SYSLOG_FORM_TIMESTAMP:
        // We are here to form the system log entry string.
//        unixTime = rtcNow.unixtime();
//        unixTimeString += String(unixTime, DEC);
//        unixTimeString += ",";
        // Append the err message and parameters.
        tasksState[SYSTEMLOG_TASK] = SYSLOG_FORM_ENTRY;
        break;
        
      case SYSLOG_FORM_ENTRY:        
      // We are here to form the System Log  entry.
      //  1.  Unix timestamp followed by comma.
      //  2.  Parameters followed by commas.
      
      // Place the Unix timestamp converted to ASCII digita
      // followed by comma.
//      unixTime = rtcNow.unixtime();
//      unixTimeString = String(unixTime, DEC);
//      unixTimeString+= ",";
      // Get an index into the system log message array,
      // in program memory, of pointers to the system lof
      // messages, aslso stored in program memory.
      msgIdx = sysLogQueue[sysLogQueueIdx].msgIdx;
      parmCount = sysLogQueue[sysLogQueueIdx].validParnmCount;

      // Copy the system log message, in program memory,
      // into the local char array.
      strcpy_P(sysLogMsgBuff, (char*)pgm_read_word
                                  (&(sysLogPointers[msgIdx])));
      copyIdx = 0;
      while(copyIdx < SYSLOG_MSGBUF_SIZE && sysLogMsgBuff[copyIdx] != '\0')
        unixTimeString += sysLogMsgBuff[copyIdx++];
      // Convert the parameters, if any, to ASCII digits and append to the
      //log entry.
      if(parmCount > 0)
      {
        unixTimeString += ",";
        unixTimeString += String(sysLogQueue[sysLogQueueIdx].parameter_1, DEC);        
      }
      parmCount--;
            
      if(parmCount > 0)
      {
        unixTimeString +=  ",";
        unixTimeString += String(sysLogQueue[sysLogQueueIdx].parameter_2, DEC);
      }
      parmCount--;
      
      if(parmCount > 0)
      {
        unixTimeString +=  ',';
        unixTimeString += String(sysLogQueue[sysLogQueueIdx].parameter_3, DEC);
      }
      unixTimeString+="\r\n";
      // Capture the SD card for our exclusive used.
      tasksState[SYSTEMLOG_TASK] = SYSLOG_CAPTURE_SD;
      break;
      
      case SYSLOG_CAPTURE_SD:        
      // We are here to wait for exclusive access to the SD 
      // card.
        cli();
        if(!sdBusy)
        {
          sdBusy = true;
          tasksState[SYSTEMLOG_TASK] = SYSLOG_WRITE_SD;
        }
        sei();
      // Still busy continue to wait.
        break;
      
      case SYSLOG_WRITE_SD:        
        // We are here to write the System Log entry to the
        // System log file on the SD card.
        sysLogFile = SD.open(sysLogFileName, FILE_WRITE);
        sysLogFile.println(unixTimeString);
        sdBusy = false;
        tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE;   
        break;
      
    default:
      // Here due to state index error.
      Serial.println("SystemLog: Ilg State.");
      taskScheduled[SYSTEMLOG_TASK] = false;
      break;
    }
}

