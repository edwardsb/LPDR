/*
  The Monitor task:
  //  1.  Is scheduled by the Setup task.
  //  2.  Once scheduled it remains scheduled.
  //  3.  Performs the power managment task.
  //  4.  Monitors the rain bucket interrupt.
  //  5.  Turns power on/off the Pi as required.
  //  6.  Transfers the rainfall data to Pi.
*/

#include "Definitions.h"
#include "SysLog.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <SPI.h>

extern boolean consoleInput;



  // Local error definitions.
  #define SD_WRT_ERR_MAX            3
  
  // External references used by this task.
//                      // It is the responsibility of the SerialEvent() client to
//                      // set the following 2 variables false prior to waiting for
//                      // a string from the Pi.
//                      extern bool piStartupReceived;
//                      extern  bool piShutdownReceived;
//                      extern  bool piNakReceived;
//                      extern  bool piAckReceived;
  // The following variables are used for controlling and 
  // communicating with the SdReadWrite().
//                      extern int sdRecords;  
//                      extern bucketControl bucketRdWrtTaskCtl;
//                      extern boolean queueBucketTips;
//                      extern int PushRelay(struct relayControl *controlPtr);

  // End of external references used by this task.
  
  // Variables private to this task.
  String piDatapoint;
  boolean bucketTipActive;
  boolean monitorBucketTipsStop = false;
  boolean  watchDogFired = false;
  int relayCtlIdx;
  // End of variables private to this task.
   
// Enter here only from the exec.
void MonitorTask(void)      
{
  static int noOfDatapointChars;
  static int datapointIdx;
  static int piCharRetryCounter;
  static int sdEDrrMsgCounter = 0;
  static int ledBlinker = 0;
  static int monitorSecsSinceReport = 0;
  static sysLogControl monitorLog;
  static int sdWrtErrCount = SD_WRT_ERR_MAX;
  static boolean reportIsDone = false;

  switch(tasksState[MONITOR_TASK])
  {
    case TASK_INIT_STATE:
      // Enter here one time when the monitoring is first
      // started.     
      // Define and attach the Rain Bucket tip indicator
      // signal to the BucketTip() handler.
      // Comments for the following watchdog timer setup are at
      // the end of the Monitor task.
      // Set up the watchdog timer to interrupt every 8 seconds.
      MCUSR &= ~(1<<WDRF);      
      WDTCSR |= (1<<WDCE) | (1<<WDE);    
      WDTCSR = 1<<WDP0 | 1<<WDP3; // 8.0 seconds
      WDTCSR |= _BV(WDIE);
      // Schedule the sleep state.
      tasksState[MONITOR_TASK] = MONITOR_SLEEP;
      break;

    case MONITOR_SLEEP:
      // Enter this state if we are supposed to attempt to sleep.
      // If all stayAwakeFlags are cleared then it is okay to
      // to sleep.
      if(keepAwakeFlags == 0)
      {
          set_sleep_mode(SLEEP_MODE_PWR_SAVE);
          delay(300);
          sleep_mode();  
          //
          // SLEEPING
          //
          // WAKEUP<----------------An interrupt has occured.
          sleep_disable();
          // Re-enable the peripherals.
          //power_all_enable();
        // Here when the MPU wakes up due to an interrupt. The interrupt
        // may have been the watchdog or External interrupt 0. External
        // Interrupt 0 is tied to the normally open momentary C type
        // contact in the rain bucket. If the wakeup was due to the 
        // external interrupt then the was interrupt detached in the 
        // interrupt routine itself and will not occur until it is
        // reattached some time later on. However the signal (switch)
        // may still be closed or the contact may be bouncing. One 
        // thing we know for sure is that the rain bucket cannot 
        // physically tip again before at least a full watchdog time
        // out period or (8 seconds). In the bucket tipped interrupt
        // set the switch delay to BUCKET_SWITCH_DELAY to gaurantee
        // that that many full watchdog periods pass before the 
        // interrupt is aeattached.
        // There are three cases:
        //  1.  Watchdog woke us up. bucket tip may happen at any 
        //      moment.
        //  2.  Bucket tip woke us up don't worry about the wacthdog
        //      it will only set the flag anyway.
//*******************BEGIN DIAGNOSTIC CODE**************************
//*******************BEGIN DIAGNOSTIC CODE**************************
        Serial.print("Wakeup ");
        if(bucketTipped)
          Serial.println("from Bucket");
        else if(consoleInput)
          Serial.println("from Console");
        else
          Serial.println("from Watchdog");          
        delay(300);      
//********************END DIAGNOSTIC CODE***************************
//********************END DIAGNOSTIC CODE***************************      
        // Up date the minutes since the last report was made.
        monitorSecsSinceReport+=WDT_SLEEP_SECONDS;     
 
        // Check for a bucket tip.
        cli();
        if(bucketTipped)
        {
          // The rain bucket's interrupt woke us up.
          sei();
          // Indicate that the DataLogger() tassk should delay past
          // any switch bounce or momentary closure.
          switchDelay = BUCKET_SWITCH_DELAY;
          // Schedule the DataLogger() task to record the datapoint.
          taskScheduled[DATA_LOGGER_TASK] = true;
          tasksState[DATA_LOGGER_TASK] = TASK_INIT_STATE; 
          // Go wait for the data to be logged.
          tasksState[MONITOR_TASK] = MONITOR_WAIT_RECORD;
        }
        else if(consoleInput)
        {
          // The console interrupt woke us up.
          sei(); 
          keepAwakeFlags |= (1<<CONSOLE_TASK);
          consoleInput = false;
          taskScheduled[CONSOLE_TASK] = true;
        }
        else
        {
          // The watch dog interrupt woke us up.
           sei();
//*******************BEGIN DIAGNOSTIC CODE**************************
//*******************BEGIN DIAGNOSTIC CODE**************************
//Commented out for test.
// Remain in the sleep-wakeup mode for testing.
//          // Go see if itg is time to report to the database.
//          taskScheduled[DATA_REOPORT_TASK] = true;
//          tasksState[DATA_REOPORT_TASK] = TASK_INIT_STATE;   
//          // Go wait for the database report.
//          tasksState[MONITOR_TASK] = MONITOR_WAIT_REPORT;
//********************END DIAGNOSTIC CODE***************************
//********************END DIAGNOSTIC CODE***************************      
        }
      }    // End of if(keepAwakeFlags == 0)
      break;
      
      case MONITOR_WAIT_RECORD:
      // Here to wait for the DataRecord() task to complete.
      if(!taskScheduled[DATA_LOGGER_TASK])
      {
        // The datapoint has been recorded. Now see if it is
        // time to report and data to the database.
        taskScheduled[DATA_REOPORT_TASK] = true;
        tasksState[DATA_REOPORT_TASK] = TASK_INIT_STATE;   
        // Go wait for the database report.
        tasksState[MONITOR_TASK] = MONITOR_WAIT_REPORT;
      }
      // Else continue to wait here.
      break;

      case MONITOR_WAIT_REPORT:
        if(!taskScheduled[DATA_REOPORT_TASK])
          // Go sleep.
          tasksState[MONITOR_TASK] = MONITOR_SLEEP;
        // else continue to wait here.
      break;
    }  //  End if switch(tasksState[MONITOR_TASK])
}
// Enter this watchdog interrupt handler any time that 
// the watch dog timer times out.
ISR(WDT_vect)
{
  // We may want to ignore the wattch dog sometimes.
  watchDogFired = true;
}
// Enter this interrupt handler any time that the rain
// bucket's normally open contact is closed or bouncing.
// Enter only once per tip because this interrupt routine
// detached immediately after entry. It is not reattached
// until latter in the monitor routine or the RtcReader
// task.
void BucketTip()
{
  detachInterrupt(EXT_INTERRUPT_0);
  switchDelay = BUCKET_SWITCH_DELAY + 1;
  bucketTipped = true;
} 


// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
unsigned long monitorTimeoutPeriod;
unsigned long monitorPreviousMillis;
unsigned long monitorElapsedTime;

void monitorSetTimeout( unsigned long t)
{
 monitorTimeoutPeriod = t;
 monitorPreviousMillis = millis();
}
boolean monitorTimeout() 
{
  unsigned long currentMillis = millis();
  
  //check if time has rolled over
  if ( currentMillis < monitorPreviousMillis)
  {
    monitorElapsedTime = MAX_UNSIGNED_LONG - monitorPreviousMillis + currentMillis;
    if (monitorElapsedTime >= monitorTimeoutPeriod)
    {
      return 1; //timeout period has been reached
    }
    else
    {
      return 0; //timeout period has not been reached
    }  
  }
  //time has not rolled over, simply compute elaspedTime
  else
  {
    monitorElapsedTime = currentMillis - monitorPreviousMillis;
//    
//    Serial.print("monitorTimeout() monitorElapsedTime = ");
//    Serial.print(monitorElapsedTime,DEC);
//    Serial.print("  monitorTimeoutPeriod = ");
//    Serial.print(monitorTimeoutPeriod);
//    Serial.print("  currentMillis = ");
//    Serial.println(currentMillis);
//
    if (monitorElapsedTime >= monitorTimeoutPeriod)
    {
      //Serial.print("Returning 1 ");
      return true; //timeout period has been reached
    }
    else
    {
       //Serial.print("Returning 0 ");
      return false; //timeout period has not been reached
    }
  }
}


