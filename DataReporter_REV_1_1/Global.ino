

#include "Definitions.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

// Enumerations.
enum sdOpeation
{
  SD_READ,
  SD_WRITE
};
enum result
{
  NOT_STARTED,
  FAIL,
  SUCCESS  
};
// End of enumerations.
// Scheduler variables.
taskPointer taskPointers[MAX_TASKS];
bool taskScheduled[MAX_TASKS];        // bool works, boolean does not???
int tasksState[MAX_TASKS];
// End of scheduler variables.

// sdRecords contains the number of bucket tip entries recorded into
// the BucketTips.txt file on the SD card. Data points (bucket tips)
// cannot be recorded to the database at all times so they are
// stored in the BucketTips.txt file until reporting is performed.

// The following are used communicate with the RtcReader task.
// Create a RTC_DS1307 class object.
//unixTimeString???...
String unixTimeString;
// unixTime???...
unsigned long unixTime;
// The rela time clock is a shared resource but the reading of
// the RTC does not require a task state machine. Tasks requiring
// the date and time must capture the resource for exclusive use.
// Turn the interrupts off and test the flag. If  it is not true
// then set the flag, turn the interrupts on and get the date-time.
// set the flag false when complete.
bool rtcBusy = false;
// End of the RTC variables.

// Variables associated with the system log queue.
// SystemLog operation control structure.
struct sysLogControl
{ 
int msgIdx;      // Index into LogMessages task0MsgPointers[] array
int validParnmCount;
int parameter_1;
int parameter_2;
int parameter_3; 
};
sysLogControl sysLogQueue[QUEUE_MAX_ITEMS];
int sysLogQueueCount = 0;
int sysLogQueueInIdx = 0;
int sysLogQueueOutIdx = 0;

//File sysLogFile;
// End of variables associated with the system log queue.


// Variables associated with the Data Logging queue.
// Data Logging operation control structure.
struct dataLogControl
{ 
  int idx;      // Index into dataLogQueue[] array
  int type;
  String timeStamp;   // mm-dd-yy hh:mm:ss
  String value;      // Float or integer
};
dataLogControl dataLogQueue[QUEUE_MAX_ITEMS];
int dataLogQueueCount = 0;
int dataLogQueueInIdx = 0;
int dataLogQueueOutIdx = 0;
// End of variables associated with the system log queue.



// Variables associated with the bucket.
struct bucketControl
{ 
  sdOpeation sdOp;
  result sdOpStatus;
  int sdErr;
  unsigned long timeStamp;
};
// The bucketQueue array holds any bucket tips detected
// during the time that reports are being generated by
// the Pi. See: Monitor.void BucketTip() handler.
bucketControl bucketQueue[QUEUE_MAX_ITEMS];
// The queueBucketTips flag, when set indicates that
// bucket tips are to be queued for inclusion in the
// bucket tip SD file at a later time.
boolean queueBucketTips;
//File bucketTipFile; 
//char *bucketTipFileName = BUCKET_TIPS_FILE ;// System Log variables.
// insertBucketTip is the, zero based, byte position, in the Bucket 
// Tips file, where the next bucket record will be written.
// removeBucketTip is the, zero based, byte position, in the Bucket
// Tips file, where the next bucket record should be read from.
unsigned long insertBucketTip = 0;
unsigned long removeBucketTip = 0;
// bucketRdWrtTaskCtl is used only to control the 
// SdReadrite task
bucketControl bucketRdWrtTaskCtl;
int bucketQueueCount = 0;
int bucketQueueInIdx = 0;
int bucketQueueOutIdx = 0;

// The bucketTipped flag is set only in the DrMonitor.BucketTip()
// interrupt (EXT_INTERRUPT_0) routine. DrMonitor.BucketTip() is
// entered on the falling edge of the of the rain bucket's 
// momentary switch. The EXT_INTERRUPT_0 interrupt is detached
// (disabeled) in BucketTip(). Once the bucket tips you must wait
// for the contact to stop bouncing and wait for the momentary 
// closure to end before reattaching (enabling) the interrupt
// to catch the next bucket tip.
bool bucketTipped;
// switchDelay, when non-zero, indicates how many sleep periods
// to delay before reattaching the rain bucket's tip interrupt.
// It is set only in DrMonitor.BucketTip(). switchDelay is used
// the DrMonitor task to determine how long to wait before
// reattaching the bucket tip interrupt.
int switchDelay;
// End of variables associated with the bucket queue.

File datapointFile;

// System Log variables.
File sysLogFile; 
char *sysLogFileName = SYS_LOG_FILE;// System Log variables.
bool sdBusy = false;
// Create an array in RAM to access the messages.
char sysLogMsgBuff[SYSLOG_MSGBUF_SIZE];
// End of System Log variables.



