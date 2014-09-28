//
// This file contains the required Arduino setup() and loop()
// functions and the DataReporter's Executive Task Scheduler
// that drives the DataReporter's associated task state
// machines:
//    Monitor
//    LogData
//    ReportData
// All hardware and software initialization is done in the 
// SetupTask().
#include "Definitions.h"
#include "GprsOperation.h"
//*******************BEGIN DIAGNOSTIC CODE**************************
//*******************BEGIN DIAGNOSTIC CODE**************************

int solarOut_A0;                   // Solar Panel A/D Channel 0 binary reading.
int battery_A1;                    // Battery A/D Channel 1 binary reading.
int regulator_A2;                  // 5 VDC regulator output.
int load_A3;                       // 5 VDC regulator output.


float solarPanelVolts;                     // Solar panel output voltage.
float batteryVolts;                 // Battery terminal voltage.
float regulatorVolts;                 // 5 VDC regulator output voltage.
float loadVolts;                 // 5 VDC regulator output voltage.
//********************END DIAGNOSTIC CODE***************************
//********************END DIAGNOSTIC CODE***************************
void setup() 
{
  // VERY IMPORTANT:
  // VERY IMPORTANT:
  // VERY IMPORTANT:
  // Do not use an analog reference other than EXTERNAL because
  // there is an external precision voltage referance connected to
  // the 2560's AREF pin.
  analogReference(EXTERNAL);
   // All hardware and software setup is done in the setup task.
  SetupTask();
delay(3000);  
}

void loop() 
{
//********THE EXECUTIVE TASK SCHEDULER********
    // If the task is scheduled then call it.
    for(int taskIdx = 0; taskIdx < MAX_TASKS; taskIdx++)
      if(taskScheduled[taskIdx])
          taskPointers[taskIdx]();
//******END OF YHE EXECUTIVE TASK SCHEDULER******
}

