//
// The Arduino IDE console task provides a means of controling
// the testing of the DataReporter.
//

#include "Definitions.h"
#include "Rtc.h"
#include "SysLog.h"
#include <SPI.h>
#include <SD.h>
#include <String.h>

extern boolean consoleInput;
extern int keepAwakeFlags;
extern boolean monitorTimeout();
extern void monitorSetTimeout( unsigned long t);
extern rtcControl cnslRtcControl;

//
// Local definitions go here.
//
#define CNSL_GET_RTC_ARGS    1
#define CNSL_SCHED_RTC_TASK  2
#define CNSL_WAIT_RTC        3
#define CNSL_SYSLOG_MSG      4
#define CNSL_WAIT_SYSLOG     5

#define CNSL_RTC_INPUT_WAIT  5000  // 10 * mSecs = 10 seconds/character.
#define CNSP_RTC_SET_LENGTH  12    // exactly 12 digits mmddyyhhnnss.
//
// Local globals go here.
//
File myFile;
String rtcSetString("");

rtcControl cnslRtcControl;
sysLogControl cnslSysLogControl;

// Enter here only from the exec.
void ConsoleTask(void)      
{
  static int timeWaiting;
  switch(tasksState[CONSOLE_TASK])
  {
    case TASK_INIT_STATE:
       
    if(Serial.available() != 0)  // check for serial data
    {
      switch(Serial.read())      // see which command we received
      {
        //*********************************************
        //*********************************************
        case 'R':
          Serial.println("Set RTC.");
          // Let Monitor know not to sleep until we finish
          // setting the clock.
          keepAwakeFlags |= (1<<CONSOLE_TASK);
          // Schedule the console task to get the arguments 
          // for setting the RTC from the IDE Serial Console
          // input.
          tasksState[CONSOLE_TASK] = CNSL_GET_RTC_ARGS;
          // Let op know what the input format for setting the
          // RTC.
          Serial.println("Input the 12 digit RTC time as mmddyyhhnnss");
          // Initialize the timeout time.
          monitorSetTimeout(CNSL_RTC_INPUT_WAIT);
          // Clear the input string buffer.
          rtcSetString = "";
        break;        
        //*********************************************
        //*********************************************
        case 'S':
          Serial.println("Entering a test message into the System Log.");
          
          // Let Monitor know not to sleep until we finish
          // setting the clock.
          keepAwakeFlags |= (1<<CONSOLE_TASK);
          // Schedule the console task to write a test message
          // to the System Log.
          tasksState[CONSOLE_TASK] = CNSL_SYSLOG_MSG;
        break;        
        //*********************************************
        //*********************************************
        case 's':
          Serial.println("Solar Regulator Enable.");
          digitalWrite(SOLAR_REG_ENABLE, HIGH);
          consoleExit();
        break;        
        case 'x':
          Serial.println("Solar Regulator Disable.");
          digitalWrite(SOLAR_REG_ENABLE, LOW);
          consoleExit();
        break;
         //*********************************************
         //*********************************************  
        case 'e':
          Serial.println("SD Card Power Enable.");
          digitalWrite(SD_PWR_ENABLE, HIGH);
          consoleExit();
        break;        
        case 'd':
          Serial.println("SD Card Power Disable.");
          digitalWrite(SD_PWR_ENABLE, LOW);
          consoleExit();
        break;
        //*********************************************
        //*********************************************  
//        case 'I':
//          Serial.println("SD Card Initialization.");
//          InitializeSdCaed();
//          Serial.println("SD Card Initialization Complete.");
//          consoleExit();
//        break;     
        case 't':
          Serial.println("SD Card Testing Started.");
          TestSdOperation();
          Serial.println("SD Card Testing Completed");
          consoleExit();
        break;        
        //*********************************************
        //*********************************************  
        case 'm':
          MakeAnalogMeasurements();
          consoleExit();
        break;        
        //*********************************************
        //*********************************************  
        default:
          Serial.println("Illegal command.");
          consoleExit();
        break;
      }  //  End of switch(Serial.read())      
    }  // End of if(Serial.available() != 0)
    else 
    {
      Serial.println("Error console scheduled with no input?");
      delay(1000);
      if(Serial.available() > 1) // if we received too many bytes then clear the buffer
      Serial.flush();          
    }
    break;
    
    case CNSL_GET_RTC_ARGS:
      // We are here to get the RTC date-time string from the 
      // IDE's serial monitor. The input consists exactly of 
      // the following string:
      //      mm/dd/yy hh:nn:ss
      // the string may or may not be followed by LF and or NL.
      // We are going to remain in this state until we collect
      // the 17 characters from the console or we timeout 
      // waiting for a character.
      if(Serial.available() != 0)  // check for serial data
      {
        // We have a character reset the time waiting.
        monitorSetTimeout((unsigned long) CNSL_RTC_INPUT_WAIT);
        // Save the character in our string object.
        rtcSetString+=(char)Serial.read();
        // See if we have received enough characters to form
        // the RTC set string.
        if(rtcSetString.length() >= CNSP_RTC_SET_LENGTH)
          tasksState[CONSOLE_TASK] = CNSL_SCHED_RTC_TASK;
        // else continue to wait in this state.        
      }
      else
      {
//        boolean timeout = monitorTimeout();
//          Serial.println("Console monitorTimeout() returned timeout = 0x");
//          Serial.println(timeout, HEX);
        
        // No character yet. Have we waited too long?
        if(monitorTimeout())
        {
          Serial.println("Console timed out.");
          // We have timed out waiting let Op know, deschedule
          // the Console task and attach the IDE console character
          // received interrupt.
          Serial.println("Timed out waiting.");
          taskScheduled[CONSOLE_TASK] = false;
          keepAwakeFlags &= (!(1<<CONSOLE_TASK));
          attachInterrupt(EXT_INTERRUPT_1, AnyIdeConsoleInput, LOW);
        }
        // else continuing in this state.
      }
    break;
    
    case CNSL_SCHED_RTC_TASK:

      Serial.print("Console case CNSL_SCHED_RTC_TASK String = ");
      Serial.println(rtcSetString);
      delay(300);
      // Since the RTC is a shared resource we have to queue
      // a request to the RTC task and wait for the results.
      // Fill out the RC control structure.
      cnslRtcControl.op = RTC_SET_DATETIME;              // Set the RTC
      cnslRtcControl.data.rtcSetString = &rtcSetString;  // Date-time
      cnslRtcControl.stat = NOT_STARTED;                 // Status
      // Set console state to wait for RTC task to complete.
      tasksState[CONSOLE_TASK] = CNSL_WAIT_RTC;
      // Push the RTC request onto the RTC's FIFO queue.
      cli();    // Interrupts off.
      PushRtc(&cnslRtcControl) ;
      sei();    // Interrupts on.
    break;
    
    case CNSL_WAIT_RTC:
      // Here waiting for the RTC task to complete.
      if(cnslRtcControl.stat == SUCCESS)
        consoleExit();
    
    break;
    case CNSL_SYSLOG_MSG://***************************************************************************************************************************************
      // We are here to write a test message to the system 
      // log file,Syslog.txt, on the SD card. Since the SD
      // card is a shared resource we have to queue up a
      // request and wait for the system log task to process
      // it.
      
      // This actually requires that 3 threads be run to 
      // acomplish: SysLogTask, Rtc and SdReadWrite.
      // The numbers of parameters to include in the system
      // log entry.
      Serial.println("In case : CNSL_SYSLOG_MSG");
      // Fill out the system log control structure.
      cnslSysLogControl.msgIdx = CNSL_TEST_MSG; // Set index to message.
      cnslSysLogControl.stat = NOT_STARTED;                 // Status
//      cnslSysLogControl.type = SYSLOG_INTEGER;
//      cnslSysLogControl.validParnmCount = SYSLOG_MAX_INTPARMS;
//      cnslSysLogControl.parameter.intParm[0] = 1;
//      cnslSysLogControl.parameter.intParm[1] = 2;
      
      cnslSysLogControl.type = SYSLOG_FLOAT;
      cnslSysLogControl.validParnmCount = SYSLOG_1_PARAMETER;
      cnslSysLogControl.parameter.floatParm = 12345.6789;
      
      // Set console state to wait for RTC task to complete.
      tasksState[CONSOLE_TASK] = CNSL_WAIT_SYSLOG;
      // Push the RTC request onto the RTC's FIFO queue.
      cli();    // Interrupts off.
      PushSysLog(&cnslSysLogControl);
      sei();    // Interrupts on.    
    break;
    
    case CNSL_WAIT_SYSLOG:
      // We are here to wait for the write to the system
      // log request to complete.
      if(cnslSysLogControl.stat == SUCCESS)
        consoleExit();
      
    default:
    
    break;
  }
  
}

// Enter this interrupt handler any time that any character
// is received from the Arduino IDE Monitor.
void AnyIdeConsoleInput()
{
  detachInterrupt(EXT_INTERRUPT_1);
  consoleInput = true;
} 



//
// The TestSdOperation() Routine performs a set of SD
// card SD operations and outputs the results.
//
void TestSdOperation()
{
  // Open the file.
  myFile = SD.open("test.txt", FILE_WRITE);
  // Perform the tests using the File object and a
  // pointer to the File object.
  File *myFilePtr;  
  myFilePtr = &myFile;
  // if the file opened okay, write to it:
  if (*myFilePtr) {
    Serial.print("#1 Writing to test.txt...");
    myFilePtr->println("testing 1, 2, 3.\n");
    myFilePtr->println("testing 4, 5, 6.\n");
	// close the file:
    myFilePtr->close();
    Serial.println("#1 Done writting to the test file second time.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("#1 error opening test.txt");
  }

  // re-open the file for reading:
  // You can use the File object itself or you can use
  // a pointer to the file object.
  myFile = SD.open("test.txt");  
   
  // Using the File object pointer:
  if(*myFilePtr)
  {
    Serial.println("#1 test.txt opened for reading:");
    
    // read from the file until there's nothing else in it:
    while (myFilePtr->available()) 
    	Serial.write(myFilePtr->read());
    // close the file:
    myFilePtr->close();
    SD.remove("test.txt");
  } 
  else 
  {
  	// if the file didn't open, print an error:
    Serial.println("#1 error opening test.txt for reading.");
  }

// Using the object itself. 
   myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("#2 Writing to test.txt...");
    myFile.println("testing 1, 2, 3.\n");
    myFile.println("testing 4, 5, 6.\n");
	// close the file:
    myFile.close();
    Serial.println("#2 Done writting to the test file second time.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("#2 error opening test.txt");
  }

  myFile = SD.open("test.txt");

  if (myFile) {
    Serial.println("#2 test.txt opened for reading:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    	Serial.write(myFile.read());
    SD.remove("test.txt");
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("#2 error opening test.txt for reading.");
  }
}  // End of TestSdOperation() routine.




  //
  // Measure and display the following voltages:
  //    Solar Panel Output.
  //    Solar Regulator Output.
  //    Li-ion battery.
  void MakeAnalogMeasurements()
  {
    solarOut_A0 = analogRead(SOLAR_OUTPUT_MONITOR);
    delay(100);
    solarPanelVolts = solarOut_A0 * SOLAR_SF;
    
    battery_A1 = analogRead(BATTERY_MONITOR);
    delay(100);
    batteryVolts = battery_A1 * BATTERY_SF;
    
    regulator_A2 = analogRead(RFEGULATOR_MONITOR);
    delay(100);
    regulatorVolts = regulator_A2 * REGULATOR_SF;
    
    load_A3 = analogRead(LOAD_MONITOR);
    delay(100);
    loadVolts = load_A3 * LOAD_SF;
    
    Serial.print("Solar = ");
    Serial.print(solarPanelVolts , DEC);
    Serial.print("(");
    Serial.print(solarOut_A0 , DEC);
   Serial.print(") ");
  
    Serial.print(" Battery = ");
    Serial.print(batteryVolts , DEC);
    Serial.print("(");
    Serial.print(battery_A1 , DEC);
    Serial.print(") ");
  
    Serial.print(" Regulator = ");
    Serial.print(regulatorVolts , DEC);
    Serial.print("(");
    Serial.print(regulator_A2 , DEC);
    Serial.print(") ");
  
    Serial.print(" Load = ");
    Serial.print(loadVolts , DEC);
    Serial.print("(");
    Serial.print(load_A3 , DEC);
    Serial.println(")");
  }
  
        
// At this point we have completed the Console task, whatever
// it was. Now deschedule the console task and reattach the
// console interrupt.
void consoleExit()
{
  taskScheduled[CONSOLE_TASK] = false;
  tasksState[CONSOLE_TASK] = TASK_INIT_STATE;
  tasksState[MONITOR_TASK] = MONITOR_SLEEP;
  keepAwakeFlags &= (!(1<<CONSOLE_TASK));
  attachInterrupt(EXT_INTERRUPT_1, AnyIdeConsoleInput, LOW);
  // Give the output to the IDE Console time to flush.
   delay(300);
   
  // Read the IDE console's XT line to make sure that it has
  // gone HIGH and stays there before reattaching the interrupt.
  while( digitalRead(DIGITAL_PIN_3) == LOW )
  {
    //Serial.println("ConsoleInterrupting.");
    delay(1000);
  }
}


