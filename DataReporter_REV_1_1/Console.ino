//
// The Arduino IDE console task provides a means of controling
// the testing of the DataReporter.
//

//  taskPointers[CONSOLE_TASK] = ConsoleTask;
//  taskScheduled[CONSOLE_TASK] = true;
//  tasksState[CONSOLE_TASK] = TASK_INIT_STATE;

#include "Definitions.h"
#include <SD.h>

extern boolean consoleInput;
extern int keepAwakeFlags;

//
// Local definitions go here.
//
#define CNSL_STATE_1    1
#define CNSL_STATE_2    2
//
// Local globals go here.
//
File myFile;

// Enter here only from the exec.
void ConsoleTask(void)      
{
  switch(tasksState[CONSOLE_TASK])
  {
    case TASK_INIT_STATE:
       
    if(Serial.available() != 0)  // check for serial data
    {
      switch(Serial.read())      // see which command we received
      {
        //*********************************************
        //*********************************************
        case 's':
          Serial.println("Solar Regulator Enable.");
          digitalWrite(SOLAR_REG_ENABLE, HIGH);
        break;        
        case 'x':
          Serial.println("Solar Regulator Disable.");
          digitalWrite(SOLAR_REG_ENABLE, LOW);
        break;
         //*********************************************
         //*********************************************  
        case 'e':
          Serial.println("SD Card Power Enable.");
          digitalWrite(SD_PWR_ENABLE, HIGH);
        break;        
        case 'd':
        Serial.println("SD Card Power Disable.");
          digitalWrite(SD_PWR_ENABLE, LOW);
        break;
        //*********************************************
        //*********************************************  
//        case 'I':
//          Serial.println("SD Card Initialization.");
//          InitializeSdCaed();
//          Serial.println("SD Card Initialization Complete.");
//        break;     
//        case 't':
//          Serial.println("SD Card Testing Started.");
//          TestSdOperation();
//          Serial.println("SD Card Testing Completed");
//        break;        
        //*********************************************
        //*********************************************  
        case 'm':
          MakeAnalogMeasurements();
        break;        
        //*********************************************
        //*********************************************  
        default:
          Serial.println("Illegal command.");
        break;
      }  //  End of switch(Serial.read())
      
      // Give the output to the IDE Console time to flush.
       delay(300);
       
      // At this point we have completed the Console task, whatever
      // it was. Now deschedule the console task, reset the Console
      // task entry state and reattach the console interrupt.
      
      // Read the IDE console's XT line to make sure that it has
      // gone HIGH and stays there before reattaching the interrupt.
      while( digitalRead(DIGITAL_PIN_3) == LOW )
      {
        //Serial.println("ConsoleInterrupting.");
        delay(1000);
      }
      taskScheduled[CONSOLE_TASK] = false;
      tasksState[MONITOR_TASK] = MONITOR_SLEEP;
      keepAwakeFlags &= (!(1<<CONSOLE_TASK));
      attachInterrupt(EXT_INTERRUPT_1, AnyIdeConsoleInput, LOW);

    }  // End of if(Serial.available() != 0)
    else 
    {
      Serial.println("Error console scheduled with no input?");
      delay(1000);
      if(Serial.available() > 1) // if we received too many bytes then clear the buffer
      Serial.flush();          
    }
    break;
    
    case CNSL_STATE_1:
    
    break;
    
    case CNSL_STATE_2:
    
    break;
    
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
//  1.  Initialize the SD card hardware.
//  2.  
void TestSdOperation()
{

  // Open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.\n");
    myFile.println("testing 4, 5, 6.\n");
	// close the file:
    myFile.close();
    Serial.println("Done writting to the test file second time.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt opened for reading:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    	Serial.write(myFile.read());
    SD.remove("test.txt");
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt for reading.");
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
