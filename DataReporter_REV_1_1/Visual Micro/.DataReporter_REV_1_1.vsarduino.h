#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Mega Pro 2560V 3.3V
#define __AVR_ATmega2560__
#define 
#define _VMDEBUG 1
#define ARDUINO 105
#define ARDUINO_MAIN
#define __AVR__
#define F_CPU 8000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

//
//
void ConsoleTask(void);
void AnyIdeConsoleInput();
void TestSdOperation();
void MakeAnalogMeasurements();
void consoleExit();
void LogDatapoint(void);
void MonitorTask(void);
void BucketTip();
void monitorSetTimeout( unsigned long t);
boolean monitorTimeout();
void ReportData(void);
void PushRtc(struct rtcControl *controlPtr);
struct rtcControl *PopRtc();
void PushSysLog(struct sysLogControl *controlPtr);
struct sysLogControl *PopSysLog();
void PushSdrw(struct sdrwControl *controlPtr);
struct sdrwControl *PopSdrw();
int PushDataLog(struct dataLogControl *controlPtr);
int PopDataLog();
void RtcTask(void);
int Rtc_Init();
String ReadTimeDate(String dateTimeString);
void SetRtcDateTime(String dateTimeString);
void   saveRtcSpi_SPCR(void);
void   restoreRtcSpi_SPCR(void);
void SdReadWriteTask(void);
void SetupTask(void);
void SystemLogTask(void);
void TaskTemTask(void);

#include "C:\Users\Bob\Documents\Arduino\hardware\mega-pro-3.3V\variants\mega\pins_arduino.h" 
#include "C:\Users\Bob\Documents\Arduino\hardware\mega-pro-3.3V\cores\arduino\arduino.h"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\DataReporter_REV_1_1.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Console.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Definitions.h"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Global.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\LogData.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Monitor.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\ReportData.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\ResourceQueues.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Rtc.h"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Rtc.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\SdReadWrite.h"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\SdReadWrite.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\Setup.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\SysLog.h"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\SystemLogTask.ino"
#include "C:\Users\Bob\Documents\GitHub\LPDR\DataReporter_REV_1_1\TaskTemplate.ino"
#endif
