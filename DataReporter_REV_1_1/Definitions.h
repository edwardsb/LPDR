


//*******************TASK NUMBERS**********************
// The following assign numbers to all tasks included in the
// DataReporter.
#define MONITOR_TASK          0
#define DATA_LOGGER_TASK      1
#define DATA_REOPORT_TASK     2
#define SYSTEMLOG_TASK        3
#define MAX_TASKS             4
// End of tasks number assignments.

// End of all tasks included in the DataReporter.

//All tasks must have an initial state "case TASK_INIT_STATE::
#define TASK_INIT_STATE    0  

// SD card file names.
#define SYS_LOG_FILE          "Syslog.txt"
#define DATA_HOLDING_FILE      "DataHold.txt"
// End of SD card file names.

// Some of the task functions are asynchronously shared by 
// other tasks exclusivity is controlled by fifo queues.
#define QUEUE_MAX_ITEMS 10

// Type definitions.
typedef void (*taskPointer)(void);

// End of type definitions.


// Some common global references.
extern taskPointer taskPointers[];
extern bool        taskScheduled[];
extern int         tasksState[];
extern int stayAwakeFlags;
extern int sdRecords;
extern bool bucketTipped;
extern int switchDelay;
//extern DateTime rtcNow;
//extern String unixTimeString;
extern unsigned long unixTime;
extern bool rtcBusy;
extern bool sdBusy;
// End of common global references.

// Arduino Mega Pro (AtMega2560) I/O Port Pin assignments
#define SOLAR_REG_ENABLE  9          // Active high.
#define BATTERY_CHARGE_DISABLE  8    // Active low
#define RTC_SPI_SELECT 7            // Active low.
#define SD_SPI_SELECT 6             // Active low.
#define FLASH_SPI_SELECT 5          // Active low.
#define MODEM_ONKEY 4               // Active low (momentary).
#define MODEM_ATCMD_READY 3
#define MODEM_NETWORK_READY 2
// End of Arduino Mega Pro I/O Port Pin assignments

// The following define the Arduino Mega Pro (AtMega2560) A/D
// channel assignments for voltages monitored by the DataReporter.
#define SOLAR_OUTPUT_MONITOR = 2;
#define RFEGULATOR_5VDC_MONITOR = 0;
#define BATTERY_MONITOR = 1;

#define SOLAR_SF = 0.026;
#define REGULATOR_SF = 0.00970;
#define BATTERY_SF = 0.0082;
// End of A/D channel assignments.

// The ATmega2560 External Interrupt 2, Port D Pin 2 is the wired to
// the Arduino Mega Pro board's pin marked RX0<-0. The following two
// definitions make this connection.
#define EXT_INTERRUPT_0  0
#define DIGITAL_PIN_0  0

// BUCKET_SWITCH_DELAY the minimum number of watchdog 
// timeout periods must occur, after the begining of 
// the bucket tip is first closed (causes an interrupt
// and wakeup) and the next time that a bucket tip can
// cause the next interrupt. The watchdog period is 
// WDT_SLEEP_SECONDS seconds.
#define BUCKET_SWITCH_DELAY          2  // A minimum of 2 * 8 seconds.
// The watchdog interrupt is enabled and interrupts every:
#define WDT_SLEEP_SECONDS    8
// INTERIM_REPORT_DATAPOINTS  specifies the maximum number of data 
// points stored before entering them into the data base.
#define INTERIM_REPORT_DATAPOINTS       2
// SECS_MAX_PER_REPORT specified the maximum time between report if
// no datapoints are recorded.
#define SECS_MAX_PER_REPORT             3600
// CONTACT_BOUNCE_MAX defines the maximum mSec allowed for bucket
// contact close and bounce.
#define CONTACT_BOUNCE_MAX         2000
// MAX_UNSIGNED_LONG???...
#define MAX_UNSIGNED_LONG 0xFFFFFFFF;
// The maximum number of valid ASCII decimal digits 
// + '\r' + '\n' in an unsigned long.
#define DATAPOINT_MAX_LENGTH            (10 + 2)

// SD task definitions.
#define SDTASK_NO_DATA                  1
#define SDCARD_READ_ERROR               2
#define SDCARD_WRITE_ERROR              3
#define SDCARD_POSITION_ERROR           4
#define SDCARD_LENGTH_ERROR             5
#define SDCARD_NOCHAR_ERROR             6
#define SDCARD_ILGCHAR_ERROR            7

#define QUEUE_EMPTY    -1

// The following define the index into the System log message
// pointer task0MsgPointers[] array in LogMessages.ino file
#define SYSLOG_MSGBUF_SIZE     30


#define SYS_LOG_FILE          "Syslog.txt"
#define DATA_LOGGING_FILE      "DataLogging.txt"

// DS3234 RTC chip Definitions to match data sheet.
  #define SPI_WRITE              0x80
  #define SPI_READ               0x00
// Control and status registers.
  #define RTC_CONTROL_REGISTER   0x0E
// Status and control register bit assignments.
  #define RTC_EOSE    7
  #define RTC_BBSQW   6
  #define RTC_CONV    5
  #define RTC_RS2     4
  #define RTC_RS1     3
  #define RTC_INTCN   2
  #define RTC_A2IE    1
  #define RTC_A1IE    0
// Daste-time string fixed length.
#define DATE_TIME_LENGTH    18



// End of DS3234 RTC chip Definitions.

