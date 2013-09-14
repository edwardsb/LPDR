/*****
  HOW TO ADD A TASK:
    Add a MyNewTask.ino file using TaskTemplate.ino template.
    Replace TEMPLATE_TASK with YOUR_TASK.
    Replace TaskTemTask with YourTask.
    Add an index that identifies the new task added i.e.
      #define YOUR_TASK        n     in Definitions.h
    Initialize the new task entries in setup();
         taskPointer[YOUR_TASK] = NewTask;
         taskScheduled[YOUR_TASK] = false;
         tasksState[YOUR_TASK] = TASK_INIT_STATE;    
    Add and code the states, for the new task, as required.
  HOW TO SCHEDULE A TASK:
    Make sure that the task is not already scheduled.
      Check taskScheduled[] with interrupts off.
        Set the entry state newTaskState[]
        Set taskScheduled[] true
    Task is now running. To end the task:
        Set taskScheduled[] false
  SHARED RESOURCES:
    All have fifos.
    All mechanized by regular tasks.
    All controlled by a control structure.
    System log
    File system
    
*/
#include "Definitions.h"
 


void setup() 
{
  // All hardware and software setup is done in the setup task.
  SetupTask();
}

// THE EXECUTIVE TASK SCHEDULER
void loop() 
{
  String dateTimeString;
//    // If the task is scheduled then call it.
//    for(int taskIdx = 0; taskIdx < MAX_TASKS; taskIdx++)
//    {
//      if(taskScheduled[taskIdx])
//      {
//          taskPointers[taskIdx]();
//      }
//    }

      Serial.println(ReadTimeDate(dateTimeString));
      delay(5000);
}

