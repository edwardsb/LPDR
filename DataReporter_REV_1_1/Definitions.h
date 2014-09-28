

#ifndef __DATAREPORTER_DEF_1
  #define __DATAREPORTER_DEF_1
  enum result
  {
    NOT_STARTED,
    FAIL,
    SUCCESS  
  };
#endif

//*******************TASK NUMBERS**********************
// The following assign numbers to all tasks included in the
// DataReporter.
#define MONITOR_TASK          0
#define DATA_LOGGER_TASK      1
#define DATA_REOPORT_TASK     2
#define SYSTEMLOG_TASK        3
#define CONSOLE_TASK          4
#define RTC_TASK              5
#define SDRW_TASK             6
#define GPRS_TASK             7
#define MDMSIM_TASK           8
#define TEMPLATE_TASK         9
#define MAX_TASKS             10

//All tasks must have an initial state "case TASK_INIT_STATE
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
// XBEE_POWER_ENABLE is tied to the SEL1 and SEL2 of the
// 3.3 VDC 250 mA Boost-Buck LTC3245 regulator. This
// regulator supplies the XBee coordinator with power. It 
// initialized to the low state in Setup.
#define XBEE_POWER_ENABLE         12          // Active high
// XBEE_POWER_GOOD indicates if the XBee's power is within 6%
// of is't final nominal value of 3.3 DC. The signal goes to
// the open state to indicate. XBEE_POWER_GOOD is initialized
// as an input with no pullup in Setup.
#define XBEE_POWER_GOOD           6
// XBEE_RESET_NOT is tied directly to the XBee coordinator's
// RESET line. It is initialized in Setup to the high
// inactive state.
#define XBEE_RESET_NOT             11          // Active low.

#define SOLAR_REG_ENABLE          10          // Active high.
// The PG output is low whenever the input to the 
// Battery Manager (MCP73871) device is above the UVLO
// threshold and greater than the battery voltage. See
// Table 5-1 Pg. 25 in MCP73871 data sheet.
#define BATTERY_MNGR_PGNOT        9  // Input with pullup resistor.
// See Table 5-1 Pg. 25 in MCP73871 data sheet.
#define BATTERY_MNGR_STAT2        8  // Input with pullup resistor.
// See Table 5-1 Pg. 25 in MCP73871 data sheet.
#define BATTERY_MNGR_STAT1_LBO    7  // Input with pullup resistor.

#define SD_PWR_ENABLE             6            // Active HIGH.
#define SD_SPI_SELECT             5            // Active low.
#define RTC_SPI_SELECT            4            // Active low.

// End of Arduino Mega Pro I/O Port Pin assignments

//// Arduino Mega Pro (AtMega2560) I/O Port Pin assignments
//#define SOLAR_REG_ENABLE  9          // Active high.          <----------------------Move all up one. Start at 10
//// The PG output is low whenever the input to the 
//// Battery Manager (MCP73871) device is above the UVLO
//// threshold and greater than the battery voltage. See
//// Table 5-1 Pg. 25 in MCP73871 data sheet.
//#define BATTERY_MNGR_PGNOT        8  // Input with pullup resistor.
//// See Table 5-1 Pg. 25 in MCP73871 data sheet.
//#define BATTERY_MNGR_STAT2        7  // Input with pullup resistor.
//// See Table 5-1 Pg. 25 in MCP73871 data sheet.
//#define BATTERY_MNGR_STAT1_LBO    6  // Input with pullup resistor.
//
//#define SD_PWR_ENABLE 5            // Active HIGH.
//#define SD_SPI_SELECT 4             // Active low.
//#define RTC_SPI_SELECT 3           // Active low.
//
//// End of Arduino Mega Pro I/O Port Pin assignments


// The ATmega2560 External Interrupt 4, Port E Pin 4 is wired
// to the Sparkfun Mega Pro board's pin marked PWM 2.This is 
// the Arduino's Digital Pin 2 and is also Arduino's External
// Interrupt 0. This is the DataReporter's Bucket Tipped
// Interrupt. These definitions make this connection.
#define EXT_INTERRUPT_0  0
#define DIGITAL_PIN_2  2
// The ATmega2560 External Interrupt 5, Port E Pin 5 is wired
// to the Sparkfun Mega Pro board's pin marked PWM 3.This is 
// the Arduino's Digital Pin 3 and is also Arduino's External
// Interrupt 1. This external interrupt. is tied to  the
// Sparkfun Mega Pro board's pin marked RX0<-0. RX0 is the
// Arduino IDE's serial transmit signal. External interrupt1
// interrupts and wakes up the 2560 any time that the signal
// is low. These definitions make this connection.
#define EXT_INTERRUPT_1  1
#define DIGITAL_PIN_3    3

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
#define MAX_UNSIGNED_LONG 0xFFFFFFFF
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

#define QUEUE_EMPTY    0

// The following define the index into the System log message
// pointer task0MsgPointers[] array in LogMessages.ino file
#define SYSLOG_MSGBUF_SIZE     30


#define SYS_LOG_FILE          "SYSLOG.TXT"
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
#define DATE_TIME_ARGUMENTS 6    // Arguments required for RTC set.
// End of DS3234 RTC chip Definitions.

// ATmega2560 Register definitions.
#define PRR0    0x64
#define PRR1    0x65

// The monitor task has the following states.
#define MONITOR_SLEEP                        1
#define MONITOR_WAIT_RECORD                  2
#define MONITOR_WAIT_REPORT                  3

// Define the character in the time date string
// returned by the ReadTimeDate() function.
#define SETUP_MONTH_START      0
#define SETUP_MONTH_END      2
#define SETUP_YEAR_START       6
#define SETUP_YEAR_END       8


#define DATA_QUEUE1_FILE  "DataQ_1.txt"
#define DATA_QUEUE2_FILE  "DataQ_2.txt"

#define RTC_SEC_ADDR        0
#define RTC_MIN_ADDR        1
#define RTC_HOUR_ADDR       2
#define RTC_DAY_ADDR        3
#define RTC_DATE_ADDR       4
#define RTC_MONTH_ADDR      5
#define RTC_YEAR_ADDR       6

#define MAX_UNSIGNED_LONG 0xFFFFFFFF


  // The following define the index into the System log message
  // pointer task0MsgPointers[] array in LogMessages.ino file
  #define SYSLOG_MSGBUF_SIZE     30

  //***************SYSTEM_LOG_MESSAGES*************************
  #define CNSL_TEST_MSG         0
  #define GPRS_REGISTER_MSG     1
//
// GprsControl task communication and control stuff.
//
#define GPRS_REGISTER_COMMANDS 14
#define GPRS_M10_PWRKEY 7

//
// Definitions associated with the analog signal monitoring.
//
#define ANALOG_NO_OF_CHANNELS      4


// The following define the Arduino Mega Pro (AtMega2560) A/D
// channel assignments for voltages monitored by the DataReporter.
#define SOLAR_OUTPUT_MONITOR  0
#define BATTERY_MONITOR  1
#define RFEGULATOR_MONITOR  2
#define LOAD_MONITOR  3
// The following define the scale factors to convert the 10 bit
// A/D readings to floating point voltages.
// Monitored voltages are first scaled to 0 to 2.5 with resistor
// dividers.
#define  SOLAR_SF  0.028758950            // 47K to 4.7K
#define  BATTERY_SF  0.004174085     // 6.8K to 4.7K
#define  REGULATOR_SF  0.006257822   // 6.8K to 4.7K
#define  LOAD_SF  0.006144431       // 6.8K to 4.7K

// End of A/D channel assignments.

