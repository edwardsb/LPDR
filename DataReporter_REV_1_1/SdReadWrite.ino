//
// Task description...
//
#include "Definitions.h"
#include <SD.h>
#include "SdReadWrite.h"
//
// Local definitions go here.
//

//
// Local globals go here.
//
  sdrwControl *sdrwCurrentReqtPtr;
//File myFile;
// Enter here only from the exec.
void SdReadWriteTask(void)      
{
  switch(tasksState[SDRW_TASK])
  {
    case TASK_INIT_STATE:
      // We are here for because there may be a request for an 
      // SD card file read write operation. All Files required
      // for the DataReporter were defined/declared in the 
      // Setup() task.
      // Check for and SD requets in the SD card's FIFO.
      sdrwCurrentReqtPtr = PopSdrw();
      //Serial.print("SdReadWriteTask() case: TASK_INIT_STATE sdrwCurrentReqtPtr = ");
      //Serial.println((unsigned long)sdrwCurrentReqtPtr, HEX);
      if( sdrwCurrentReqtPtr != (sdrwControl*)QUEUE_EMPTY)
      {
//struct sdrwControl
//{ 
//   sdOpeation op;
//   result stat;
//   int errNo;
//   File *filePtr;
//   String data;
//};
       //String *myString = (String*)sdrwCurrentReqtPtr->data;
       // We have a request for an RTC operation.
        switch(sdrwCurrentReqtPtr->op)
        {
          case SD_READ:
          
          
          break;
          case SD_WRITE:
          // We are here to write the clients message to the
          // System Log file. We keep this file closed except
          // for when we read or write to it.
          sysLogFile = SD.open(SYS_LOG_FILE ,FILE_WRITE);
          // The message to be written is in the data entry of
          // the clients SdReadWrite control structure.
          sdrwCurrentReqtPtr->filePtr->println(sdrwCurrentReqtPtr->data);
//Serial.print("SdReadWrite() Msg = ");
//Serial.println((String&)sdrwCurrentReqtPtr->data);
          //sdrwCurrentReqtPtr->filePtr->flush();
          // Leave the file closed after writing the message record.
          sdrwCurrentReqtPtr->filePtr->close();          
          // Let the client know that the system log message
          // record has been written.
          sdrwCurrentReqtPtr->stat = SUCCESS;
          // Remain scheduled to see if there are any other
          // system log messages in the FIFO queue.
          tasksState[SDRW_TASK] = TASK_INIT_STATE;
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
        }    //  End of  switch(sdrwCurrentReqtPtr->op)
      }  // End of  if( sdrwCurrentReqtPtr != (sdrwControl*)QUEUE_EMPTY)
      else
      {
      // There are no more SD operation requests in the
      // queue unschedule this state machine.
      tasksState[SDRW_TASK] = TASK_INIT_STATE;
      taskScheduled[SDRW_TASK] = false;
      }
  }    // End of  switch(tasksState[SDRW_TASK])
  
}

