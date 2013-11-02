//
//
//

#include <SPI.h>

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

#define RTC_SEC_ADDR        0
#define RTC_MIN_ADDR        1
#define RTC_HOUR_ADDR       2
#define RTC_DAY_ADDR        3
#define RTC_DATE_ADDR       4
#define RTC_MONTH_ADDR      5
#define RTC_YEAR_ADDR       6


int SetTimeDate(int d, int mo, int y, int h, int mi, int s)
{ 
  int TimeDate [7]={s, mi, h, 0, d, mo, y};
  saveRtcSpi_SPCR();
  SPI.setDataMode(SPI_MODE1);
  // Convert the date-time values from the input parameters to
  // BCD, position, insert  into date-time bytes and write them
  //to the RTC date and time registers.
  for(int timeDateAddr=0; timeDateAddr<=6; timeDateAddr++)
  {
//    Serial.print("RTI Addr = ");
//    Serial.print(timeDateAddr,DEC);
//    Serial.print("  TimeDate[] = ");
//    Serial.println(TimeDate[timeDateAddr],DEC);
    // Go by the day-of-the-week we don't use it.
    if(timeDateAddr==RTC_DAY_ADDR )
      timeDateAddr++;
    int b= TimeDate[timeDateAddr]/10;
    int a= TimeDate[timeDateAddr]-b*10;
    if(timeDateAddr==RTC_HOUR_ADDR)
    {
      if (b==2)
	b=B00000010;
      else if (b==1)
	b=B00000001;
    }	
    TimeDate[timeDateAddr]= a+(b<<4);		  
    digitalWrite(RTC_SPI_SELECT, LOW);
    SPI.transfer(timeDateAddr+0x80); 
    SPI.transfer(TimeDate[timeDateAddr]);        
    digitalWrite(RTC_SPI_SELECT, HIGH);
    restoreRtcSpi_SPCR();
  }
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

void   saveRtcSpi_SPCR(void)
{
  spiRtcSpcr = SPCR;
}

void   restoreRtcSpi_SPCR(void)
{
  SPCR = spiRtcSpcr;
}
