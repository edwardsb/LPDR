
#include "Definitions.h"
#include "Rtc.h"
#include "SdReadWrite.h"
#include "SysLog.h"
#include <avr/pgmspace.h>


//  This file is used to define the text strings to be written
//  to the system log. These strings are placed in program
//  memory to conserve data memory.
//  
//  The following steps are required to place a string in the
//  program memory:
//    1.  Add the char array, containing the text of the message
//        at the end of the MESSAGE_TABLE below.
//        Note that the char array has the task name of the owner
//        to insure uniqueness. Messages start with a '\n' char.
//    2.  Add a pointer to the message as the next iten in the
//        sysLogPointers[] array. Don't forget the comma.
//    3.  Add an index to the string pointer in the 
//        sysLogPointers[] array at SYSTEM_LOG_MESSAGES 
//        in the Definitions.h file.

//
// The following creates an array of message strings, 
// pointers to the strings in program memory. An array 
// is also created for accessing the messages.
//******************************************************
// NOTE BENE: All strings must be <= SYSLOG_MSGBUF_SIZE = 30
//            and be terminated by a null '\0'
//******************************************************
// ***************MESSAGE _TABLE************************
prog_char setupString_0[] PROGMEM = "A test message.\0";
//prog_char setupString_1[] PROGMEM = "Setup task\0";
//prog_char setupString_2[] PROGMEM = "Relay Parameter err\0";
//prog_char setupString_3[] PROGMEM = "Relay operate failed\0";
//prog_char setupString_4[] PROGMEM = "Relay state error\0";
//prog_char setupString_5[] PROGMEM = "Monitor SD Write\0";
//prog_char setupString_6[] PROGMEM = "Monitor Pi on fail\0";
//prog_char setupString_7[] PROGMEM = "Monitor Pi Start Msg\0";
//prog_char setupString_8[] PROGMEM = "Monitor Datapoint\0";
//prog_char setupString_9[] PROGMEM = "Monitor Cleanup\0";
//prog_char setupString_10[] PROGMEM = "Monitor no Pi zzz\0";
//prog_char setupString_11[] PROGMEM = "Monitor Pi off fail\0";
//prog_char setupString_12[] PROGMEM = "Setup\0";
//prog_char setupString_13[] PROGMEM = "Setup\0";
//prog_char setupString_14[] PROGMEM = "Setup\0";
//prog_char setupString_15[] PROGMEM = "Setup\0";

// Cheate an array to the messages in program
// memory.
PROGMEM const char *sysLogPointers[] =
{   
  setupString_0
    //  setupString_1,
  //  setupString_2,
  //  setupString_3,
  //  setupString_4,
  //  setupString_5,
  //  setupString_6,
  //  setupString_7,
  //  setupString_8,
  //  setupString_9,
  //  setupString_10,
  //  setupString_11
  //  setupString_12
  //  setupString_13
  //  setupString_14
  //  setupString_15
};


//

// The following define the states of the SystemLogTask
// state machine.
#define SYSLOG_WAIT_RTC             1
#define SYSLOG_SD_WAIT              6

// Define a pointer to the current system log request that
// we are working.
sysLogControl *sysLogcurrentReqtPtr;
// Define an RTC control for requesting the date-time tag
// for the log message.
rtcControl sysLogRtcControl;

// Define an SD read-write control for requesting the 
// write to the SD card.
sdrwControl sysLogSdrwControl;

// Create an array in RAM to access the messages.
//char sysLogMsgBuff[DATE_TIME_LENGTH + 1 + SYSLOG_MSGBUF_SIZE + 1 + MAX_PARMS_SIZE + 1];

// The number of parameters to include in the log message.
int parmCount;
char sysLogMsgBuff[SYSLOG_MSGBUF_SIZE];
// Create a string object to assemple the log message.
String sysLogString;

void SystemLogTask(void)      
{
  switch(tasksState[SYSTEMLOG_TASK])
  {
  case TASK_INIT_STATE:
    // We are here after being scheduled by the PushSysLog()
    // function or after completing the last system log entry.
    // See if there are any more system log entries in the 
    // queue.
    sysLogcurrentReqtPtr = PopSysLog();
    //Serial.print("SystemLogTask() case: TASK_INIT_STATE  sysLogcurrentReqtPtr = ");
    //Serial.println((unsigned long)sysLogcurrentReqtPtr, HEX);

    if( sysLogcurrentReqtPtr != (sysLogControl*)QUEUE_EMPTY)
    {

      //Serial.print("SystemLogTask() (sysLogControl*)QUEUE_EMPTY = ");
      //Serial.println((unsigned long)(sysLogControl*)QUEUE_EMPTY, HEX);
      // Get an RTC string to time tag the system log entry.
      // Since the RTC is a shared resource we have to queue
      // a request to the RTC task and wait for the results.
      // Fill out the RC control structure.
      sysLogRtcControl.op = RTC_READ_DATE_TIME; // Read the RTC date-time.
      // lear the string in the control structure and put the date-time
      // string as the first field in the system log string.
      sysLogString = ""; 
      sysLogRtcControl.data.rtcSetString = &sysLogString;  // Date-time
      sysLogRtcControl.stat = NOT_STARTED;                 // Status
      // Set console state to wait for RTC task to complete.
      tasksState[SYSTEMLOG_TASK] = SYSLOG_WAIT_RTC;
      // Push the RTC request onto the RTC's FIFO queue.
      cli();    // Interrupts off.
      PushRtc(&sysLogRtcControl) ;
      sei();    // Interrupts on.
    }
    else
    {
      // There are no more system log requests in the
      // queue unschedule this state machine.
      //Serial.print("SystemLogTask() case: TASK_INIT_STATE  sysLogcurrentReqtPtr = ");
      //Serial.println((unsigned long)sysLogcurrentReqtPtr, HEX);
      tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE;
      taskScheduled[SYSTEMLOG_TASK] = false;
    }
    break;
    
  case SYSLOG_WAIT_RTC:
    //Serial.print("SystemLogTask() case: SYSLOG_WAIT_RTC sysLogRtcControl.stat = ");
    //Serial.println(sysLogRtcControl.stat, DEC);
    // Here waiting for the RTC task to complete.
    if(sysLogRtcControl.stat == SUCCESS)
    {
      //Serial.println("SystemLogTask() case: SYSLOG_WAIT_RTC SUCCESS");
      // The system log message now contains the time tag.
      // Add a space.
      sysLogString+=" ";        
      //Serial.print("SysLog sysLogString = ");
      //Serial.println(sysLogString);
      // Copy the system log message, in program memory, into 
      // temporary char array buffer.
      strcpy_P(sysLogMsgBuff, (char*)pgm_read_word(
      &(sysLogPointers[sysLogcurrentReqtPtr->msgIdx])));
      // Now Copy the error message from the temporary buffer
      // into the sysLogString string.
      for( int idx = 0; 
                idx < SYSLOG_MSGBUF_SIZE && sysLogMsgBuff[idx] != '\0'; 
                            idx++)
          sysLogString += sysLogMsgBuff[idx];        
      //Serial.print("SysLog (after adding message) sysLogString = ");
      //Serial.println(sysLogString);

      // Now add the parameters if any.
      if(sysLogcurrentReqtPtr->validParnmCount != 0)
      {
        sysLogString += '\t';
        switch(sysLogcurrentReqtPtr->type)
        {
        case SYSLOG_INTEGER:
          sysLogString += String(sysLogcurrentReqtPtr->parameter.intParm[0], DEC);
          if(sysLogcurrentReqtPtr->validParnmCount == SYSLOG_MAX_INTPARMS)
          {
            sysLogString += '\t';
            sysLogString += String(sysLogcurrentReqtPtr->parameter.intParm[1], DEC);
          }
          break;
        case SYSLOG_LONG:
          sysLogString += String(sysLogcurrentReqtPtr->parameter.longParm, DEC);
          break;
        case SYSLOG_FLOAT:
          char output[16];
          dtostrf(sysLogcurrentReqtPtr->parameter.floatParm, 10, 5, output);
          sysLogString += output;

          break;
        default:

          break;
        }   // End of  switch(sysLogcurrentReqtPtr->type)
      }    // End of if(sysLogcurrentReqtPtr->validParnmCount != 0)
      //Serial.print("SysLog (after adding parameters) sysLogString = ");
      //Serial.println(sysLogString);
      
      // Now place the sysLogString string into the SD queue to get
      // it written to the system log file.
      sysLogSdrwControl.op = SD_WRITE;     // Write to the SD card.
      // Show that the operarion has not started.
      sysLogSdrwControl.stat = NOT_STARTED;  
      // Place a pointer to the System Lo File object. This
      // File object was created in Se
      sysLogSdrwControl.filePtr = &sysLogFile;
      // Set the pointer to the string to write to the file.
      sysLogSdrwControl.data = sysLogString;
      
      //Serial.print("SysLog sysLogSdrwControl.data = ");
      //Serial.println((String)sysLogSdrwControl.data);

 // e.g.-----> SetRtcDateTime( *rtcCurrentReqtPtr->data.rtcSetString );
// e.g.    rtcTempString+=ReadTimeDate(rtcTempString);

      
      // Set console state to wait for RTC task to complete.
      tasksState[SYSTEMLOG_TASK] = SYSLOG_SD_WAIT;
      // Push the SD write request onto the SD card's
      // read-write FIFO queue.
      cli();    // Interrupts off.
      PushSdrw(&sysLogSdrwControl) ;
      sei();    // Interrupts on.
    }
    // else continue to wait in this state.
    break;

    case SYSLOG_SD_WAIT:
      // We are here to wait for the system log message to
      // be written to the system log file on the SD casrd.
    if(sysLogSdrwControl.stat == SUCCESS)
    {
      //Serial.println("SysLog success after writing SD card. ");
      // Show client success and see if there is another
      // request in the FIFO.
      sysLogcurrentReqtPtr->stat = SUCCESS;
      tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE;
    }
    // Else continue to wait in this state.
    break;

    default:
      //*******************BEGIN DIAGNOSTIC CODE**************************
      //*******************BEGIN DIAGNOSTIC CODE**************************
      // We have to do something here this is a fatal error
      // and the DataReporter will not operate. Maybe we
      // should leave a note in the EEPROM?
      Serial.println("Error opening the sysLogFile");
      //********************END DIAGNOSTIC CODE***************************
      //********************END DIAGNOSTIC CODE***************************    
    break;

  }    // End of switch(tasksState[SYSTEMLOG_TASK])
}


