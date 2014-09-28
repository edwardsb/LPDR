 /*
  This file contains the queue mechanizations, push and pop,
  for the resources shared by the various tasks of the
  LpcsDataMngr sketch. 
  
  There are 5 shared resource queus:
	1.	RTC QUEUE
	2.	SYSTEM LOG QUEUE
	3.	SD READ WRITE QUEUE
	4.	GPRS OPERATIONS QUEUE
	5.	DATA LOG QUEUE

  The queues are mechanized using an array of control
  structures, e.g. 
     relayControl relayQueue[QUEUE_MAX_ITEMS];
     and
     sysLogControl sysLogQueue[QUEUE_MAX_ITEMS];
 The control structures contain the interface for
 using the shared resource.
 To use the the shared resource:
   1.  Turn the interrupts off.
   2.  Call the push method for the resource, with 
       a pointer to your control structure, and get
       an integer which indexes to your structure in
       the resource's queue.
   3.  Turn the interrupts on.
   4.  Monitor the status in the structure to see if
       the resouirce has finished and the results.
  The queue mechanization handles the scheduling of
  the shared resource's task.
*/
#include "Definitions.h"
#include "Rtc.h"
#include "SysLog.h"
#include "SdReadWrite.h"
#include "GprsOperation.h"
#include "DataLog.h"

#define QUEUE_MAX_ITEMS      5
#define QUEUE_EMPTY    0

  
//**********************************************************************************
//**********************************************************************************
//
//                  RTC QUEUE
//
//**********************************************************************************
//**********************************************************************************
  // Shared resource control queue variables.
  rtcControl *rtcQueue[QUEUE_MAX_ITEMS];
  int rtcQueueCount;
  int rtcQueueInIdx;
  int rtcQueueOutIdx;
 //
// The following PushRtc() and PopRtc() functions mechanize
// first-in-first-out stack for queueing requests for the
// indicated shared resource. The interrupts must be off (cli)
// prior to placing a shared resource request on the queue.
void PushRtc(struct rtcControl *controlPtr)  // Requireing struct is Arduino issue.
{
  //
  // The conditions of the Queue may be :
  //   1.  The Queue may be empty.
  //       A.  rtcQueueInIdx = rtcQueueOutIdx.
  //       B.  rtcQueueCount = 0.
  //   2.  The Queue may not be empty.
  //       A.  rtcQueueInIdx <> rtcQueueOutIdx.
  //       B.  rtcQueueCount <> 0
  //   3.  The Queue may be full.
  //       A.  rtcQueueInIdx = rtcQueueOutIdx.
  //       B.  rtcQueueCount <> 0.
  //
  // In any case the new shared resource control structure
  // pointer gets inserted into the FIFO. If it's full then
  // then the shared resource request is lost.
  rtcQueue[rtcQueueInIdx] = controlPtr;   
  // Examine each case above and set the indexes accordingly.
  if(rtcQueueInIdx == rtcQueueOutIdx)
  {
    //
    // The In Index = the Out Index. The FIFO is full or empty.
    //
    if(rtcQueueCount == 0)
    {
      //
      // The FIFO is empty. Advance the in index only.
      //
      rtcQueueInIdx++;
      rtcQueueInIdx%=QUEUE_MAX_ITEMS;
      rtcQueueCount++;
    }
    else
    {
      //
      // The FIFO is full. Advance the in and out indexs. Do not
      // increment the count.
      //
      rtcQueueInIdx++;
      rtcQueueInIdx%=QUEUE_MAX_ITEMS;
      rtcQueueOutIdx++;
      rtcQueueOutIdx%=QUEUE_MAX_ITEMS;
      //
      // Let the caller know that the FIFO is full and the request
      // is lost.
      controlPtr->stat = FAIL;
    }
  }
  else
  {
    //
    // The FIFO is not empty or full. Advance the in index
    // and advance the count.
    //
    rtcQueueInIdx++;
   // rtcQueueInIdx%=QUEUE_MAX_ITEMS;
    rtcQueueCount++;
  }
  // If the queue is not empty and the RTC task is not 
  // scheduled then schedule the RTC task.
  if(rtcQueueCount != 0 && !taskScheduled[RTC_TASK])
  {
    tasksState[RTC_TASK] = TASK_INIT_STATE; 
    taskScheduled[RTC_TASK] = true;
  }  
}

//
// This function returns a pointer to the next shared
// resource control structure from the queue, if there
// are any items in the queue, else QUEUE_EMPTY if the queue is empty.
//
struct rtcControl *PopRtc()
{
  rtcControl *popRtcStatus;
  //
  // The conditions here are:
  //   1.  The FIFO may be empty.
  //       A.  Return False.
  //   2.  The FIFO may not be empty.
  //       A.  InIndex <> OutIndex.
  //       B.  Count <> 0
  //   3.  The FIFO may be full.
  //       A.  InIndex = OutIndex.
  //       B.  Count <> 0.
  //
  if( rtcQueueCount != 0)
  {
    //
    // The FIFO is not empty return an index to the command
    // control structure removed.
    //
    popRtcStatus = rtcQueue[rtcQueueOutIdx];
    if(rtcQueueInIdx == rtcQueueOutIdx)
    {
      //
      // The In Index = the Out Index. The FIFO is full 
      // and we have removed one entry. Advance the out
      // index only.
      //
      rtcQueueOutIdx++;
      rtcQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    else
    {
        //
        // The FIFO is not empty and not full.
        //
        rtcQueueOutIdx++;
        rtcQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    //
    // Decrement the number of data points in the FIFO.
    //
     rtcQueueCount--;
  }
  else
  {
  //
  // The FIFO is empty and we return empty.
  //
   popRtcStatus = (rtcControl*)QUEUE_EMPTY;
  }
  Serial.print("PopRtc() returning = ");
  Serial.println((unsigned long)popRtcStatus, HEX);
   delay(300);
  return(popRtcStatus);
}










//**********************************************************************************
//**********************************************************************************
//
//                  SYSTEM LOG QUEUE
//
//**********************************************************************************
//**********************************************************************************
//sysLogControl sysLogQueue[QUEUE_MAX_ITEMS];
//int sysLogQueueCount = 0;
//int sysLogQueueInIdx = 0;
//int sysLogQueueOutIdx = 0;
//
  // Shared resource control queue variables.
  sysLogControl *sysLogQueue[QUEUE_MAX_ITEMS];
  int sysLogQueueCount;
  int sysLogQueueInIdx;
  int sysLogQueueOutIdx;

// The following PushSysLog() and PopSysLog() functions mechanize
// first-in-first-out stack for queueing requests for the
// indicated shared resource. The interrupts must be off (cli)
// prior to placing a shared resource request on the queue.
void PushSysLog(struct sysLogControl *controlPtr)  // Requireing struct is Arduino issue.
{
  //
  // The conditions of the Queue may be :
  //   1.  The Queue may be empty.
  //       A.  sysLogQueueInIdx = sysLogQueueOutIdx.
  //       B.  sysLogQueueCount = 0.
  //   2.  The Queue may not be empty.
  //       A.  sysLogQueueInIdx <> sysLogQueueOutIdx.
  //       B.  sysLogQueueCount <> 0
  //   3.  The Queue may be full.
  //       A.  sysLogQueueInIdx = sysLogQueueOutIdx.
  //       B.  sysLogQueueCount <> 0.
  //
  // In any case the new shared resource control structure
  // pointer gets inserted into the FIFO. If it's full then
  // then the shared resource request is lost.
  sysLogQueue[sysLogQueueInIdx] = controlPtr;
  
  // Examine each case above and set the indexes accordingly.
  if(sysLogQueueInIdx == sysLogQueueOutIdx)
  {
    //
    // The In Index = the Out Index. The FIFO is full or empty.
    //
    if(sysLogQueueCount == 0)
    {
      //
      // The FIFO is empty. Advance the in index only.
      //
      sysLogQueueInIdx++;
      sysLogQueueInIdx%=QUEUE_MAX_ITEMS;
      sysLogQueueCount++;
    }
    else
    {
      //
      // The FIFO is full. Advance the in and out indexs. Do not
      // increment the count.
      //
      sysLogQueueInIdx++;
      sysLogQueueInIdx%=QUEUE_MAX_ITEMS;
      sysLogQueueOutIdx++;
      sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
      //
      // Let the caller know that the FIFO is full and the request
      // is lost.
      controlPtr->stat = FAIL;
    }
  }
  else
  {
    //
    // The FIFO is not empty or full. Advance the in index
    // and advance the count.
    //
    sysLogQueueInIdx++;
    sysLogQueueCount++;
  }
  // If the queue is not empty and the RTC task is not 
  // scheduled then schedule the RTC task.
  if(sysLogQueueCount != 0 && !taskScheduled[SYSTEMLOG_TASK])
  {
    tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE; 
    taskScheduled[SYSTEMLOG_TASK] = true;
  }
}

//
// This function returns a pointer to the next shared
// resource control structure from the queue, if there
// are any items in the queue, else QUEUE_EMPTY if the queue is empty.
//
struct sysLogControl *PopSysLog()
{
  sysLogControl *popSysLogStatus;
  //
  // The conditions here are:
  //   1.  The FIFO may be empty.
  //       A.  Return False.
  //   2.  The FIFO may not be empty.
  //       A.  InIndex <> OutIndex.
  //       B.  Count <> 0
  //   3.  The FIFO may be full.
  //       A.  InIndex = OutIndex.
  //       B.  Count <> 0.
  //
  if( sysLogQueueCount != 0)
  {
    //
    // The FIFO is not empty return an index to the command
    // control structure removed.
    //
    popSysLogStatus = sysLogQueue[sysLogQueueOutIdx];
    if(sysLogQueueInIdx == sysLogQueueOutIdx)
    {
      //
      // The In Index = the Out Index. The FIFO is full 
      // and we have removed one entry. Advance the out
      // index only.
      //
      sysLogQueueOutIdx++;
      sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    else
    {
        //
        // The FIFO is not empty and not full.
        //
        sysLogQueueOutIdx++;
        sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    //
    // Decrement the number of data points in the FIFO.
    //
     sysLogQueueCount--;
  }
  else
  {
  //
  // The FIFO is empty and we return empty.
  //
   popSysLogStatus = (sysLogControl*)QUEUE_EMPTY;
  }
  return(popSysLogStatus);
}




//**********************************************************************************
//**********************************************************************************
//
//                  SD READ WRITE QUEUE
//
//**********************************************************************************
//**********************************************************************************
  // Shared resource control queue variables.
  sdrwControl *sdrwQueue[QUEUE_MAX_ITEMS];
  int sdrwQueueCount;
  int sdrwQueueInIdx;
  int sdrwQueueOutIdx;
//
// The following PushSdrw() and PopSdrw() functions mechanize
// first-in-first-out stack for queueing requests for the
// indicated shared resource. The interrupts must be off (cli)
// prior to placing a shared resource request on the queue.
void PushSdrw(struct sdrwControl *controlPtr)  // Requireing struct is Arduino issue.
{
  //
  // The conditions of the Queue may be :
  //   1.  The Queue may be empty.
  //       A.  sdrwQueueInIdx = sdrwQueueOutIdx.
  //       B.  sdrwQueueCount = 0.
  //   2.  The Queue may not be empty.
  //       A.  sdrwQueueInIdx <> sdrwQueueOutIdx.
  //       B.  sdrwQueueCount <> 0
  //   3.  The Queue may be full.
  //       A.  sdrwQueueInIdx = sdrwQueueOutIdx.
  //       B.  sdrwQueueCount <> 0.
  //
  // In any case the new shared resource control structure
  // pointer gets inserted into the FIFO. If it's full then
  // then the shared resource request is lost.
  sdrwQueue[sdrwQueueInIdx] = controlPtr;
  
  // Examine each case above and set the indexes accordingly.
  if(sdrwQueueInIdx == sdrwQueueOutIdx)
  {
    //
    // The In Index = the Out Index. The FIFO is full or empty.
    //
    if(sdrwQueueCount == 0)
    {
      //
      // The FIFO is empty. Advance the in index only.
      //
      sdrwQueueInIdx++;
      sdrwQueueInIdx%=QUEUE_MAX_ITEMS;
      sdrwQueueCount++;
    }
    else
    {
      //
      // The FIFO is full. Advance the in and out indexs. Do not
      // increment the count.
      //
      sdrwQueueInIdx++;
      sdrwQueueInIdx%=QUEUE_MAX_ITEMS;
      sdrwQueueOutIdx++;
      sdrwQueueOutIdx%=QUEUE_MAX_ITEMS;
      //
      // Let the caller know that the FIFO is full and the request
      // is lost.
      controlPtr->stat = FAIL;
    }
  }
  else
  {
    //
    // The FIFO is not empty or full. Advance the in index
    // and advance the count.
    //
    sdrwQueueInIdx++;
    sdrwQueueInIdx%=QUEUE_MAX_ITEMS;
    sdrwQueueCount++;
  }
  // If the queue is not empty and the SD task is not 
  // scheduled then schedule the SD task.
  if(sdrwQueueCount != 0 && !taskScheduled[SDRW_TASK])
  {
    tasksState[SDRW_TASK] = TASK_INIT_STATE; 
    taskScheduled[SDRW_TASK] = true;
  }
  
}

//
// This function returns a pointer to the next shared
// resource control structure from the queue, if there
// are any items in the queue, else QUEUE_EMPTY if the queue is empty.
//
struct sdrwControl *PopSdrw()
{
  sdrwControl *popsdrwStatus;
  //
  // The conditions here are:
  //   1.  The FIFO may be empty.
  //       A.  Return False.
  //   2.  The FIFO may not be empty.
  //       A.  InIndex <> OutIndex.
  //       B.  Count <> 0
  //   3.  The FIFO may be full.
  //       A.  InIndex = OutIndex.
  //       B.  Count <> 0.
  //
  if( sdrwQueueCount != 0)
  {
    //
    // The FIFO is not empty return an index to the command
    // control structure removed.
    //
    popsdrwStatus = sdrwQueue[sdrwQueueOutIdx];
    if(sdrwQueueInIdx == sdrwQueueOutIdx)
    {
      //
      // The In Index = the Out Index. The FIFO is full 
      // and we have removed one entry. Advance the out
      // index only.
      //
      sdrwQueueOutIdx++;
      sdrwQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    else
    {
        //
        // The FIFO is not empty and not full.
        //
        sdrwQueueOutIdx++;
        sdrwQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    //
    // Decrement the number of data points in the FIFO.
    //
     sdrwQueueCount--;
  }
  else
  {
  //
  // The FIFO is empty and we return empty.
  //
   popsdrwStatus = (sdrwControl*)QUEUE_EMPTY;
  }
  return(popsdrwStatus);
}



//**********************************************************************************
//**********************************************************************************
//
//                  GPRS OPERATIONS QUEUE
//
//**********************************************************************************
//**********************************************************************************
  // Shared resource control queue variables.
  gprsControl *gprsQueue[QUEUE_MAX_ITEMS];
  int gprsQueueCount;
  int gprsQueueInIdx;
  int gprsQueueOutIdx;
//
// The following PushGprs() and PopGprs() functions mechanize
// first-in-first-out stack for queueing requests for the
// indicated shared resource. The interrupts must be off (cli)
// prior to placing a shared resource request on the queue.
void PushGprs(struct gprsControl *controlPtr)  // Requireing struct is Arduino issue.
{
  //
  // The conditions of the Queue may be :
  //   1.  The Queue may be empty.
  //       A.  gprsQueueInIdx = gprsQueueOutIdx.
  //       B.  gprsQueueCount = 0.
  //   2.  The Queue may not be empty.
  //       A.  gprsQueueInIdx <> gprsQueueOutIdx.
  //       B.  gprsQueueCount <> 0
  //   3.  The Queue may be full.
  //       A.  gprsQueueInIdx = gprsQueueOutIdx.
  //       B.  gprsQueueCount <> 0.
  //
  // In any case the new shared resource control structure
  // pointer gets inserted into the FIFO. If it's full then
  // then the shared resource request is lost.
  gprsQueue[gprsQueueInIdx] = controlPtr;
  // Examine each case above and set the indexes accordingly.
  if(gprsQueueInIdx == gprsQueueOutIdx)
  {
    //
    // The In Index = the Out Index. The FIFO is full or empty.
    //
    if(gprsQueueCount == 0)
    {
      //
      // The FIFO is empty. Advance the in index only.
      //
      gprsQueueInIdx++;
      gprsQueueInIdx%=QUEUE_MAX_ITEMS;
      gprsQueueCount++;
    }
    else
    {
      //
      // The FIFO is full. Advance the in and out indexs. Do not
      // increment the count.
      //
      gprsQueueInIdx++;
      gprsQueueInIdx%=QUEUE_MAX_ITEMS;
      gprsQueueOutIdx++;
      gprsQueueOutIdx%=QUEUE_MAX_ITEMS;
//      //
//      // Let the caller know that the FIFO is full and the request
//      // is lost.
//      controlPtr->stat = FAIL;
    }
  }
  else
  {
    //
    // The FIFO is not empty or full. Advance the in index
    // and advance the count.
    //
    gprsQueueInIdx++;
    gprsQueueInIdx%=QUEUE_MAX_ITEMS;
    gprsQueueCount++;
  }
  // If the queue is not empty and the GPRS task is not 
  // scheduled then schedule the GPRS task.
  if(gprsQueueCount != 0 && !taskScheduled[GPRS_TASK])
  {
    tasksState[GPRS_TASK] = TASK_INIT_STATE; 
    taskScheduled[GPRS_TASK] = true;
  }
  
}

//
// This function returns a pointer to the next shared
// resource control structure from the queue, if there
// are any items in the queue, else QUEUE_EMPTY if the
// queue is empty.
//
struct gprsControl *PopGprs()
{
  gprsControl *popGprsStatus;
  //
  // The conditions here are:
  //   1.  The FIFO may be empty.
  //       A.  Return False.
  //   2.  The FIFO may not be empty.
  //       A.  InIndex <> OutIndex.
  //       B.  Count <> 0
  //   3.  The FIFO may be full.
  //       A.  InIndex = OutIndex.
  //       B.  Count <> 0.
  //
  if( gprsQueueCount != 0)
  {
    //
    // The FIFO is not empty return an index to the command
    // control structure removed.
    //
    popGprsStatus = gprsQueue[gprsQueueOutIdx];
    if(gprsQueueInIdx == gprsQueueOutIdx)
    {
      //
      // The In Index = the Out Index. The FIFO is full 
      // and we have removed one entry. Advance the out
      // index only.
      //
      gprsQueueOutIdx++;
      gprsQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    else
    {
        //
        // The FIFO is not empty and not full.
        //
        gprsQueueOutIdx++;
        gprsQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    //
    // Decrement the number of data points in the FIFO.
    //
     gprsQueueCount--;
  }
  else
  {
  //
  // The FIFO is empty and we return empty.
  //
   popGprsStatus = (gprsControl*)QUEUE_EMPTY;
  }
  return(popGprsStatus);
}

//**********************************************************************************
//**********************************************************************************
//
//                  DATA LOGGING QUEUE
//
//**********************************************************************************
//**********************************************************************************
 // Shared resource control queue variables.
dataLogControl *dataLogQueue[QUEUE_MAX_ITEMS];
int dataLogQueueCount = 0;
int dataLogQueueInIdx = 0;
int dataLogQueueOutIdx = 0;

 //
// The following PushDataLog() and PopDataLog() functions mechanize
// first-in-first-out stack for queueing requests for the
// indicated shared resource. The interrupts must be off (cli)
// prior to placing a shared resource request on the queue.
void PushDataLog(struct dataLogControl *controlPtr)  // Requireing struct is Arduino issue.
{
  //
  // The conditions of the Queue may be :
  //   1.  The Queue may be empty.
  //       A.  dataLogQueueInIdx = dataLogQueueOutIdx.
  //       B.  dataLogQueueCount = 0.
  //   2.  The Queue may not be empty.
  //       A.  dataLogQueueInIdx <> dataLogQueueOutIdx.
  //       B.  dataLogQueueCount <> 0
  //   3.  The Queue may be full.
  //       A.  dataLogQueueInIdx = dataLogQueueOutIdx.
  //       B.  dataLogQueueCount <> 0.
  //
  // In any case the new shared resource control structure
  // pointer gets inserted into the FIFO. If it's full then
  // then the shared resource request is lost.
  dataLogQueue[dataLogQueueInIdx] = controlPtr;   
  // Examine each case above and set the indexes accordingly.
  if(dataLogQueueInIdx == dataLogQueueOutIdx)
  {
    //
    // The In Index = the Out Index. The FIFO is full or empty.
    //
    if(dataLogQueueCount == 0)
    {
      //
      // The FIFO is empty. Advance the in index only.
      //
      dataLogQueueInIdx++;
      dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
      dataLogQueueCount++;
    }
    else
    {
      //
      // The FIFO is full. Advance the in and out indexs. Do not
      // increment the count.
      //
      dataLogQueueInIdx++;
      dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
      dataLogQueueOutIdx++;
      dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
      //
      // Let the caller know that the FIFO is full and the request
      // is lost.
      controlPtr->stat = FAIL;
    }
  }
  else
  {
    //
    // The FIFO is not empty or full. Advance the in index
    // and advance the count.
    //
    dataLogQueueInIdx++;
   // dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
    dataLogQueueCount++;
  }
  // If the queue is not empty and the RTC task is not 
  // scheduled then schedule the RTC task.
  if(dataLogQueueCount != 0 && !taskScheduled[RTC_TASK])
  {
    tasksState[RTC_TASK] = TASK_INIT_STATE; 
    taskScheduled[RTC_TASK] = true;
  }  
}

//
// This function returns a pointer to the next shared
// resource control structure from the queue, if there
// are any items in the queue, else QUEUE_EMPTY if the queue is empty.
//
struct dataLogControl *PopDataLog()
{
  dataLogControl *popDataLogStatus;
  //
  // The conditions here are:
  //   1.  The FIFO may be empty.
  //       A.  Return False.
  //   2.  The FIFO may not be empty.
  //       A.  InIndex <> OutIndex.
  //       B.  Count <> 0
  //   3.  The FIFO may be full.
  //       A.  InIndex = OutIndex.
  //       B.  Count <> 0.
  //
  if( dataLogQueueCount != 0)
  {
    //
    // The FIFO is not empty return an index to the command
    // control structure removed.
    //
    popDataLogStatus = dataLogQueue[dataLogQueueOutIdx];
    if(dataLogQueueInIdx == dataLogQueueOutIdx)
    {
      //
      // The In Index = the Out Index. The FIFO is full 
      // and we have removed one entry. Advance the out
      // index only.
      //
      dataLogQueueOutIdx++;
      dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    else
    {
        //
        // The FIFO is not empty and not full.
        //
        dataLogQueueOutIdx++;
        dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
    }
    //
    // Decrement the number of data points in the FIFO.
    //
     dataLogQueueCount--;
  }
  else
  {
  //
  // The FIFO is empty and we return empty.
  //
   popDataLogStatus = (dataLogControl*)QUEUE_EMPTY;
  }
  Serial.print("PopDataLog() returning = ");
  Serial.println((unsigned long)popDataLogStatus, HEX);
   delay(300);
  return(popDataLogStatus);
}


