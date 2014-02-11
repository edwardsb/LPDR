//
// The Arduino IDE console task provides a means of controling
// the testing of the DataReporter.
//

#include "Definitions.h"
#include "Rtc.h"
#include "SysLog.h"
#include <SPI.h>
#include <SD.h>
#include <gprsOperation.h>
#include <String.h>

extern boolean consoleInput;
extern int keepAwakeFlags;
extern boolean monitorTimeout();
extern void monitorSetTimeout( unsigned long t);
extern rtcControl cnslRtcControl;

//
// Local definitions go here.
//
#define CNSL_GET_RTC_ARGS        1
#define CNSL_SCHED_RTC_TASK      2
#define CNSL_WAIT_RTC            3
#define CNSL_SYSLOG_MSG          4
#define CNSL_WAIT_SYSLOG         5
#define CNSL_ENABLE_MODEM        6
#define CNSL_WAIT_MODEM          7
#define CNSL_WAIT_SERVER_DATA    8
#define CNSL_WAIT_ABORT          9

#define CNSL_RTC_INPUT_WAIT  5000  // 10 * mSecs = 10 seconds/character.
#define CNSP_RTC_SET_LENGTH  12    // exactly 12 digits mmddyyhhnnss.

#define CNSL_SERVER_DATA_TIMEOUT    30000  // 30 seconds.
#define CNSL_SERVER_ABORT_TIMEOUT   1000   // 1 second for nextcharacter.
//
// Local globals go here.
//
String rtcSetString("");

char cnslServer[] = "arduino.cc";
char cnslPath[] = "/asciilogo.txt";
char cnslModemInput;
rtcControl cnslRtcControl;
sysLogControl cnslSysLogControl;
gprsControl cnslGprsControl;

unsigned long cnslTimeoutPeriod;
unsigned cnslPreviousMillis;

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

        case 'M':
          Serial.println("Enabling Quectel M10 Modem.");
          // Let Monitor know not to sleep until we finish
          // setting the clock.
          keepAwakeFlags |= (1<<CONSOLE_TASK);
          // Schedule the console task to get the arguments 
          // for setting the RTC from the IDE Serial Console
          // input.
          tasksState[CONSOLE_TASK] = CNSL_ENABLE_MODEM;
        break;        



      }  //  End of switch(Serial.read())      
    }  // End of if(Serial.available() != 0)
    else 
    {
      Serial.println("Error console scheduled with no input?");
      delay(1000);
      if(Serial.available() > 1) // if we received too many bytes then clear the buffer
      Serial.flush();     
      consoleExit();     
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
    case CNSL_SYSLOG_MSG:
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
      cnslSysLogControl.type = SYSLOG_FLOAT;
      cnslSysLogControl.validParnmCount = SYSLOG_1_PARAMETER;
      cnslSysLogControl.parameter.floatParm = 12345.6789;
      
      // Set console state to wait for system log task to complete.
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
      break;
      
    case CNSL_ENABLE_MODEM://***********************************************************************************************************************
      // This Quectel M10 modem test consists of:
      //  1.  Power on the modem
      //  2.  Request the modem resource:
      //    A.  Register.
      //    B.  Configure the modem for GPRS data
      //        transfers.
      //    C.  Connect to the requested server.
      //    D.  Leave the modem in the data mode.
      //  3.  Allow this task to communicate with
      //      specified server.
      //  4.  Power off the modem.
      //
      // Initialize the modem control structure.
      cnslGprsControl.gprsStatus = GPRS_POWER_OFF;
      cnslGprsControl.clientStatus = GPRS_CLIENT_WAIT;
      cnslGprsControl.server = cnslServer;
      cnslGprsControl.port = 80;
      // Push the request onto the GPRS operations FIFO
      // queue.
      cli();  //Interrupts off.
      PushGprs(&cnslGprsControl);
      sei();  //Interrupts on
      tasksState[CONSOLE_TASK] = CNSL_WAIT_MODEM;
    break;  
    
    case CNSL_WAIT_MODEM:
      // Wait for the modem to register and connect to
      // to my server.
      if(cnslGprsControl.gprsStatus == GPRS_DATA_EXCHANGE)
      {
        // We are connected to the Arduino server through
        // the modem. We now directly control the modem
        // until we are through talking to the server. Let
        // the op know and request that the Arduino logo 
        // be sent.
        
        Serial.println("connected");
        // Make an HTTP GET request:
        Serial1.print("GET ");
        Serial1.print(cnslPath);
        Serial1.println(" HTTP/1.1");
        Serial1.print("Host: ");
        Serial1.println(cnslServer);
        Serial1.println("Connection: close");
        Serial1.println(); 
        cnslSetTimeout(CNSL_SERVER_DATA_TIMEOUT); 
        tasksState[CONSOLE_TASK] = CNSL_WAIT_SERVER_DATA;
        Serial.println("Waiting 30 seconds for server data.");
        Serial.println("type a<ENTER> to abort.");      
      }
      else if(cnslGprsControl.gprsStatus == GPRS_OPERATION_ERROR)
      {
        // There was a problem registering, configuring
        // the modem or connecting to the server.Tell op 
        // what the error was. The GprsOperations() task
        // has written a message to the system log file.
        // The following information would not be used by 
        // the GprsOperations() client.
        Serial.println("Error registering or configuring the modem.");
        Serial.println("See the system log.");
        consoleExit();        
      }
      // else continue to wait.    
    break;

    case CNSL_WAIT_SERVER_DATA:
      char sentChar;
      // See if we have waited too long.
      if(!cnslTimeout())
      {
        // Check for an abort entry from the IDE's Serial
        // Monitor.
        if(Serial.available() != 0)
        {
          // We have received an input from the IDE
          // Serial Monitor See if op wants to abort.          
          sentChar = Serial.read();
          if(sentChar == 'a')
          {
          // The op wants to abort any further input from
          // the server. We need to flush all serial input
          // from the IDE and modem. Set a new time out, 
          // wait in another state to drain any further
          // characters that the server or IDE may be 
          // sending.
            Serial.flush();
            Serial1.flush();
            // There may still be more characters comming.
            // Continue to flush until we'er sure there is 
            // no more left.
            cnslSetTimeout(CNSL_SERVER_ABORT_TIMEOUT); 
            tasksState[CONSOLE_TASK] = CNSL_WAIT_ABORT;
            Serial.println("Wait while aborting.");
          }
          // else continue to wait.
        }        
        else if(Serial1.available() != 0)
        {
          // Output anything that the server sends to 
          // the IDE monitor.
          sentChar = Serial1.read();
          Serial.print(sentChar);        
        }
        // else continue to wait.        
      }
      else
      {
        // We have timed out waiting for the server
        // to respond with the requested page.
        Serial.println("Timed out waiting for server response.");
        cnslSetTimeout(CNSL_SERVER_ABORT_TIMEOUT); 
        tasksState[CONSOLE_TASK] = CNSL_WAIT_ABORT;
        Serial.println("Wait while aborting.");
      }
    break;
    
    case CNSL_WAIT_ABORT:
      // Flush the the serial inputs is they have 
      // any data.
      if(cnslTimeout())
      {
        // We have waited for any further input from
        // the op and the server make sure that any
        // further data received during the delay
        // is discarded.
        if(Serial.available() || Serial1.available())
        {
          Serial.flush();
          Serial1.flush();
          // Wait a little longer to detect any further
          // further input from the opor server..
          cnslSetTimeout(CNSL_SERVER_ABORT_TIMEOUT); 
        }
        else
        {
          // There is no more input from the server or
          // the op. Give up the modem resource. 
          cnslGprsControl.clientStatus = GPRS_CLIENT_DONE;
          consoleExit();
        }
      }
    break;
    
    default:
      consoleExit();
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
  File myile;
  // Open the file.
  myile = SD.open("test.txt", FILE_WRITE);
  // Perform the tests using the File object and a
  // pointer to the File object.
  File *myFilePtr;  
  myFilePtr = &myile;
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
  myile = SD.open("test.txt");  
   
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
   myile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myile) {
    Serial.print("#2 Writing to test.txt...");
    myile.println("testing 1, 2, 3.\n");
    myile.println("testing 4, 5, 6.\n");
	// close the file:
    myile.close();
    Serial.println("#2 Done writting to the test file second time.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("#2 error opening test.txt");
  }

  myile = SD.open("test.txt");

  if (myile) {
    Serial.println("#2 test.txt opened for reading:");
    
    // read from the file until there's nothing else in it:
    while (myile.available()) 
    	Serial.write(myile.read());
    SD.remove("test.txt");
    // close the file:
    myile.close();
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
  
    Serial.print(" Regulator = ");
    Serial.print(regulatorVolts , DEC);
    Serial.print("(");
    Serial.print(regulator_A2 , DEC);
    Serial.print(") ");
  
    Serial.print(" Load = ");
    Serial.print(loadVolts , DEC);
    Serial.print("(");
    Serial.print(load_A3 , DEC);
    Serial.print(")");
    
    Serial.print(" Battery = ");
    Serial.print(batteryVolts , DEC);
    Serial.print("(");
    Serial.print(battery_A1 , DEC);
    Serial.println(") ");
  
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
  consoleInput = false;
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


// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
void cnslSetTimeout( unsigned long t)
{
 cnslTimeoutPeriod = t;
 cnslPreviousMillis = millis();
}
boolean cnslTimeout() 
{
  unsigned long elapsedTime;
  unsigned long currentMillis = millis();  
  //check if time has rolled over
  if ( currentMillis < cnslPreviousMillis)
  {
    elapsedTime = 
      MAX_UNSIGNED_LONG - cnslPreviousMillis + currentMillis;
    if (elapsedTime >= cnslTimeoutPeriod)
      return 1; //timeout period has been reached
    else
      return 0; //timeout period has not been reached
  }
  //time has not rolled over, simply compute elaspedTime
  else
  {
    elapsedTime = currentMillis - cnslPreviousMillis;

    if (elapsedTime >= cnslTimeoutPeriod)
      return true; //timeout period has been reached
    else
      return false; //timeout period has not been reached
  }
}
 
//          Serial.println("gsrpResponseCount = ");
//          Serial.println(gsrpResponseCount, DEC);
//          Serial.println("gprsCharInCount = ");
//          Serial.println(gprsCharInCount, DEC);
//          Serial.println("gprsResponses = ");
//          Serial.println(gprsResponses);
//          Serial.println("gprsConfigureStep = ");
//          Serial.println(gprsConfigureStep, DEC);
//          Serial.println("gsrpCmdLength = ");
//          Serial.println(gsrpCmdLength, DEC);
//          Serial.println("gprsModemOutParseState = ");
//          Serial.println(gprsModemOutParseState, DEC);
//          Serial.println("gprsModemOutputCount = ");
//          Serial.println(gprsModemOutputCount, DEC);
//          Serial.println("gprsAtResponseCount = ");
//          Serial.println(gprsAtResponseCount, DEC);
//          Serial.println("gprsOtherResponseCount = ");
//          Serial.println(gprsOtherResponseCount, DEC);
//          Serial.println("gprsTextCount = ");
//          Serial.println(gprsTextCount, DEC);
//          Serial.println("gprsValidTextCount = ");
//          Serial.println(gprsValidTextCount, DEC);
//          Serial.println("gprsErrorCount = ");
//          Serial.println(gprsErrorCount, DEC);

