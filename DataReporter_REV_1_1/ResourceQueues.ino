 /*
  This file contains the queue mechanizations, push and pop,
  for the resources shared by the various tasks of the
  LpcsDataMngr sketch. 
  
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
#include "SdReadWrite.h".h"

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

































//// Push the caller's system log control structure pointer 
//// onto the Queue. If the SystemLog task is not currently
//// scheduled then schedule it.
//int PushSysLog(struct sysLogControl *controlPtr)  // Requireing struct is Arduino issue.
//{
  int returnIdx;
//  //
//  // The conditions of the Queue may be :
//  //   1.  The Queue may be empty.
//  //       A.  sysLogQueueInIdx = sysLogQueueOutIdx.
//  //       B.  sysLogQueueCount = 0.
//  //   2.  The Queue may not be empty.
//  //       A.  sysLogQueueInIdx <> sysLogQueueOutIdx.
//  //       B.  sysLogQueueCount <> 0
//  //   3.  The Queue may be full.
//  //       A.  sysLogQueueInIdx = sysLogQueueOutIdx.
//  //       B.  sysLogQueueCount <> 0.
//  //
//  // In any case the new data point gets inserted into the FIFO.
//  // If it's full then then the oldest data point is lost.
//  returnIdx = sysLogQueueInIdx;
//  sysLogQueue[sysLogQueueInIdx] = *controlPtr;
//  // Examine each case above and set the indexes accordingly.
//  if(sysLogQueueInIdx = sysLogQueueOutIdx)
//  {
//    //
//    // The In Index = the Out Index. The FIFO is full or empty.
//    //
//    if(sysLogQueueCount == 0)
//    {
//      //
//      // The FIFO is empty. Advance the in index only.
//      //
//      sysLogQueueInIdx++;
//      sysLogQueueInIdx%=QUEUE_MAX_ITEMS;
//      sysLogQueueCount++;
//    }
//    else
//    {
//      //
//      // The FIFO is full. Advance the in and out indexs. Do not
//      // increment the count.
//      //
//      sysLogQueueInIdx++;
//      sysLogQueueInIdx%=QUEUE_MAX_ITEMS;
//      sysLogQueueOutIdx++;
//      sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//  }
//  else
//  {
//    //
//    // The FIFO is not empty. Advance the in index and advance the
//    // count.
//    //
//    sysLogQueueInIdx++;
//    sysLogQueueInIdx%=QUEUE_MAX_ITEMS;
//    sysLogQueueCount++;
//  }
//  // If the queue is not empty and the system log task is not 
//  // scheduled then schedule the system log task.
//  if(sysLogQueueCount != 0 && !taskScheduled[SYSTEMLOG_TASK])
//  {
//    tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE; 
//    taskScheduled[SYSTEMLOG_TASK] = true;
//  }
//  // Copy the caller's structure to the queue and return
//  // the index into the queue.
//  sysLogQueue[returnIdx].msgIdx = controlPtr->msgIdx;
//  sysLogQueue[returnIdx].validParnmCount = controlPtr->validParnmCount;
//  sysLogQueue[returnIdx].parameter_1 = controlPtr->parameter_1;
//  sysLogQueue[returnIdx].parameter_2 = controlPtr->parameter_2;
//  sysLogQueue[returnIdx].parameter_3 = controlPtr->parameter_3;
//  return(returnIdx);
//}
//
////
//// This function returns an index to the next ystem log control
//// structure from the sysLogQueue() if there are any items
//// in the queue or QUEUE_EMPTY if the queue is empty.
////
//int PopSysLog()
//{
//  int popSysLogStatus;
//  //
//  // The conditions here are:
//  //   1.  The FIFO may be empty.
//  //       A.  Return False.
//  //   2.  The FIFO may not be empty.
//  //       A.  InIndex <> OutIndex.
//  //       B.  Count <> 0
//  //   3.  The FIFO may be full.
//  //       A.  InIndex = OutIndex.
//  //       B.  Count <> 0.
//  //
//  if( sysLogQueueCount != 0)
//  {
//    //
//    // The FIFO is not empty return an index to the command
//    // control structure removed.
//    //
//    popSysLogStatus = sysLogQueueOutIdx;
//    if(sysLogQueueInIdx == sysLogQueueOutIdx)
//    {
//      //
//      // The In Index = the Out Index. The FIFO is full and we have
//      // removed one data point. Advance the out index only.
//      //
//      sysLogQueueOutIdx++;
//      sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    else
//    {
//        //
//        // The FIFO is not empty and not full.
//        //
//        sysLogQueueOutIdx++;
//        sysLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    //
//    // Decrement the number of data points in the FIFO.
//    //
//     sysLogQueueCount--;
//  }
//  else
//  {
//    //
//    // The FIFO is empty and we return empty.
//    //
//     popSysLogStatus = QUEUE_EMPTY;
//  }
//  return(popSysLogStatus);
//}
//
//
//
//
//// Push the caller's bucket control structure onto the 
//// Queue. 
//int PushBucket(struct bucketControl *controlPtr)  // Requireing struct is Arduino issue.
//{
//  int returnIdx;
//  //
//  // The conditions of the Queue may be :
//  //   1.  The Queue may be empty.
//  //       A.  bucketQueueInIdx = bucketQueueOutIdx.
//  //       B.  bucketQueueCount = 0.
//  //   2.  The Queue may not be empty.
//  //       A.  bucketQueueInIdx <> bucketQueueOutIdx.
//  //       B.  bucketQueueCount <> 0
//  //   3.  The Queue may be full.
//  //       A.  bucketQueueInIdx = bucketQueueOutIdx.
//  //       B.  bucketQueueCount <> 0.
//  //
//  // In any case the new data point gets inserted into the FIFO.
//  // If it's full then then the oldest data point is lost.
//  returnIdx = bucketQueueInIdx;
//  bucketQueue[bucketQueueInIdx] = *controlPtr;
//  // Examine each case above and set the indexes accordingly.
//  if(bucketQueueInIdx = bucketQueueOutIdx)
//  {
//    //
//    // The In Index = the Out Index. The FIFO is full or empty.
//    //
//    if(bucketQueueCount == 0)
//    {
//      //
//      // The FIFO is empty. Advance the in index only.
//      //
//      bucketQueueInIdx++;
//      bucketQueueInIdx%=QUEUE_MAX_ITEMS;
//      bucketQueueCount++;
//    }
//    else
//    {
//      //
//      // The FIFO is full. Advance the in and out indexs. Do not
//      // increment the count.
//      //
//      bucketQueueInIdx++;
//      bucketQueueInIdx%=QUEUE_MAX_ITEMS;
//      bucketQueueOutIdx++;
//      bucketQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//  }
//  else
//  {
//    //
//    // The FIFO is not empty. Advance the in index and advance the
//    // count.
//    //
//    bucketQueueInIdx++;
//    bucketQueueInIdx%=QUEUE_MAX_ITEMS;
//    bucketQueueCount++;
//  }
//  // Copy the caller's structure to the queue and return
//  // the index into the queue.
//  bucketQueue[returnIdx].sdOp = controlPtr->sdOp;
//  bucketQueue[returnIdx].sdOpStatus = controlPtr->sdOpStatus;
//  bucketQueue[returnIdx].sdErr = controlPtr->sdErr;
//  bucketQueue[returnIdx].timeStamp = controlPtr->timeStamp;
//  return(returnIdx);
//}
//
////
//// This function returns an index to the next ystem log control
//// structure from the bucketQueue() if there are any items
//// in the queue or QUEUE_EMPTY if the queue is empty.
////
//int PopBucket()
//{
//  int popBucketStatus;
//  //
//  // The conditions here are:
//  //   1.  The FIFO may be empty.
//  //       A.  Return False.
//  //   2.  The FIFO may not be empty.
//  //       A.  InIndex <> OutIndex.
//  //       B.  Count <> 0
//  //   3.  The FIFO may be full.
//  //       A.  InIndex = OutIndex.
//  //       B.  Count <> 0.
//  //
//  if( bucketQueueCount != 0)
//  {
//    //
//    // The FIFO is not empty return an index to the command
//    // control structure removed.
//    //
//    popBucketStatus = bucketQueueOutIdx;
//    if(bucketQueueInIdx == bucketQueueOutIdx)
//    {
//      //
//      // The In Index = the Out Index. The FIFO is full and we have
//      // removed one data point. Advance the out index only.
//      //
//      bucketQueueOutIdx++;
//      bucketQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    else
//    {
//        //
//        // The FIFO is not empty and not full.
//        //
//        bucketQueueOutIdx++;
//        bucketQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    //
//    // Decrement the number of data points in the FIFO.
//    //
//     bucketQueueCount--;
//  }
//  else
//  {
//  //
//  // The FIFO is empty and we return empty.
//  //
//   popBucketStatus = QUEUE_EMPTY;
//  }
//  return(popBucketStatus);
//}
//
////*************************************************************************************************************
////*************************************************************************************************************
////*************************************************************************************************************
////*************************************************************************************************************
////*************************************************************************************************************
////*************************************************************************************************************
//
//// Push the caller's data log control structure onto the 
//// Queue. If the LogData task is not currently scheduled
//// then schedule it.
int PushDataLog(struct dataLogControl *controlPtr)  // Requireing struct is Arduino issue.
{
  int returnIdx;
//  //
//  // The conditions of the Queue may be :
//  //   1.  The Queue may be empty.
//  //       A.  dataLogQueueInIdx = dataLogQueueOutIdx.
//  //       B.  dataLogQueueCount = 0.
//  //   2.  The Queue may not be empty.
//  //       A.  dataLogQueueInIdx <> dataLogQueueOutIdx.
//  //       B.  dataLogQueueCount <> 0
//  //   3.  The Queue may be full.
//  //       A.  dataLogQueueInIdx = dataLogQueueOutIdx.
//  //       B.  dataLogQueueCount <> 0.
//  //
//  // In any case the new data point gets inserted into the FIFO.
//  // If it's full then then the oldest data point is lost.
//  returnIdx = dataLogQueueInIdx;
//  dataLogQueue[dataLogQueueInIdx] = *controlPtr;
//  // Examine each case above and set the indexes accordingly.
//  if(dataLogQueueInIdx = dataLogQueueOutIdx)
//  {
//    //
//    // The In Index = the Out Index. The FIFO is full or empty.
//    //
//    if(dataLogQueueCount == 0)
//    {
//      //
//      // The FIFO is empty. Advance the in index only.
//      //
//      dataLogQueueInIdx++;
//      dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
//      dataLogQueueCount++;
//    }
//    else
//    {
//      //
//      // The FIFO is full. Advance the in and out indexs. Do not
//      // increment the count.
//      //
//      dataLogQueueInIdx++;
//      dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
//      dataLogQueueOutIdx++;
//      dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//  }
//  else
//  {
//    //
//    // The FIFO is not empty. Advance the in index and advance the
//    // count.
//    //
//    dataLogQueueInIdx++;
//    dataLogQueueInIdx%=QUEUE_MAX_ITEMS;
//    dataLogQueueCount++;
//  }
//  // If the queue is not empty and the LogDAta task is not 
//  // scheduled then schedule the LogData task.
//  if(dataLogQueueCount != 0 && !taskScheduled[SYSTEMLOG_TASK])
//  {
//    tasksState[DATA_LOGGER_TASK] = TASK_INIT_STATE; 
//    taskScheduled[DATA_LOGGER_TASK] = true;
//  }
//  // Copy the caller's structure to the queue and return
//  // the index into the queue.
//  dataLogQueue[returnIdx].idx = controlPtr->idx;
//  dataLogQueue[returnIdx].type = controlPtr->type;
//  dataLogQueue[returnIdx].timeStamp = controlPtr->timeStamp;
//  dataLogQueue[returnIdx].value = controlPtr->value;
  return(returnIdx);
}
//
////
//// This function returns an index to the next ystem log control
//// structure from the dataLogQueue() if there are any items
//// in the queue or QUEUE_EMPTY if the queue is empty.
////
int PopDataLog()
{
  int popDataLogStatus;
//  //
//  // The conditions here are:
//  //   1.  The FIFO may be empty.
//  //       A.  Return False.
//  //   2.  The FIFO may not be empty.
//  //       A.  InIndex <> OutIndex.
//  //       B.  Count <> 0
//  //   3.  The FIFO may be full.
//  //       A.  InIndex = OutIndex.
//  //       B.  Count <> 0.
//  //
//  if( dataLogQueueCount != 0)
//  {
//    //
//    // The FIFO is not empty return an index to the command
//    // control structure removed.
//    //
//    popDataLogStatus = dataLogQueueOutIdx;
//    if(dataLogQueueInIdx == dataLogQueueOutIdx)
//    {
//      //
//      // The In Index = the Out Index. The FIFO is full and we have
//      // removed one data point. Advance the out index only.
//      //
//      dataLogQueueOutIdx++;
//      dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    else
//    {
//        //
//        // The FIFO is not empty and not full.
//        //
//        dataLogQueueOutIdx++;
//        dataLogQueueOutIdx%=QUEUE_MAX_ITEMS;
//    }
//    //
//    // Decrement the number of data points in the FIFO.
//    //
//     dataLogQueueCount--;
//  }
//  else
//  {
//    //
//    // The FIFO is empty and we return empty.
//    //
//     popDataLogStatus = QUEUE_EMPTY;
//  }
  return(popDataLogStatus);
}
//
//

