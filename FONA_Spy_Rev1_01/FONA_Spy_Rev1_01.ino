//
// The purpose of this sketch is to is to add the following
// control and monitoring signals to Adafruit's FONA cellular
// module to the test sketch described at:
//  https://learn.adafruit.com/adafruit-fona-mini-gsm-gprs-cellular-phone-module/arduino-test
//  1.  Key - This is also a super important pin (but not as important as Vio). 
//      This is the power on/off indicator. Its also tied to the button in the 
//      top left. Tie this pin to ground for 2 seconds to turn the module on or
//      off. It's not a level signal so it isn't like "low is off, high is on"
//      - instead you must pulse it for 2 seconds to turn off/on. The module 
//      comes by default off. Tie this permanently to ground if you never want 
//      your micro to turn off the FONA for power saving. This signal is tied to
//      DIO8
#define FONA_KEY_NOT 8
//  2.  PS - this is the Power Status pin. It is low when the module is off and
//      high when the module has power. If you're using the Key button or pin, 
//      you can monitor this pad to see when the module's booted up. This is 
//      tied to the Pwr LED too. This signal is tied to DIO9
#define FONA_PS 9
//  3.  NS - this is the Network Status pin. It pulses to signal the current status
//      of the module. This is also tied to the Net LED.
#define FONA_NS 11
//  4.  Reset - this is module hard reset pin. By default it has a high pull-up
//      (module not in reset). If you absolutely got the module in a bad space, 
//      toggle this pin low for 100ms to perform a hard reset. This signal is 
//      tied to DIO10
#define FONA_RESET_NOT 10
//
// The following commands were added:
//  [~]  Power ON-OFF
//  [#]  Reset the FONA module.
//  {$}  Print the states of the PS and NS FONA status lines.
//  [&]  Call the Adafruit_FONA object's begin method.

/*************************************************** 
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA 
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/* 
THIS CODE IS STILL IN PROGRESS!

Open up the serial console on the Arduino at 115200 baud to interact with FONA
*/

#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(&fonaSS, FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

// the fonaObjectInitialized is set when the Adafruit_FONA object's
// begin function if called.
boolean fonaObjectInitialized = false;
//
//
//
#define MAX_UNSIGNED_LONG 0xFFFFFFFF
unsigned long elapsedTime;
unsigned long cnslTimeoutPeriod;
unsigned long cnslPreviousMillis;



void setup() {
  //
  // Setup and initialize the FONA's KEY signal. At UNO turn on or
  // reset DIO8 is 0 and the output line is in the tri state (open).
  // Set the DIO8 state to be high and then set it to be an output.
  digitalWrite(FONA_KEY_NOT, HIGH);
  pinMode(FONA_KEY_NOT, OUTPUT); 
  // Setup and initialize the FONA's Reset signal. At UNO turn on or
  // reset DIO11 is 0 and the output line is in the tri state (open).
  // Set the DIO11 state to be high and then set it to be an output.
  digitalWrite(FONA_RESET_NOT, HIGH);
  pinMode(FONA_RESET_NOT, OUTPUT); 
  // Set the DIO8 state to be high and then set it to be an output.
  // Set the Network Status and Power Status DIO's as inputs.
  pinMode(FONA_NS, INPUT);  
  pinMode(FONA_PS, INPUT);
  
  Serial.begin(115200);
  Serial.println(F("FONA MODIFIED basic test"));
//  Serial.println(F("Initializing....(May take 3 seconds)"));

  
//  // See if the FONA is responding
//  if (! fona.begin(4800)) {  // make it slow so its easy to read!
//    Serial.println(F("Couldn't find FONA"));
//    while (1);
//  }
//  Serial.println(F("FONA is OK"));

  printMenu();
}

void printMenu(void) {
   Serial.println(F("-------------------------------------"));
   Serial.println(F("[?] Print this menu"));
   Serial.println(F("[a] read the ADC (2.8V max)"));
   Serial.println(F("[b] read the Battery V"));
   Serial.println(F("[C] read the SIM CCID"));
   Serial.println(F("[U] Unlock SIM with PIN code"));
   Serial.println(F("[i] read RSSI"));
   Serial.println(F("[n] get Network status"));
   Serial.println(F("[v] set audio Volume"));
   Serial.println(F("[V] get Volume"));
   Serial.println(F("[H] set Headphone audio"));
   Serial.println(F("[e] set External audio"));
   Serial.println(F("[T] play audio Tone"));
   Serial.println(F("[f] tune FM radio"));
   Serial.println(F("[F] turn off FM"));
   Serial.println(F("[m] set FM volume"));
   Serial.println(F("[M] get FM volume"));
   Serial.println(F("[q] get FM station signal level"));
   Serial.println(F("[P] PWM/Buzzer out"));
   Serial.println(F("[c] make phone Call"));
   Serial.println(F("[h] Hang up phone"));
   Serial.println(F("[N] Number of SMSs"));
   Serial.println(F("[r] Read SMS #"));
   Serial.println(F("[R] Read All SMS"));
   Serial.println(F("[d] Delete SMS #"));
   Serial.println(F("[s] Send SMS"));
   Serial.println(F("[G] Enable GPRS"));
   Serial.println(F("[g] Disable GPRS"));
   Serial.println(F("[l] Query GSMLOC (GPRS)"));
   Serial.println(F("[w] Read webpage (GPRS)"));
   Serial.println(F("[S] create Serial passthru tunnel"));
   // Added commands.
   Serial.println(F("[~] Toggle SIM 800's Power On and Off"));
   Serial.println(F("[#] Reset the FONA Module"));
   Serial.println(F("[$] Read SIM 800's Power and Network Status Signals."));
   Serial.println(F("[&] Call AdafruitFONA Object's begin() Method."));
   Serial.println(F("-------------------------------------"));
   Serial.println(F(""));
  
}
void loop() {
  Serial.print(F("FONA> "));
  while (! Serial.available() );
  
  char command = Serial.read();
  Serial.println(command);
  
  
  switch (command) {
    case '?': {
      printMenu();
      break;
    }
    
    case 'a': {
      // read the ADC
      uint16_t adc;
      if (! fona.getADCVoltage(&adc)) {
        Serial.println(F("Failed to read ADC"));
      } else {
        Serial.print(F("ADC = ")); 
        Serial.print(adc); 
        Serial.println(F(" mV"));
      }
      break;
    }
    
    case 'b': {
        // read the battery voltage
        uint16_t vbat;
        if (! fona.getBattVoltage(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); 
          Serial.print(vbat); 
          Serial.println(F(" mV"));
        }
        break;
    }

    case 'U': {
        // Unlock the SIM with a PIN code
        char PIN[5];
        flushSerial();
        Serial.println(F("Enter 4-digit PIN"));
        readline(PIN, 3);
        Serial.println(PIN);
        Serial.print(F("Unlocking SIM card: "));
        if (! fona.unlockSIM(PIN)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }        
        break;
    }

    case 'C': {
        // read the CCID
        fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
        Serial.print(F("SIM CCID = ")); 
        Serial.println(replybuffer);
        break;
    }

    case 'i': {
        // read the RSSI
        uint8_t n = fona.getRSSI();
        int8_t r;
        
        Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
        if (n == 0) r = -115;
        if (n == 1) r = -111;
        if (n == 31) r = -52;
        if ((n >= 2) && (n <= 30)) {
          r = map(n, 2, 30, -110, -54);
        }
        Serial.print(r); Serial.println(F(" dBm"));
       
        break;
    }
    
    case 'n': {
        // read the network/cellular status
        uint8_t n = fona.getNetworkStatus();
        Serial.print(F("Network status ")); 
        Serial.print(n);
        Serial.print(F(": "));
        if (n == 0) Serial.println(F("Not registered"));
        if (n == 1) Serial.println(F("Registered (home)"));
        if (n == 2) Serial.println(F("Not registered (searching)"));
        if (n == 3) Serial.println(F("Denied"));
        if (n == 4) Serial.println(F("Unknown"));
        if (n == 5) Serial.println(F("Registered roaming"));
        break;
    }
    
    /*** Audio ***/
    case 'v': {
      // set volume
      flushSerial();
      Serial.print(F("Set Vol %"));
      uint8_t vol = readnumber();
      Serial.println();
      if (! fona.setVolume(vol)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;
    }

    case 'V': {
      uint8_t v = fona.getVolume();
      Serial.print(v); Serial.println("%");
    
      break; 
    }
    
    case 'H': {
      // Set Headphone output
      if (! fona.setAudio(FONA_HEADSETAUDIO)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      fona.setMicVolume(FONA_HEADSETAUDIO, 15);
      break;
    }
    case 'e': {
      // Set External output
      if (! fona.setAudio(FONA_EXTAUDIO)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }

      fona.setMicVolume(FONA_EXTAUDIO, 10);
      break;
    }

    case 'T': {
      // play tone
      flushSerial();
      Serial.print(F("Play tone #"));
      uint8_t kittone = readnumber();
      Serial.println();
      // play for 1 second (1000 ms)
      if (! fona.playToolkitTone(kittone, 1000)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;
    }
    
    /*** FM Radio ***/
    
    case 'f': {
      // get freq
      flushSerial();
      Serial.print(F("FM Freq (eg 1011 == 101.1 MHz): "));
      uint16_t station = readnumber();
      Serial.println();
      // FM radio ON using headset
      if (fona.FMradio(true, FONA_HEADSETAUDIO)) {
        Serial.println(F("Opened"));
      }
     if (! fona.tuneFMradio(station)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Tuned"));
      }
      break;
    }
    case 'F': {
      // FM radio off
      if (! fona.FMradio(false)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;
    }
    case 'm': {
      // Set FM volume.
      flushSerial();
      Serial.print(F("Set FM Vol [0-6]:"));
      uint8_t vol = readnumber();
      Serial.println();
      if (!fona.setFMVolume(vol)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;
    }
    case 'M': {
      // Get FM volume.
      uint8_t fmvol = fona.getFMVolume();
      if (fmvol < 0) {
        Serial.println(F("Failed"));
      } else {
        Serial.print(F("FM volume: "));
        Serial.println(fmvol, DEC);
      }
      break;
    }
    case 'q': {
      // Get FM station signal level (in decibels).
      flushSerial();
      Serial.print(F("FM Freq (eg 1011 == 101.1 MHz): "));
      uint16_t station = readnumber();
      Serial.println();
      int8_t level = fona.getFMSignalLevel(station);
      if (level < 0) {
        Serial.println(F("Failed! Make sure FM radio is on (tuned to station)."));
      } else {
        Serial.print(F("Signal level (dB): "));
        Serial.println(level, DEC);
      }
      break;
    }
    
    /*** PWM ***/
    
    case 'P': {
      // PWM Buzzer output @ 2KHz max
      flushSerial();
      Serial.print(F("PWM Freq, 0 = Off, (1-2000): "));
      uint16_t freq= readnumber();
      Serial.println();
      if (! fona.PWM(freq)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;
    }

    /*** Call ***/
    case 'c': {      
      // call a phone!
      char number[30];
      flushSerial();
      Serial.print(F("Call #"));
      readline(number, 30);
      Serial.println();
      Serial.print(F("Calling ")); Serial.println(number);
      if (!fona.callPhone(number)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
      }
      
      break;
    }
    case 'h': {
       // hang up! 
      if (! fona.hangUp()) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      break;     
    }
    
    /*** SMS ***/
    
    case 'N': {
        // read the number of SMS's!
        int8_t smsnum = fona.getNumSMS();
        if (smsnum < 0) {
          Serial.println(F("Could not read # SMS"));
        } else {
          Serial.print(smsnum); 
          Serial.println(F(" SMS's on SIM card!"));
        }
        break;
    }
    case 'r': {
      // read an SMS
      flushSerial();
      Serial.print(F("Read #"));
      uint8_t smsn = readnumber();
      
      Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
      uint16_t smslen;
      if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
        Serial.println("Failed!");
        break;
      }
      Serial.print(F("***** SMS #")); Serial.print(smsn); 
      Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
      Serial.println(replybuffer);
      Serial.println(F("*****"));
      
      break;
    }
    case 'R': {
      // read all SMS
      int8_t smsnum = fona.getNumSMS();
      uint16_t smslen;
      for (int8_t smsn=1; smsn<=smsnum; smsn++) {
        Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
        if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
           Serial.println(F("Failed!"));
           break;
        }
        // if the length is zero, its a special case where the index number is higher
        // so increase the max we'll look at!
        if (smslen == 0) {
          Serial.println(F("[empty slot]"));
          smsnum++;
          continue;
        }
        
        Serial.print(F("***** SMS #")); Serial.print(smsn); 
        Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
        Serial.println(replybuffer);
        Serial.println(F("*****"));
      }
      break;
    }

    case 'd': {
      // delete an SMS
      flushSerial();
      Serial.print(F("Delete #"));
      uint8_t smsn = readnumber();
      
      Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
      if (fona.deleteSMS(smsn)) {
        Serial.println(F("OK!"));
      } else {
        Serial.println(F("Couldn't delete"));
      }
      break;
    }
    
    case 's': {
      // send an SMS!
      char sendto[21], message[141];
      flushSerial();
      Serial.print(F("Send to #"));
      readline(sendto, 20);
      Serial.println(sendto);
      Serial.print(F("Type out one-line message (140 char): "));
      readline(message, 140);
      Serial.println(message);
      if (!fona.sendSMS(sendto, message)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
      }
      
      break;
    }
    /*********************************** GPRS */
    
    case 'g': {
       // turn GPRS off
       if (!fona.enableGPRS(false))  
         Serial.println(F("Failed to turn off"));
       break;
    }
    case 'G': {
       // turn GPRS on
       if (!fona.enableGPRS(true))  
         Serial.println(F("Failed to turn on"));
       break;
    }
    case 'l': {
       // check for GSMLOC (requires GPRS)
       uint16_t returncode;
       
       if (!fona.getGSMLoc(&returncode, replybuffer, 250))
         Serial.println(F("Failed!"));
       if (returncode == 0) {
         Serial.println(replybuffer);
       } else {
         Serial.print(F("Fail code #")); Serial.println(returncode);
       }
       
       break;
    }
    case 'w': {
      // read website URL
      uint16_t statuscode;
      int16_t length;
      char url[80];
      
      flushSerial();
      Serial.println(F("NOTE: in beta! Use small webpages to read!"));
      Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
      Serial.print(F("http://")); readline(url, 79);
      Serial.println(url);
      
       Serial.println(F("****"));
       if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
         Serial.println("Failed!");
         break;
       }
       while (length > 0) {
         while (fona.available()) {
           char c = fona.read();
           
           // Serial.write is too slow, we'll write directly to Serial register!
           loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
           UDR0 = c;
           
           length--;
           if (! length) break;
         }
       }
       Serial.println(F("\n****"));
       fona.HTTP_GET_end();
       break;
    }
    
    
    /*****************************************/
      
    case 'S': {
      Serial.println(F("Creating SERIAL TUBE"));
      while (1) {
        while (Serial.available()) {
          fona.write(Serial.read());
        }
        if (fona.available()) {
          Serial.write(fona.read());
        }
      }
      break;
    }
    //***********************************************************************************
    //***********************************************************************************
    //***********************************************************************************
    // Instantiate the Adafruit_FONA object's begin method and 
    // display the results.
    case '&': 
      // Do not try to call the object's begin method more than once.
//      if(!fonaObjectInitialized)
//      {
        // See if the FONA is responding
        if (! fona.begin(4800)) // make it slow so its easy to read!
        {  
          Serial.println(F("Couldn't find FONA"));
          fonaObjectInitialized = false;
        }
//        else
//        {
//          Serial.println(F("FONA is OK"));
//          fonaObjectInitialized = true;
//        }
//      }
//      else
//        Serial.println(F("fona.begin() already executed."));
      break;
    //***********************************************************************************
    //***********************************************************************************
    //***********************************************************************************
    // Toggle the FONA power on or off. The FONA's PS (FONA_PS)
    // indicates whether the SIM800 power is on or off. 
    case '~':
      if(digitalRead(FONA_PS))
      {
        // The FONA PS
        // The SIM800 power is on turn it off. Assert the FONA_KEY_NOT
        // signal to toggle the power.
        digitalWrite(FONA_KEY_NOT, LOW);
        delay(2100);
        // Make sure that the power is off of the SIM800.
        if(!digitalRead(FONA_PS))
          Serial.println(F("SIM800 Power if off."));
        else
          Serial.println(F("SIM800 failed to power off."));
      }
      else
      {
        // The SIM800 power is off turn it on. Assert the FONA_KEY_NOT
        // signal to toggle the power.
        digitalWrite(FONA_KEY_NOT, LOW);
        delay(2100);
        // Make sure that the power is off of the SIM800.
        if(!digitalRead(FONA_PS))
          Serial.println("SIM800 Power if on.");
        else
          Serial.println("SIM800 failed to power on.");
      }
      digitalWrite(FONA_KEY_NOT, HIGH);
      break;
    
    //************************************************************************************
    //***********************************************************************************
    //***********************************************************************************
    // Reset the FONA.
    case '#': 
        digitalWrite(FONA_RESET_NOT, LOW);
        delay(110);
        digitalWrite(FONA_RESET_NOT, HIGH);
        Serial.println(F("SIM800 Reset asserted for 110 mSec."));
      break;
    
    //************************************************************************************
    // Read and show the states of the FONA's Power and Network status
    // signals.
    case '$': 
      Serial.println(F("Wait while the state of the Net Connection is determined."));            
      if(!digitalRead(FONA_PS))
      {
        Serial.println(F("Power Status is Asserted (Low)"));        
      }
      else
      {
        Serial.println(F("Power Status is Negated (High)"));        
      }
      // Determine the state of the network connection:
      //  1.  Off > 3 seconds        SIM800 is not running.
      //  2.  64 ms on 800 ms off    SIM800 is not registered to network.
      //  2.  64 ms on 3000 ms off   SIM800 is registered on the network. 
      //  2.  64 ms on 300 ms off    GPRS Communication established.
      DisplayNetworkStatus();
      break;
      
    default:
      Serial.println(F("Unknown command"));
      printMenu();
      break;
  }  // End of switch (command)
  // flush input
  flushSerial();
  while (fona.available()) {
    Serial.write(fona.read());
  }

}

void flushSerial() {
    while (Serial.available()) 
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}
  
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;
  
  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while(Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;
        
        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }
    
    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
//
// Determine and display the Networ Status.
void DisplayNetworkStatus()
{
  // Determine the state of the SIM800's network connection 
  // by observing the tramsitions on the NS (Net Status) signal:
  //  1.  Off > 3 seconds        SIM800 is not running.
  //  2.  64 ms on 800 ms off    SIM800 is not registered to network.
  //  2.  64 ms on 3000 ms off   SIM800 is registered on the network. 
  //  2.  64 ms on 300 ms off    GPRS Communication established.
  
  #define MAX_NS_OFF_MSECS    3000
  #define MAX_NS_ON_MSECS     64

  unsigned long currentTime;
  unsigned long OffTime;
  unsigned long OnTime;
  boolean timedOut;
  //  1.  Get synchronized, i.e. find the positive transition if there
  //      is one so that we can measure the  following off period.
  //  2.  Wait not longer than MAX_NS_OFF_MSECS for the NS signal
  //      to go high.
  //    A.  If you wait longer than MAX_NS_OFF_MSECS then display
  //        SIM800 is not running.
  //  3.  Wait not longer than MAX_NS_ON_MSECS for the NS signal 
  //      to go low.
  //    A.  If you wait longer than MAX_NS_ON_MSECS then report
  //        measurement error 1.
  //  4.  Wait not longer than MAX_NS_OFF_MSECS + TTT for the NS signal
  //      to go high and record elapsedTime.
  //    A.  If the elapsed time is > AAA and < BBB then display(300)
  //        GPRS Communication established.
  //    B.  If the elapsed time is > CCC and < DDD then display(800)
  //        SIM800 is not registered to network.
  //    C.  If the elapsed time is > EEE and < FFF then display(800)
  //        SIM800 is not registered to network.
  //    D.  If you wait longer than MAX_NS_OFF_MSECS then report
  //        measurement error 2
  //
  //  1.  If NS is high then wait a maximum of MAX_NS_ON_MSECS
  //      for it to go low to get synchronized.  
//Serial.print(F("The initial state of NS = "));
//Serial.println(digitalRead(FONA_NS), DEC);
  cnslSetTimeout((unsigned long) MAX_NS_ON_MSECS+20  );
  timedOut = false;
  while(digitalRead(FONA_NS) && !timedOut)
  {
    delay(5);
    timedOut  = cnslTimeout();      
  }
  // If we have timed out then the NS signal was high > 64 mSec's
  // and something is broken
  if(timedOut)
  {
     Serial.println(F("ERROR: The SIM800's NS signal is HIGH > 64 mSec's."));
Serial.print(F("Timed out. elapsedTime = "));
Serial.println(elapsedTime, DEC);
  }  
  else
  {
    // 2.  The NS signal is low we can now measure the high transition
    //     period if any. Wait not longer than MAX_NS_OFF_MSECS for the
    //     NS signal to go high.
    cnslSetTimeout((unsigned long) MAX_NS_OFF_MSECS );
    timedOut = false;
    while(!digitalRead(FONA_NS) && !timedOut)
    {
      delay(5);
      timedOut  = cnslTimeout();      
    } 
    // If we have timed out then the NS signal was off for a period
    // of MAX_NS_OFF_MSECS and the SIM800 is not running.
    if(timedOut)
    {
      Serial.println(F("The SIM800 is not running."));
Serial.print(F("Timed out. elapsedTime = "));
Serial.println(elapsedTime, DEC);
    }
    else
    {
      // We have not timed out and the NS signal has gone high. We
      // now have to wait no longer than MAX_NS_ON_MSECS for NS to
      // go low so we can measure the period of the low time and
      // thus the state of the Net Connection.
      cnslSetTimeout((unsigned long) MAX_NS_ON_MSECS);
      timedOut = false;
      while(digitalRead(FONA_NS) && !timedOut)
      {
        delay(5);
        timedOut  = cnslTimeout();      
      } 
      // If we timed out then the NS signal was high too long and
      // it is an error condition.
      if(timedOut)
      {
        Serial.println(F("ERROR: NS high period > 64mSec's."));
//Serial.print(F("On time error. elapsedTime = "));
//Serial.println(elapsedTime, DEC);
      }
      else
      {
//Serial.print(F("NS on time meadured okay. elapsedTime = "));
//Serial.println(elapsedTime, DEC);
        // We are in the NS low period that indicates the Net Status
        // measure the length of low period. Wait no longer than
        // MAX_NS_OFF_MSECS for the NS signal to go back high 
        // indicating the end of the low period and the status of the
        // Net.
        cnslSetTimeout((unsigned long) MAX_NS_OFF_MSECS );
        timedOut = false;
        while(!digitalRead(FONA_NS) && !timedOut)
        {
          delay(5);
          timedOut  = cnslTimeout();      
        }
        // If we timed out then
        if(timedOut)
          Serial.println(F("ERROR: NS high period after Net state low indicator."));
        else
        {
//          Serial.print(F("We did not time out. elapsedTime = "));
//          Serial.println(elapsedTime, DEC);
          // We did not time out and the duration of the off time indicates
          // the state of the SIM800's network connection.
          //  64 ms on 800 ms off    SIM800 is not registered to network.
          //  64 ms on 3000 ms off   SIM800 is registered on the network. 
          //  64 ms on 300 ms off    GPRS Communication established.
          if(elapsedTime >= 300 && elapsedTime <= 300)
            Serial.println(F("GPRS Communication established."));
          else if(elapsedTime >= 500 && elapsedTime <= 1000)
            Serial.println(F("SIM800 is not registered to network."));
          else if(elapsedTime >= 2000 && elapsedTime <= 3200)
            Serial.println(F("SIM800 is registered on the network."));
          else
            Serial.println(F("Error: NS signal off period error."));
        }
      }  // End of  // We are in the NS low period that indicates the Net Status
    }  // End of // We have not timed out and the NS signal has gone high.
  }  // End of "The NS signal is low we can now measure the high transition"
}  // End of DisplayNetworkStatus()



// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
void cnslSetTimeout( unsigned long t)
{
  
 cnslTimeoutPeriod = t;
 cnslPreviousMillis = millis();
}
boolean cnslTimeout() 
{
  unsigned long currentMillis;  
  currentMillis = millis();  
  //check if time has rolled over
  if ( currentMillis < cnslPreviousMillis)
  {
    elapsedTime = 
      MAX_UNSIGNED_LONG - cnslPreviousMillis + currentMillis;
    if (elapsedTime >= cnslTimeoutPeriod)
      return true; //timeout period has been reached
    else
      return false; //timeout period has not been reached
  }
  //time has not rolled over, simply compute elaspedTime
  else
  {
    elapsedTime = currentMillis - cnslPreviousMillis;

    if (elapsedTime >= cnslTimeoutPeriod)
      return true; //timeout period has been reached
    else
      return false; //timeout period has not been reached
  }
}

