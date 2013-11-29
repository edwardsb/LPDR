//
// This task contains all hardware and software initialization 
// tasks required prior to starting the executive.
//
#include "Definitions.h"

extern File dataQ_1_File;
extern File dataQ_2_File;
extern File sysLogFile;

//File myFile;
int suBatteryChargeStatus;
void SetupTask(void)      
{
  int taskIdx;
  // Setup the communications link to the console.
  Serial.begin(9600);
  Serial.println("Begin SetupTask.");  

  // Digital I/O SOLAR_REG_ENABLE is connected to Pin 4 (Vdisable
  // input) of the KA278 Solar Voltage Regulator and controls the
  // output of the regulator. A voltage > 1.4V (or open) on Pin4
  // enables the egulator and charges the battery when the solar
  // pannel is sufficiently illuminated..  The Reset/default state
  // of the Digital I/O SOLAR_REG_ENABLE is INPUT and the Regulator
  // output will be enabled to charge the battery. The Reset/default
  // state of the bit controlling the state Digital I/O 
  // SOLAR_REG_ENABLE is low therefore setit HIGH prior to eanabling
  // Digital I/O SOLAR_REG_ENABLE as an OUTPUT.
  digitalWrite(SOLAR_REG_ENABLE, HIGH);
  pinMode(SOLAR_REG_ENABLE, OUTPUT);
//*******************BEGIN DIAGNOSTIC CODE**************************
//*******************BEGIN DIAGNOSTIC CODE**************************
  digitalWrite(SOLAR_REG_ENABLE, LOW);
//********************END DIAGNOSTIC CODE***************************
//********************END DIAGNOSTIC CODE***************************

  // The Digital I/O's, BATTERY_MNGR_PGNOT, BATTERY_MNGR_STAT2
  // and BATTERY_MNGR_STAT1_LBO are all programmed as inputs
  // with PULLUP's and monitor the state of the Battery Manager's
  // (MCP73871) device. Table 5-1 Pg. 25 in MCP73871 data sheet 
  // gives a summary of the MCP73871'z status output during the
  // battery's charge cycle. 
  pinMode(BATTERY_MNGR_PGNOT, INPUT_PULLUP);
  pinMode(BATTERY_MNGR_STAT2, INPUT_PULLUP);
  pinMode(BATTERY_MNGR_STAT1_LBO, INPUT_PULLUP);
  // Read the Battery Manager's charging status bits and form
  // the status word as state in the range of 0 to 7. Note
  // that LOW = 0 and HGIH = 1.
  suBatteryChargeStatus =  digitalRead(BATTERY_MNGR_PGNOT);
  suBatteryChargeStatus |=  (digitalRead(BATTERY_MNGR_PGNOT) << 1);
  suBatteryChargeStatus |=  (digitalRead(BATTERY_MNGR_PGNOT) << 2);

//
//  // The digital I/O RTC_SPI_SELECT GPIO is used as the SPI slave
//  // select signal (SS) for the DS3224 RTC chip. SS is asserted
//  // low. Make sure that it is not selected (negated).
  digitalWrite(RTC_SPI_SELECT, HIGH);
  pinMode(RTC_SPI_SELECT, OUTPUT);
  digitalWrite(RTC_SPI_SELECT, HIGH);

  // The digital I/O SD_SPI_SELECT GPIO is used as the SPI slave
  // select signal (SS) for the SD card. SS is asserted
  // low. Make sure that it is not selected (negated).
  digitalWrite(SD_SPI_SELECT, HIGH);
  pinMode(SD_SPI_SELECT, OUTPUT);
  digitalWrite(SD_SPI_SELECT, HIGH);

  // The digital I/O 
  digitalWrite(SD_PWR_ENABLE, HIGH);
  pinMode(SD_PWR_ENABLE, OUTPUT);
  digitalWrite(SD_PWR_ENABLE, HIGH);

  // Clear all of the task's schedule flags to not scheduled
  // set the entry state to the initial state.
  for(int taskIdx = 0; taskIdx < MAX_TASKS; taskIdx++)
    taskScheduled[taskIdx] = false;
  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[DATA_LOGGER_TASK] = LogDatapoint;
  taskScheduled[DATA_LOGGER_TASK] = false;
  tasksState[DATA_LOGGER_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[DATA_REOPORT_TASK] = ReportData;
  taskScheduled[DATA_REOPORT_TASK] = false;
  tasksState[DATA_REOPORT_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[SYSTEMLOG_TASK] = SystemLogTask;
  taskScheduled[SYSTEMLOG_TASK] = false;
  tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[RTC_TASK] = RtcTask;
  taskScheduled[RTC_TASK] = false;
  tasksState[RTC_TASK] = TASK_INIT_STATE;

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[SDRW_TASK] = SdReadWriteTask;
  taskScheduled[SDRW_TASK] = false;
  tasksState[SDRW_TASK] = TASK_INIT_STATE;
  
    // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[MONITOR_TASK] = MonitorTask;
  taskScheduled[MONITOR_TASK] = false;
  tasksState[MONITOR_TASK] = TASK_INIT_STATE;

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[CONSOLE_TASK] = ConsoleTask;
  taskScheduled[CONSOLE_TASK] = false;
  tasksState[CONSOLE_TASK] = TASK_INIT_STATE;

  // Show the Monitor task that all tasks indicate that the
  // Processor may be put to sleep.
    keepAwakeFlags = 0;
  // Schedule the Monitor task. The Monitor task is always running.
    taskScheduled[MONITOR_TASK] = true;

//*******************BEGIN DIAGNOSTIC CODE**************************
//*******************BEGIN DIAGNOSTIC CODE**************************
  // For testing we temporarly use the ATmega2560's external interrupt
  // to detect console inputs. For production the console will use
  // external interrupt 0 which is on DIGITAL_PIN_2.
  pinMode(DIGITAL_PIN_3, INPUT);
  // Attach interrupt 0 to call AnyIdeConsoleInput() on interrupt.
  attachInterrupt(EXT_INTERRUPT_1, AnyIdeConsoleInput, LOW);
  consoleInput = false;
//********************END DIAGNOSTIC CODE***************************
//********************END DIAGNOSTIC CODE***************************    

  // Initialize the SD card and make sure that the files that we need
  // for DataReporter operation are open.
  // NOTE BENE: When the SD.begin() is called the SPI bus is initialized
  // and the RTC becomes avaliable.
  Serial.print("Initializing SD card...");  
  pinMode(53, OUTPUT);      
  // Initialize the SD card to provide the file system.
  if (!SD.begin(SD_SPI_SELECT))
  {
    //*******************BEGIN DIAGNOSTIC CODE**************************
    //*******************BEGIN DIAGNOSTIC CODE**************************
    // We have to do something here this is a fatal error
    // and the DataReporter will not operate. Maybe we
    // should leave a note in the EEPROM?
    Serial.println("initialization failed!");
    //********************END DIAGNOSTIC CODE***************************
    //********************END DIAGNOSTIC CODE***************************    
  }
  Serial.println("SD card initialization Successful.");  
  
  // Get a date-time string and create this month's history
  // file name. The date-time string is a fixed length string
  // with the following format: "mm/dd/yy hh:mm:ss"
  // Define a string object to hold the history file name.
  String historyFileName("Hist");
  // Define a string object to hold the date-time string.
  String setupDtString;
  setupDtString+=ReadTimeDate(setupDtString);
  Serial.print(" setupDtString = ");
  Serial.println(setupDtString);
   

  // Open the SYS_LOG_FILE ("Syslog.txt").This file
  // holds all of the system log messages.
  sysLogFile = SD.open(SYS_LOG_FILE ,FILE_WRITE);
  // Now write to the sysLogFile.
  if (sysLogFile) 
  {
    Serial.println("Writing to the sysLogFile");
    sysLogFile.print("DataReporter RESET at ");
    sysLogFile.println(setupDtString);
    sysLogFile.close();
  } 
  else 
  {
    //*******************BEGIN DIAGNOSTIC CODE**************************
    //*******************BEGIN DIAGNOSTIC CODE**************************
    // We have to do something here this is a fatal error
    // and the DataReporter will not operate. Maybe we
    // should leave a note in the EEPROM?
    Serial.println("Error opening the sysLogFile");
    //********************END DIAGNOSTIC CODE***************************
    //********************END DIAGNOSTIC CODE***************************    
  }
  // Open up the history file for this month.The history files
  // contain all data points for the current month. If the file
  // for this month does not exist then create it. The files are
  // uniquely named HistMMYY.txt.
   // Build the history file name.
  historyFileName+=setupDtString.substring(SETUP_MONTH_START,SETUP_MONTH_END);
  historyFileName+=setupDtString.substring(SETUP_YEAR_START,SETUP_YEAR_END);
  historyFileName+=".txt";
  Serial.print("historyFileName = ");
  Serial.println(historyFileName);
  // Open the history file for read and write at the end
  // of the file. File name argument must a pointer to
  // character string.
  char histFileNameArray[DATE_TIME_LENGTH+1];
  historyFileName.toCharArray(&histFileNameArray[0], DATE_TIME_LENGTH );
  File historyFile = SD.open(&histFileNameArray[0],FILE_WRITE);
  // Make sure that the history file opened okay.
  if (historyFile) {
    Serial.println("History file opened okay.");
  } 
  else 
  {
    //*******************BEGIN DIAGNOSTIC CODE**************************
    //*******************BEGIN DIAGNOSTIC CODE**************************
    // We have to do something here this is a fatal error
    // and the DataReporter will not operate. Maybe we
    // should leave a note in the EEPROM?
    Serial.println("Error opening the history file");
    //********************END DIAGNOSTIC CODE***************************
    //********************END DIAGNOSTIC CODE***************************    
  }
  // As data points are collected by the DataReporter they are
  // saved in one of two queueing files, DataQ_1.txt and
  // DataQ_2.txt. These two files buffers data points during the
  // time that it takes to successfully transfer theem to the
  // cloud based database. We create these files here and initialize
  // the associated controlling variables here.
  // Open the DATA_QUEUE1_FILE ("DataQ_1.txt").
  dataQ_1_File = SD.open(DATA_QUEUE1_FILE ,FILE_WRITE);
  if (dataQ_1_File) {
    Serial.println("Data queue 1 file opened okay.");
  } 
  else 
  {
    //*******************BEGIN DIAGNOSTIC CODE**************************
    //*******************BEGIN DIAGNOSTIC CODE**************************
    // We have to do something here this is a fatal error
    // and the DataReporter will not operate. Maybe we
    // should leave a note in the EEPROM?
    Serial.println("Error opening the Data queue 1 file");
    //********************END DIAGNOSTIC CODE***************************
    //********************END DIAGNOSTIC CODE***************************    
  }
  // Open the DATA_QUEUE1_FILE ("DataQ_1.txt").
  dataQ_2_File = SD.open(DATA_QUEUE2_FILE ,FILE_WRITE);
  if (dataQ_2_File) {
    Serial.println("Data queue 2 file opened okay.");
  } 
  else 
  {
    //*******************BEGIN DIAGNOSTIC CODE**************************
    //*******************BEGIN DIAGNOSTIC CODE**************************
    // We have to do something here this is a fatal error
    // and the DataReporter will not operate. Maybe we
    // should leave a note in the EEPROM?
    Serial.println("Error opening the Data queue 2 file");
    //********************END DIAGNOSTIC CODE***************************
    //********************END DIAGNOSTIC CODE***************************    
  }
  // Initialize the data point queueing control variables.
  dataQueue1Busy = true;
  dataQueue1Count = 0;
  dataQueue2Count = 0;
  Serial.println("End of Setup.");  
  delay(100);


}

