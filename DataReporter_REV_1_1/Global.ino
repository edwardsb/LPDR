

#include "Definitions.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

  

// Variables associated with the system log queue.

// Scheduler variables.
taskPointer taskPointers[MAX_TASKS];
bool taskScheduled[MAX_TASKS];        // bool works, boolean does not???
int tasksState[MAX_TASKS];
// End of scheduler variables.

// The processor is forced into the sleep mode by the Monitor
// thread that is always scheduled. Other tasks may have
// processes to complete during which the processor must remain
// awake. To maccomplish this the tasks use the keepAwakeFlags
// variable. Each task has a flag bit, indicated by the task
// number, in keepAwakeFlags which it sets or clears to
// prevent or allow the Monitor task from putting the processor
// to sleep respectively. The task number 0 --> 15 indicates
// the assigned bit 0 (LSB) to 15 (MSB). Bit set indicates
// do not sleep.
int keepAwakeFlags;

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
// End of the RTC variables.

// System Log variables.
File sysLogFile; 
char *sysLogFileName = SYS_LOG_FILE;// System Log variables.
// End of System Log variables.
// End of variables associated with the system log queue.




//// Variables associated with the system log queue.
//// SystemLog operation control structure.
//struct rtcControl
//{ 
//int operation;      // Index into LogMessages task0MsgPointers[] array
//int validParnmCount;
//int parameter_1;
//int parameter_2;
//int parameter_3; 
//};
//sysLogControl sysLogQueue[QUEUE_MAX_ITEMS];
//int sysLogQueueCount = 0;
//int sysLogQueueInIdx = 0;
//int sysLogQueueOutIdx = 0;
//
//// System Log variables.
//File sysLogFile; 
//char *sysLogFileName = SYS_LOG_FILE;// System Log variables.
//bool sdBusy = false;
//// Create an array in RAM to access the messages.
//char sysLogMsgBuff[SYSLOG_MSGBUF_SIZE];
//// End of System Log variables.
//// End of variables associated with the system log queue.
 






// Variables associated with the bucket.
//struct bucketControl
//{ 
//  sdOpeation sdOp;
//  result sdOpStatus;
//  int sdErr;
//  unsigned long timeStamp;
//};
//// The bucketQueue array holds any bucket tips detected
//// during the time that reports are being generated by
//// the Pi. See: Monitor.void BucketTip() handler.
//bucketControl bucketQueue[QUEUE_MAX_ITEMS];
//// The queueBucketTips flag, when set indicates that
//// bucket tips are to be queued for inclusion in the
//// bucket tip SD file at a later time.
//boolean queueBucketTips;
////File bucketTipFile; 
////char *bucketTipFileName = BUCKET_TIPS_FILE ;// System Log variables.
//// insertBucketTip is the, zero based, byte position, in the Bucket 
//// Tips file, where the next bucket record will be written.
//// removeBucketTip is the, zero based, byte position, in the Bucket
//// Tips file, where the next bucket record should be read from.
//unsigned long insertBucketTip = 0;
//unsigned long removeBucketTip = 0;
//// bucketRdWrtTaskCtl is used only to control the 
//// SdReadrite task
//bucketControl bucketRdWrtTaskCtl;
//int bucketQueueCount = 0;
//int bucketQueueInIdx = 0;
//int bucketQueueOutIdx = 0;

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


// The consoleInput flag is set true any time that any 
// character is received from the Arduino IDE Monitor.
// See Console.AnyIdeConsoleInput() interrupt routine.
boolean consoleInput;
// When dataQueue1Busy is true it indicates that data points,
// currently being recorded are placed in DataQ_1.txt and that
// the DataQ_2.txt contains data points, if any, are ready to be
// transmitted to the cloud database.
volatile bool dataQueue1Busy;
// dataQueue1Count and dataQueue2Count indicate the number of data
// points ready to be transmitted to the cloud based datatbase.
int dataQueue1Count;
int dataQueue2Count;

File dataQ_1_File;
File dataQ_2_File;

