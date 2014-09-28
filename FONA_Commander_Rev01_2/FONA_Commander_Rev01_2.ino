//
// The purpose of this sketch is to interface the Arduino Mega2560 5V
// 16 MHz and the Sparkfun Mega Pro 3.3V 8 MHz with the Adafruit FONA.
//
// The sketch provides a means of specifing canned AT commands in a
// list and selecting selecting from the list for output to the FONA.
//
// The sketch records a limited amount of the data output on the 
// SIM800'sserial transmit signal and provids the display of this
// data. See RESPONSE_MAX and RESPONSE_BYTES_MAX for setting the
// size of the recording.
//
// To add a canned AT command add the following line types in the
// AT_CommandList file:
//  #define AT_xxx  to identify the command
//  prog_char AT_Command_1[] PROGMEM = "...";
//    Where the string "..." is the exact characters sent to the
//    SIM800.
//    AT_Command_nn, is a pointer to the AT string.
// In this file set NUMBER_OF_AT_COMMANDS to the number of commands
// in the atCmdPointers[] array. Don't forget the ',' last entry in
// the array.
//
// To add new command to the sketch see the 'a' command for an example
// and template. The template demonstrates adding a command with op 
// input parameters.

extern PROGMEM const char *atCmdPointers [];
// NUMBER_OF_AT_COMMANDS sets the number of AT command strings
// pointers in the PROGMEM const char *atCmdPointers [] array
// in the AT_Command_List file.
#define NUMBER_OF_AT_COMMANDS  41

// ATCMD_DISPLAY_MAX sets the maximum number of characters in the
// AT command including the null terminator. See 
// prog_char AT_Command_nn[] PROGMEM = "AT+HTTPTERM\r\n";in the
// AT_Command_List file.
#define ATCMD_DISPLAY_MAX          60

#define NO_OF_CMDS_PER_LINE    3
#define CR                     0xD    // \r  Carriage return
#define LF                     0xA    // \n  New Line aka Line feed
#define RESPONSE_MAX           20
#define RESPONSE_BYTES_MAX     4000
#define RSP_SECOND_CHAR        1
//
// The sketch also provides commands for controlling the following
// FONA/SIM800 control and status lines.
//  1.  Key - This is also a super important pin (but not as important as Vio). 
//      This is the power on/off indicator. Its also tied to the button in the 
//      top left. Tie this pin to ground for 2 seconds to turn the module on or
//      off. It's not a level signal so it isn't like "low is off, high is on"
//      - instead you must pulse it for 2 seconds to turn off/on. The module 
//      comes by default off. Tie this permanently to ground if you never want 
//      your micro to turn off the FONA for power saving. This signal is tied to
//      DIO3
#define FONA_KEY_NOT 3
//  2.  PS - this is the Power Status pin. It is low when the module is off and
//      high when the module has power. If you're using the Key button or pin, 
//      you can monitor this pad to see when the module's booted up. This is 
//      tied to the Pwr LED too. This signal is tied to DIO4
#define FONA_PS 4
//  3.  NS - this is the Network Status pin. It pulses to signal the current status
//      of the module. This is also tied to the Net LED. Tied to DIO2
#define FONA_NS 2
//  4.  Reset - this is module hard reset pin. By default it has a high pull-up
//      (module not in reset). If you absolutely got the module in a bad space, 
//      toggle this pin low for 100ms to perform a hard reset. This signal is 
//      tied to DIO5
#define FONA_RESET_NOT 5
//******************************************************************
//******************************************************************
//******************************************************************
#define IDE_INPUT_CHARS_MAX        50
#define FIRST_CMD_CHAR             0
#define SECOND_CMD_CHAR            1
#define THIRD_CMD_CHAR             2
#define MAX_DISP_RSP               200
#define UNKNOWN_COMMAND            0xFF      // loop() switch state.
#define IGNORE_INPUT_CHAR          0xFE      // loop() switch state.
#define SEND_AT_COMMAND            0xFD      // loop() switch state.

//******************************************************************
// Variables associated with the SIM800 commands.
//******************************************************************
char ideChar;
char ideBuff[IDE_INPUT_CHARS_MAX];
int ideIdx;
int ideCharCounter;
int atCommandNumber;
int ideState;
unsigned long lastCmdTimeMsec;
//******************************************************************
// Variables associated with recording the SIM800 serial output.
//******************************************************************
#define LOOK_FOR_CR                  0 
#define LOOK_FOR_CR_LF               1
#define LOOK_FOR_CR_LF_RSP           2
#define LOOK_FOR_CR_LF_RSP_CR        3
#define LOOK_FOR_CR_LF_RSP_CR_LF     4

char response[RESPONSE_BYTES_MAX];            //
unsigned long rspStartTimes[RESPONSE_MAX];    //
unsigned long rspAssctCmd[RESPONSE_MAX];      //
int rspStartIdx[RESPONSE_MAX];                //
int rspCharCount[RESPONSE_MAX];               //
char dispBuff[ATCMD_DISPLAY_MAX];
int rspIdx;
boolean ideStringInput = false;
String dispString;
int rspCharIndex;
char rspChar;
boolean rspFirstChar;
boolean rspEchoOn;
boolean rspCr;
int rspCrLfCounter;
String rspString;
int lookState;


// Variables associated with Ben's time out routines.
#define MAX_UNSIGNED_LONG 0xFFFFFFFF
unsigned long elapsedTime;
unsigned long cnslTimeoutPeriod;
unsigned long cnslPreviousMillis;
//******************************************************************
//                            setup()
//******************************************************************
void setup()
{
  // Setup coommunications with the Arduino IDE.
    Serial.begin(9600);
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
  
  // Setup communications with the SIM800 on the FONA board.
    Serial3.begin(4800);
    
    Serial.println(F("\nFONA/SIM800 Commander_Rev01 Arduino 1.0.5\n"));
    displayMenu();
    //lastCmdTimeMsec = millis();
    initForCmd();
    initRsp();
}  // End of void setup()
//******************************************************************
//                              loop()
//******************************************************************
void loop()
{
  if (Serial.available())
  {
    // A character was received from the Arduino IDE.
    ideChar = Serial.read();
    ideBuff[ideCharCounter] = ideChar;
    if(ideChar == CR);
    else if(ideChar == LF)
    {
      if(!ideStringInput)
        parseCommand();
      else
        parseParameters();
    }
    else
      ideCharCounter++;
//******************************************************************
//                COMMAND PROCESSOR    switch (ideChar)
//******************************************************************
    switch (ideState) 
    {  
      case IGNORE_INPUT_CHAR:
      break;
      
       case '?':
        displayMenu();
        initForCmd();
      break;
      
       case 'b':
        displayMenu();
        initForCmd();
      break;
      
//******************************************************************
//        switch (ideChar)      case 'a':
//******************************************************************
      case 'a':
        // The 'a' command is a template for adding commands to the
        // FONA_Commander sketch.
        #define CMD_STATE    0
        #define GO_PAST_NL   1
        #define GET_PARM_1   2
        static int aCmdSubState = CMD_STATE;
        // This command needs an input string.
        switch(aCmdSubState)
        {  
          case CMD_STATE:
            ideStringInput = true;
            aCmdSubState = GET_PARM_1;
            ideCharCounter = 0;
            Serial.println(F("Input the parameters for the  'a' command."));
          break;
          
           case GET_PARM_1:
             if(!ideStringInput)
             {
               String aString = "";
               int aCmdIdx = 0;
               while(ideBuff[aCmdIdx] != 0)
                 aString+=ideBuff[aCmdIdx++];
               Serial.print(F("IDE parameter string = "));
               Serial.println(aString);
               aCmdSubState = CMD_STATE;
               initForCmd();
             }
        break;  // Out of switch(aCmdSubState)
        } 
      break;  // Out of switch (ideState)
//******************************************************************
//        switch (ideChar)      case SEND_AT_COMMAND:
//******************************************************************
      case SEND_AT_COMMAND:
        // Here to execute the selected SIM800 AT command.
        // atCommandNumber contains the index into the
        // atCmdPointers[] array.
        if(atCommandNumber < NUMBER_OF_AT_COMMANDS)
        {
        char dispChar;
        int idx = 0;
        dispString = "Command[";
        dispString+=atCommandNumber;
        dispString+="] at ";
        dispString+=millis();
        dispString+= " = ";
        strcpy_P(dispBuff, 
          (char*)pgm_read_word(&(atCmdPointers[atCommandNumber])));
        while((dispChar = dispBuff[idx++]) != 0)
        {
          if(dispChar == CR)
            dispString+="<CR>";
          else if(dispChar == LF)
            dispString+="<LF>";
          else
            dispString+=dispChar;          
        }
        lastCmdTimeMsec = millis();
        Serial3.write((const uint8_t*)dispBuff, idx);
        Serial.println(dispString);
        lookState = LOOK_FOR_CR;
        }
        else
          Serial.println(F("Illegal Command Number Entered."));
        initForCmd();

      break;
//******************************************************************
//   switch (ideChar)      case 'd':  Display SIM800 responses.
//******************************************************************
      case 'd':
        if(rspIdx != 0)
        {
          // Display all responses recorded in the response[] array
          for(int rspTmpIdx = 0; rspTmpIdx < rspIdx; rspTmpIdx++)
          {
            rspDisplay(rspTmpIdx);   
           
           
          }   
        } 
        else
            Serial.println("Response Recording Buffer Enpty.");
        initForCmd();
      break;
                
//******************************************************************
//   switch (ideChar)      case 'e':  Clear recorded response data.
//******************************************************************
      case 'e':
        initRsp();
        Serial.println(F("\nResponse Buffer Cleared."));
        Serial.println(F("Recording SIM800 Responses."));
        initForCmd();        
      break;
    
//******************************************************************
//        switch (ideChar)      case '~':
//******************************************************************
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
          Serial.println(F("SIM800 Power if on."));
        else
          Serial.println(F("SIM800 failed to power on."));
      }
      digitalWrite(FONA_KEY_NOT, HIGH);
      initForCmd();
      break;
//******************************************************************
//        switch (ideChar)      case '#':
//******************************************************************
    // Reset the FONA.
    case '#': 
        digitalWrite(FONA_RESET_NOT, LOW);
        delay(110);
        digitalWrite(FONA_RESET_NOT, HIGH);
        Serial.println(F("SIM800 Reset asserted for 110 mSec."));
        initForCmd();
      break;
//******************************************************************
//        switch (ideChar)      case '$':
//******************************************************************
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
      initForCmd();
      break;
//******************************************************************
//        switch (ideChar)      case default:
//******************************************************************
    default:
      Serial.println(F("Unknown command select from the following set."));
      displayMenu();
      initForCmd();
      break;
    }  // End of switch (ideChar)
  }   // End of if (Serial.available())
//******************************************************************
//           Record the SIM800 serial output responses.
//******************************************************************
  if(Serial3.available())
  {
    // During the recording of the serial output of the SIM800
    // if the string <CR LF>response<CR LF> is detected then display 
    // response on a new line.
    // A character was sent out of the SIM800's TX signal.
    rspChar = Serial3.read();
    // Ignore any nonprintable characters the SIM800 does
    // output them.
    checkForResponse(rspChar);
    if((rspChar != 0) && (rspChar <= '~'))
    {
      // See if there is enough room in the response 
      // buffer.
      if(rspCharIndex  < RESPONSE_BYTES_MAX )
      {
        response[rspCharIndex] = rspChar;
        rspCharCount[rspIdx]++;
        if(rspFirstChar)
        {
          // It is the start of a pesponse.
          rspFirstChar = false;
          rspStartTimes[rspIdx] = millis();  
          rspAssctCmd[rspIdx] = lastCmdTimeMsec;
          rspStartIdx[rspIdx] = rspCharIndex;
          // If the first character is not a CR then the
          // SIM800 has it "input echo input characters"
          // flag set to echo.
          if(rspChar != CR)
             rspEchoOn = true;
          else
            rspCr = true;
        }
        else
        {
          // It is not the first character.
          if(rspEchoOn)
          {
            // Echo is on look for the CR indicating the end
            // of the command echo.
            if(rspChar == CR)
            {
                rspIdx++;
                rspCr = false;
                rspFirstChar = true;
                rspEchoOn = false;
                rspCrLfCounter = 0;
            }
          }
          else
          {
            // The SIM00's echo option is off. We are into 
            // the response now look for the second <CR LF>
            // pair to indicate the end of the response.
            if(rspChar == CR)
              rspCr = true;
            if((rspChar == LF) && rspCr)
            {
              rspCr = false;
              rspCrLfCounter++;
            }
            if(rspCrLfCounter == 2)
            {
              // We have reached the end of the response. See if
              // there is enough room for another response
              if(rspIdx < RESPONSE_MAX )
              {
                rspIdx++;
                rspCr = false;
                rspFirstChar = true;
                rspEchoOn = false;
                rspCrLfCounter = 0;
              }              
            }  // End of  if(rspCrLfCounter == 2)
          }  // End of else The SIM00's echo option is off. 
        }  // End of else it's not first character in response.
      }  // End of if(rspCharIndex  < RESPONSE_BYTES_MAX )
      rspCharIndex++;
    }
  }  // End of if (Seria2.available())

}  // End of void loop()
//******************************************************************
//                          parseCommand()
//******************************************************************
void parseCommand()
{
  char ideChar = ideBuff[0];
  // We are here because we were waiting for  a command from
  // the Arduino IDE Serial Monitor.
  // Check for a valid command or string input.
  //  1.  A single alpha character followed by a LF.
  //  2.  Two decimal digits indicating that an AT command
  //      is to be sent to the SIM800.
  //    A.  ideStringInput is false.
  // ideCharCounter can be 1 or 2  
//Serial.println("parseCommand()");
//Serial.print("ideCharCounter = ");
//Serial.print(ideCharCounter, DEC);
//Serial.print("  ideChar = 0x");
//Serial.print(ideChar, HEX);
//Serial.print("  ideBuff[1] = 0x");
//Serial.println(ideBuff[1], HEX);
  if(ideCharCounter == 0 || ideCharCounter > 2)
    // Illegal input, too many command characters.
    initForCmd();
  else if(ideCharCounter == 1) 
  {
    // It is a single digit decimal command or single character
    // prinable.
    if(ideChar >= '0' && ideChar <= '9') 
    {
      // It is a single digit command index.
      ideState = SEND_AT_COMMAND;
      atCommandNumber = ideChar - '0';
    }
    else if(ideChar >= '!' && ideChar <= '~')
      ideState = ideChar;
    else
    // Illegal input format.
    initForCmd();
  }
  else
  {
//Serial.println("Valid 2 char command 1.");
    // It must be a two digit AT command index.
    if((ideChar >= '0') && (ideChar <= '9') && 
                  (ideBuff[1] >= '0') && (ideBuff[1] <= '9'))
    {
//Serial.println("Valid 2 char command 2.");
      atCommandNumber = (ideChar - '0') * 10; 
      atCommandNumber+=(ideBuff[1] - '0');
      ideState = SEND_AT_COMMAND;
    }
    else
    {
      // Illegal input format.
      initForCmd();
    }    
  }
}
//******************************************************************
//                          parseParameters()
//******************************************************************
void parseParameters()
{
  ideStringInput = false;  
}
//******************************************************************
//                          initForCmd()
//******************************************************************
void initForCmd()
{
  // Here to get ready for next IDE input.
//Serial.println("initForCmd() ");
  for(int idx = 0; idx < IDE_INPUT_CHARS_MAX; idx++)
    ideBuff[idx] = 0;
  ideIdx = 0;
  ideCharCounter = 0;
  ideState = IGNORE_INPUT_CHAR;

  ideIdx = 0;
}
//******************************************************************
//                          displayMenu()
//******************************************************************
extern PROGMEM const char *atCmdPointers[];
void displayMenu()
{
  char dispBuff[50];
  int cmdPtrIdx = 0;
  int lineIdx = 0;
  int atCmdIdx = 0;
  int displayLineLength = 0;
  boolean startNewLine = false;
  String dispString = "";
  
  Serial.println(F("[?] Print this menu."));
  Serial.println(F("[~] Toggle SIM 800's Power On and Off"));
  Serial.println(F("[#] Reset the FONA Module"));
  Serial.println(F("[$] Read SIM 800's Power and Network Status Signals."));
  Serial.println(F("[d] Display the recorded SIM800's responses."));
  Serial.println(F("[e] Erase the recorded SIM800's responses."));
  
  Serial.println(F("\nThe List of valid AT command numbers follows:\n                                              "));
  // 
  // while there are commands left to be displayed (cmdPtrIdx)
  while(cmdPtrIdx < NUMBER_OF_AT_COMMANDS)
  {
    // If there is room on this line (lineIdx) display another
    // AT command.
    // Move the next command from program memory to SRAM.
    strcpy_P(dispBuff, (char*)pgm_read_word(&(atCmdPointers[cmdPtrIdx])));
    int myStringLength = strlen(dispBuff);
    // If this command will extend beyond the max line length
    // then start a new line.
    if((myStringLength + displayLineLength)  >= ATCMD_DISPLAY_MAX)
    {
      lineIdx = 0;
      startNewLine = false;
      Serial.print("\n");
      displayLineLength=  0;
     }
    if(lineIdx < NO_OF_CMDS_PER_LINE && !startNewLine)
    {
      startNewLine = false;
      atCmdIdx = 0;
      // Initialize the AT command display string.
      dispString = "[";
      dispString+=cmdPtrIdx;
      dispString+= "]->";
      while(dispBuff[atCmdIdx] != CR)
        dispString+=dispBuff[atCmdIdx++];
      dispString+="\t";
      lineIdx++;
      cmdPtrIdx++;
      Serial.print(dispString);
      displayLineLength+=dispString.length();
//      if(displayLineLength > ATCMD_DISPLAY_MAX)
//      {
//        startNewLine = true;
//        displayLineLength = 0;
//      }
    }
    else   //    else begin a new line.
    {        
      lineIdx = 0;
      startNewLine = false;
      displayLineLength = 0;
      Serial.print("\n");
    }
  }  //  End of while(cmdPtrIdx < NUMBER_OF_AT_COMMANDS)
  Serial.print("\n");
 }

//******************************************************************
//                         DisplayNetworkStatus()
//******************************************************************
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
  unsigned long cnslTimeoutPeriod;
  unsigned cnslPreviousMillis;
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
//Serial.print(F("Timed out. elapsedTime = "));
//Serial.println(elapsedTime, DEC);
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
//Serial.print(F("Timed out. elapsedTime = "));
//Serial.println(elapsedTime, DEC);
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
//Serial.print(F("We did not time out. elapsedTime = "));
//Serial.println(elapsedTime, DEC);
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
//******************************************************************
//                      void rspDisplay(int idx)
//******************************************************************
void rspDisplay(int idx)
{
// Parse and display the idx'th response in the response[]
// array.
  char dspChar;
  char nextChar;
  String dispString = "Response at ";
  
  dispString+=rspStartTimes[idx];
  dispString+=" (";
  dispString+=(rspStartTimes[idx] - rspAssctCmd[idx]);
  //dispString+=(rspStartTimes[idx]);
  dispString+=") = ";
  for(int rspIdx = rspStartIdx[idx]; 
            rspIdx < (rspCharCount[idx] + rspStartIdx[idx]); rspIdx++)
  {
    dspChar = response[rspIdx];
    nextChar = response[rspIdx + 1];
    if(dspChar == CR && nextChar != LF)
      dispString+="<CR>";
    else if(dspChar == LF)
      dispString+="<CR LF>";
    else
      dispString+=dspChar;
  }
  Serial.println(dispString );
}
//******************************************************************
//                      cnslSetTimeout()
//******************************************************************
// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
void cnslSetTimeout( unsigned long t)
{
 cnslTimeoutPeriod = t;
 cnslPreviousMillis = millis();
}
//******************************************************************
//               boolean cnslTimeout()
//******************************************************************
boolean cnslTimeout() 
{
  unsigned long currentMillis = millis();  
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
//******************************************************************
//               void initRsp()
//******************************************************************
void initRsp()
{
  rspChar = 0;
  rspCharIndex = 0;
  rspFirstChar = true;
  rspIdx = 0;
  rspEchoOn = false;
  rspCr = false;
  rspCrLfCounter = 0;
 
  for(int idx = 0; idx<RESPONSE_BYTES_MAX; idx++)
    response[idx] = 0;
  for(int idx = 0; idx<RESPONSE_MAX; idx++)
  {
     rspStartTimes[idx] = 0;
     rspStartIdx[idx] = 0;
     rspCharCount[idx] = 0;
  }
}



//******************************************************************
//          void checkForResponse(char chr)
//******************************************************************
// We have a stste machine that is "called" with each 
// character in the SIM800's output stream. If the state
// machine detects the string then it outputs <OK> to the
// display
void checkForResponse(char chr)
{
  switch(lookState)
  {
    case LOOK_FOR_CR:
     if(chr == CR)
     {
       rspString = "Response at ";
       lookState = LOOK_FOR_CR_LF;
     }
    break;
    case LOOK_FOR_CR_LF:
     if(chr == LF)
     {
       rspString+=millis();
       rspString+=" <CR LF>";
       lookState = LOOK_FOR_CR_LF_RSP_CR; 
     }
      else
        lookState = LOOK_FOR_CR;
    break;
    case LOOK_FOR_CR_LF_RSP_CR:
     if( chr != CR)
     {
       if(rspString.length() < MAX_DISP_RSP)
         // Limit the number of characters in the displayed 
         // response.
         rspString+=chr;
         // Remain in this state.
       else
       {
         Serial.println("Response too big.");
         lookState = LOOK_FOR_CR;
       }
     }
      else
      {
        rspString+="<CR>";
        lookState = LOOK_FOR_CR_LF_RSP_CR_LF;
      }
    break;
    case LOOK_FOR_CR_LF_RSP_CR_LF:
     if(chr == LF)
     {
       rspString+="<LF>";
       Serial.println(rspString);       
     }
      else
        lookState = LOOK_FOR_CR;
    break;
    default:
      lookState = LOOK_FOR_CR;
    break;
  }  
}













// We have a stste machine that is "called" with each 
// character in the SIM800's output stream. If the state
// machine detects the string then it outputs <OK> to the
// display
//void checkForOK(char chr)
//{
//#define LOOK_FOR_CR                  0 
//#define LOOK_FOR_CR_LF               1
//#define LOOK_FOR_CR_LF_O             2
//#define LOOK_FOR_CR_LF_O_K           3
//#define LOOK_FOR_CR_LF_O_K_CR        4
//#define LOOK_FOR_CR_LF_O_K_CR_LF     5
//static int lookState = LOOK_FOR_CR;
//  switch(lookState)
//  {
//    case LOOK_FOR_CR:
//     if(chr == CR)
//       lookState = LOOK_FOR_CR_LF;
//    break;
//    case LOOK_FOR_CR_LF:
//     if(chr == LF)
//       lookState = LOOK_FOR_CR_LF_O; 
//      else
//        lookState = LOOK_FOR_CR;
//    break;
//    
//    case LOOK_FOR_CR_LF_O:
//     if(chr == 'O')
//       lookState = LOOK_FOR_CR_LF_O_K;
//      else
//        lookState = LOOK_FOR_CR;
//    break;
//    
//    case LOOK_FOR_CR_LF_O_K:
//     if(chr == 'K')
//       lookState = LOOK_FOR_CR_LF_O_K_CR;
//      else
//        lookState = LOOK_FOR_CR;
//    break;
//    
//    case LOOK_FOR_CR_LF_O_K_CR:
//     if(chr == CR)
//       lookState = LOOK_FOR_CR_LF_O_K_CR_LF;
//      else
//        lookState = LOOK_FOR_CR;
//    break;
//    
//    case LOOK_FOR_CR_LF_O_K_CR_LF:
//     if(chr == LF)
//        Serial.println("<OK>");
//        lookState = LOOK_FOR_CR;
//    break;
//    
//    default:
//      lookState = LOOK_FOR_CR;
//    break;
//  }
//}

