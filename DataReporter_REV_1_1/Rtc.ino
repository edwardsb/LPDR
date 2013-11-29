//
// Task description...
//
#include <SPI.h>
#include "Definitions.h"
#include "Rtc.h"

//
// Local definitions go here.
//
  #define RTC_STATE_1    1
  #define RTC_STATE_2    2
//
// Local globals go here.
//
  rtcControl *rtcCurrentReqtPtr;
  String rtcTempString;

// Enter here only from the exec.
void RtcTask(void)      
{
  switch(tasksState[RTC_TASK])
  {
    case TASK_INIT_STATE:

    // We are here for because there may be a request for an 
    // RTC operation.
    // Check for and RTC requets in the RTC's FIFO.
    rtcCurrentReqtPtr = PopRtc();
    //Serial.print("RtcTask() case: TASK_INIT_STATE rtcCurrentReqtPtr = ");
    //Serial.println((unsigned long)rtcCurrentReqtPtr, HEX);
    if( rtcCurrentReqtPtr != (rtcControl*)QUEUE_EMPTY)
    {
      // We have a request for an RTC operation.
      switch(rtcCurrentReqtPtr->op)
      {
        case RTC_SET_DATETIME :
          // Call the SetRtcDateTime() function with a pointer to
          // the string containing the new date-time fields.
          SetRtcDateTime( *rtcCurrentReqtPtr->data.rtcSetString );
          // Let caller know that the requested RTC operation is 
          // complete.
          rtcCurrentReqtPtr->stat = SUCCESS;
          // Then continue in this state to see if there are any
          // other RTC operation requests in the FIFO.
        break;
        case RTC_READ_DATE_TIME:
         // Here to read the RTC, convert the RTC's date-time
         // register data to ASCII and place the ASCII date-time
         // string into the caller's buffer. Clear our temp
         // buffer first.
          rtcTempString = "";        
          rtcTempString+=ReadTimeDate(rtcTempString);
          //Serial.print("RtcTask() Date-Time = ");
          //Serial.println(rtcTempString);          
         // Now move the date-time string to the lient's
         // control structure.
         *rtcCurrentReqtPtr->data.rtcSetString = rtcTempString;
         rtcCurrentReqtPtr->stat = SUCCESS;         
        break;
        
        case RTC_DEC_SECONDS:
        
        break;
        case RTC_INC_SECONDS:
        
        break;
        
        default:
        
        break;
      }      
    }
    else
    {
      // There are no more RTC operation requests in the
      // queue unschedule this state machine.
      tasksState[RTC_TASK] = TASK_INIT_STATE;
      taskScheduled[RTC_TASK] = false;
    }
    break;
    
    case RTC_STATE_1:
    
    break;
    
    case RTC_STATE_2:
    
    break;
    
    default:
      //*******************BEGIN DIAGNOSTIC CODE**************************
      //*******************BEGIN DIAGNOSTIC CODE**************************
      // We have to do something here this is a fatal error
      // and the DataReporter will not operate. Maybe we
      // should leave a note in the EEPROM?
      Serial.println("Error opening the sysLogFile");
      //********************END DIAGNOSTIC CODE***************************
      //********************END DIAGNOSTIC CODE***************************    
    break;
  }
  
}

byte spiRtcSpcr;
int Rtc_Init()
{ 
  // Select the RTC as the SPI slave.
  pinMode(RTC_SPI_SELECT,OUTPUT); // chip select
  digitalWrite(RTC_SPI_SELECT, HIGH);
  // start the SPI library:
//Serial.print("Before SPI.begin() SPCR = ");  
//Serial.println(SPCR, HEX);
  SPI.begin();  
//Serial.print("After SPI.begin() SPCR = ");  
//Serial.println(SPCR, HEX); 
  // Set the bit order used by the RTC.
  SPI.setBitOrder(MSBFIRST);   
//Serial.print("After setBitOrder() SPCR = ");  
//Serial.println(SPCR, HEX); 
  // Set the data transfer mode used by the RTC.
  // i.e. Clock polarity and clock phase.
  SPI.setDataMode(SPI_MODE1); // both mode 1 & 3 should work.  
//Serial.print("After setDataMode() SPCR = "); 
//Serial.println(SPCR, HEX); 
  // Set the clock at 4 mHz, i.e. divide the CPU
  // clock (8mHz) by 2.
  SPI.setClockDivider(SPI_CLOCK_DIV2);
 //  Make sure that all of the DS3234 control register bits
  // are in the correct state. Select the RTC as the slave.
  digitalWrite(RTC_SPI_SELECT, LOW);
  // Address the RTC Contgrol register.
  SPI.transfer( SPI_WRITE || RTC_CONTROL_REGISTER);
  // Start the clock oscillator !EOSC = 0
  // Battery-Backed square wave out disabeled. BBSQW = 0
  // Force a temperature compensation cycle. CONV = 1
  // Square wave output 1 Hz. Not used. RS2 = Rs1 = 0
  // Interrupt Control. No interrupts. Square wave output enabled (not used).
  // Alarms disabeled. A2IE = A1IE = 0.
  // Write the RTC Control register.
  SPI.transfer((1<<RTC_BBSQW) | (1<<RTC_CONV));
  // Deselect the RTC.
  digitalWrite(RTC_SPI_SELECT, HIGH);
   
  delay(10);
}

String ReadTimeDate(String dateTimeString)
{
  saveRtcSpi_SPCR();
  SPI.setDataMode(SPI_MODE1);
  int TimeDate [7]; //second,minute,hour,null,day,month,year

  //rtcString = dateTimeString;
  // Read the date/time registers, RTC addresses 0x00 --> 0x06   
  for(int timeDateAddr=0; timeDateAddr<=6; timeDateAddr++)
  {
    if(timeDateAddr==RTC_DAY_ADDR)
      timeDateAddr++;
    digitalWrite(RTC_SPI_SELECT, LOW);
    SPI.transfer(timeDateAddr+0x00); 
    
//    Serial.print("RTI Addr = ");
//    Serial.print(timeDateAddr,HEX);
    
    unsigned int data = SPI.transfer(0x00);   
    
//    Serial.print("  Data = 0x");
//    Serial.println(data,HEX);
    
     digitalWrite(RTC_SPI_SELECT, HIGH);
    int dataLs4Bits = data & B00001111;    
    if(timeDateAddr==RTC_HOUR_ADDR)
    {	
      int msdData=(data & B00110000)>>4; //24 hour mode
      if(msdData==B00000010)
        msdData=20;        
      else if(msdData==B00000001)
        msdData=10;
      TimeDate[timeDateAddr]=dataLs4Bits+msdData;
    }
    else if(timeDateAddr==RTC_DATE_ADDR)
    {
      int msdData=(data & B00110000)>>4;
      TimeDate[timeDateAddr]=dataLs4Bits+msdData*10;
    }
    else if(timeDateAddr==RTC_MONTH_ADDR)
    {
      int msdData=(data & B00010000)>>4;
      TimeDate[timeDateAddr]=dataLs4Bits+msdData*10;
    }
    else if(timeDateAddr==RTC_YEAR_ADDR)
    {
      int msdData=(data & B11110000)>>4;
      TimeDate[timeDateAddr]=dataLs4Bits+msdData*10;
    }
    else
    {	
      int msdData=(data & B01110000)>>4;
      TimeDate[timeDateAddr]=dataLs4Bits+msdData*10;	
    }
  }  // End of for(int timeDateAddr=0; timeDateAddr<=6;timeDateAddr++)
  #define DATE_TIME_MONTH          5
  #define DATE_TIME_DATE           4
  #define DATE_TIME_YEAR           6
  #define DATE_TIME_HOUR           2
  #define DATE_TIME_MIN            1
  #define DATE_TIME_SEC            0 
//    for(int timeDateAddr=0; timeDateAddr<=6; timeDateAddr++)
//    {
//      Serial.print("Byte ");
//      Serial.print(timeDateAddr, DEC);
//      Serial.print(" = ");
//      Serial.println(TimeDate[timeDateAddr], HEX);  
//    }

  if(TimeDate[DATE_TIME_MONTH] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_MONTH]);
  dateTimeString.concat("/") ;
  if(TimeDate[DATE_TIME_DATE] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_DATE]);
  dateTimeString.concat("/") ;
  if(TimeDate[DATE_TIME_YEAR] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_YEAR]);
  dateTimeString.concat(" ") ;
  if(TimeDate[DATE_TIME_HOUR] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_HOUR]);
  dateTimeString.concat(":") ;
  if(TimeDate[DATE_TIME_MIN] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_MIN]);
  dateTimeString.concat(":") ;
  if(TimeDate[DATE_TIME_SEC] < 10)
    dateTimeString+=0;
  dateTimeString.concat(TimeDate[DATE_TIME_SEC]);
  restoreRtcSpi_SPCR();
  return(dateTimeString);
}


void SetRtcDateTime(String dateTimeString)
{
  // the rtcMemInMap[] array maps the RTC memory addresses
  // 0 --> 6 to the to the op input field index 0 --> 5
  // in the rtcSetString string object.
  byte rtcMemInMap[7] = {10,8,6,0,2,0,4};
  // stringIdx is used to pick the date-time value substrings
  // out of the rtcSetString string
  int substringIdx;
  // rtcMemAddr indicates the current RTC memory location.
  int rtcMemAddr;
  // rtcMemVal  is the data to be stored into the RTC memory
  // location.
  int rtcMemVal;
  // Most and least significant nibble of RTC memory location
  int msn, lsn;  //Most and least significant nibble of RTC.
  // Save the current RTC operating mode.
  saveRtcSpi_SPCR();
  SPI.setDataMode(SPI_MODE1);
  // Go through each of the 7 RTC memory locations rtcMemAddr
  // and use the substring field, indicated by rtcMemInMap[],
  // and use that field to set the RTC memory data.
  for(rtcMemAddr = 0; rtcMemAddr <= 6; rtcMemAddr++)
  {
    if(rtcMemAddr == RTC_DAY_ADDR)
      rtcMemAddr++;  // Go past day-of-week.
    // Get the substring indicated by rtcMemInMap[rtcMemAddr] 
    // and then convert that substring to integer in rtcMemVal. 
    substringIdx = rtcMemInMap[rtcMemAddr];
    rtcMemVal = (int)(rtcSetString.substring(substringIdx, substringIdx+2).toInt());
//    Serial.print("substringIdx = ");
//    Serial.print(substringIdx, DEC);
//    Serial.print("  rtcMemAddr = ");
//    Serial.print(rtcMemAddr, DEC);
//    Serial.print("  substring = ");
//    Serial.print(rtcSetString.substring(substringIdx, substringIdx+2));
//    Serial.print("  rtcMemVal = ");
//    Serial.println(rtcMemVal, DEC);    
    msn = rtcMemVal / 10;
    lsn = rtcMemVal - msn * 10;
    // If this is the hour then the format may be 12 hour AM/PM
    // military 24 hour time. They are the same for AM. If it is
    // PM then the msn may be:
    // msn = 1   then AM/PM
    // msn = 2   then 24 military and bit 5 (= 20) hours must be
    //           set to 1 making the msn = 20 hours.
    if(rtcMemAddr == RTC_HOUR_ADDR)
    {
      if (msn == 2)
	msn = B00000010;
      else if (msn == 1)
	msn = B00000001;
    }	
    rtcMemVal= lsn + (msn << 4);
    // Now write the RTC's rtcMemAddr memory location.	  
    digitalWrite(RTC_SPI_SELECT, LOW);  // RTC slave select
    SPI.transfer(rtcMemAddr+0x80);      // Address with write bit set.
    SPI.transfer(rtcMemVal);            // Data.        
    digitalWrite(RTC_SPI_SELECT, HIGH); // RTC slave de-select.
  }
  restoreRtcSpi_SPCR();
}


void   saveRtcSpi_SPCR(void)
{
  spiRtcSpcr = SPCR;
}

void   restoreRtcSpi_SPCR(void)
{
  SPCR = spiRtcSpcr;
}
