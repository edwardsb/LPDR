//
// This task contains all hardware and software initialization 
// tasks required prior to starting the executive.
//
#include "Definitions.h"

File myFile;

void SetupTask(void)      
{
  int taskIdx;
  // Setup the communications link to the console.
  Serial.begin(9600);
  Serial.println("In DrSetupTask 1.");  

  // Digital I/O SOLAR_REG_ENABLE controls the output of the Solar Regulator.
  // The default state of the digital I/O pins is input and the
  // state of the controlling register is 0.
  digitalWrite(SOLAR_REG_ENABLE, LOW);
  pinMode(SOLAR_REG_ENABLE, OUTPUT);
  // Now turn on the Solar Regulator.
  digitalWrite(SOLAR_REG_ENABLE,HIGH);

  // The Digital I/O BATTERY_CHARGE_DISABLE is used to enable and 
  // disable the Semtech SC819 Li-ion battery charger. HIGH
  // disables and LOW eables. The default state at turn-on
  // should be disabeled.
  digitalWrite(BATTERY_CHARGE_DISABLE, HIGH);
  pinMode(BATTERY_CHARGE_DISABLE, OUTPUT);
  digitalWrite(BATTERY_CHARGE_DISABLE, HIGH); 


  // The digital I/O RTC_SPI_SELECT GPIO is used as the SPI slave
  // select signal (SS) for the DS3224 RTC chip. SS is asserted
  // low. Make sure that it is not selected (negated).
  digitalWrite(RTC_SPI_SELECT, HIGH);
  pinMode(RTC_SPI_SELECT, OUTPUT);
  digitalWrite(RTC_SPI_SELECT, HIGH);

  // The digital I/O SD_SPI_SELECT GPIO is used as the SPI slave
  // select signal (SS) for the SD card. SS is asserted
  // low. Make sure that it is not selected (negated).
  digitalWrite(SD_SPI_SELECT, HIGH);
  pinMode(SD_SPI_SELECT, OUTPUT);
  digitalWrite(SD_SPI_SELECT, HIGH);

  // Clear all of the task's schedule flags to not scheduled
  // set the entry state to the initial state.
  for(int taskIdx = 0; taskIdx < MAX_TASKS; taskIdx++)
    taskScheduled[taskIdx] = false;
  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[DATA_LOGGER_TASK] = LogDatapoint;
  taskScheduled[DATA_LOGGER_TASK] = true;
  tasksState[DATA_LOGGER_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[DATA_REOPORT_TASK] = ReportData;
  taskScheduled[DATA_REOPORT_TASK] = true;
  tasksState[DATA_REOPORT_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[SYSTEMLOG_TASK] = SystemLogTask;
  taskScheduled[SYSTEMLOG_TASK] = true;
  tasksState[SYSTEMLOG_TASK] = TASK_INIT_STATE;   

  // If you add a new task initialize the following
  // global variab les for it here.
  taskPointers[MONITOR_TASK] = MonitorTask;
  taskScheduled[MONITOR_TASK] = true;
  tasksState[MONITOR_TASK] = TASK_INIT_STATE;

  Serial.println("In DrSetupTask 2.");  


  //Note that the DS3234 RTC and the SD card both communicate
  // via the SPI bus. The SD card initialization apparently
  // includes an ATmega 2560 SPI bus control register 
  // that is compatible with the DS3234 tnd the Rtc_Init() is
  // not required in fact it causes problems with the SD card
  // operation.
  //Rtc_Init();

  Serial.print("Initializing SD card...");
  
   pinMode(53, OUTPUT);
   
   
  // Initialize the SD card to provide the file system.
  if (!SD.begin(6))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("SD card initialization done.");
  // To set the RTC date and time: 
  //  1.  Uncomment the following line.
  //  2.  Run the program.
  //  3.  Comment the line.
  // date(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)
  //SetTimeDate(4,9,13,1,7,30);

  // Open up the history file for this month.The history files
  // contain all data points for the current month. If the file
  // for this month does not exist then create it. The files are
  // uniquely named HistMMYY.txt.
  //
  // Get a date-time string to create the history file. The 
  // date-time string is a fixed length string with the 
  // following format: "mm/dd/yy hh:mm:ss"
#define SETUP_MONTH_START      0
#define SETUP_MONTH_END      2
#define SETUP_YEAR_START       6
#define SETUP_YEAR_END       8
  // Define a string object to hold the history file name.
  String historyFileName("Hist");
  // Define a string object to hold the date-time string.
  String setupDtString;
  // Build the file name.
  setupDtString+=ReadTimeDate(setupDtString);
  historyFileName+=setupDtString.substring(SETUP_MONTH_START,SETUP_MONTH_END);
  historyFileName+=setupDtString.substring(SETUP_YEAR_START,SETUP_YEAR_END);
  historyFileName+=".txt";
  Serial.print("historyFileName = ");
  Serial.println(historyFileName);
  // Open the history file for read and writ at the end
  // of the file. File name argument must a pointer to
  // character string.
  char histFileNameArray[DATE_TIME_LENGTH+1];
  historyFileName.toCharArray(&histFileNameArray[0], DATE_TIME_LENGTH );
  File historyFile = SD.open(&histFileNameArray[0],FILE_WRITE);

  // Now write to the history file.
  if (historyFile) {
    Serial.println("Writing to the historyFile");
    historyFile.println("History 1, 2, 3.");
    //historyFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("Error opening the history file");
  }

  // Open the DATA_HOLDING_FILE ("DataHold.txt"). This file
  // holds all datapoints collected but not written to the
  // database yet.
    File holdingFile = SD.open(DATA_HOLDING_FILE ,FILE_WRITE);
  // Now write to the holding file.
  if (holdingFile) {
    Serial.println("Writing to the holdingFile");
    holdingFile.println("Holding 1, 2, 3.");
  } 
  else {
    // If the file didn't open, print an error:
    Serial.println("Error opening the holding file");
  }

  // Open the SYS_LOG_FILE ("Syslog.txt").This file
  // holds all of the system log messages.
    File sysLogFile = SD.open(SYS_LOG_FILE ,FILE_WRITE);
  // Now write to the sysLogFile.
  if (sysLogFile) {
    Serial.println("Writing to the sysLogFile");
    holdingFile.println("sysLogFile 1, 2, 3.");
  } 
  else {
    // If the file didn't open, print an error:
    Serial.println("Error opening the sysLogFile");
  }


  //  
  //  
  ////      
  ////
  ////
  //
  //  // open the file. note that only one file can be open at a time,
  //  // so you have to close this one before opening another.
  //  myFile = SD.open("test.txt", FILE_WRITE);
  //  
  //  // if the file opened okay, write to it:
  //  if (myFile) {
  //    Serial.print("Writing to test.txt...");
  //    myFile.println("testing 1, 2, 3.");
  //	// close the file:
  //    myFile.close();
  //    Serial.println("Done writting to the test file second time.");
  //  } else {
  //    // if the file didn't open, print an error:
  //    Serial.println("error opening test.txt");
  //  }
  //
  //  // re-open the file for reading:
  //  myFile = SD.open("test.txt");
  //  if (myFile) {
  //    Serial.println("test.txt:");
  //    
  //    // read from the file until there's nothing else in it:
  //    while (myFile.available()) {
  //    	Serial.write(myFile.read());
  //    SD.remove("test.txt");
  //    }
  //    // close the file:
  //    myFile.close();
  //  } else {
  //  	// if the file didn't open, print an error:
  //    Serial.println("error opening test.txt");
  //  }



}

