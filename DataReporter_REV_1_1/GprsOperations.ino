//
// GprsOperations task mechanizes the following GPRS processes:
//  1.  Powers on, configures the modem and registers
//      for a GSM (Global System for Moble communications)
//      connection with the internet via the GPRS service 
//      (the transmission of digital data 
//      packets)provided by GSM .
//  2.  Connets to a specified server on a specified port.
//      When this task completes the modem is ready to 
//      communicate with internet based servers via TCP/IP.
//      The ISO stack is mechanized in the modem.
//  3.  Disconnects from the server.
//  4.  Safely powers off the modem.
//
// After applying power to the modem it attempts to verify that
// it can reach and communicate with GSM Base Station (tower).
// Once radio communication is established with a tower the modem
// uses the information in it's SIM (Subscriber Identification
// Module) card to attempt to register with a GSM provider. Once
// accepted by a GSM provider the modemi's home cell location
// and it's network address are established. The modem is now
// known to all GSM providers.
//
// Once registered the modem is configured to provide TCP/IP
// socket data packet type service over the iternet with a 
// set of modem AT control commands.
//
// This task only registers with the GSM's GPRS service and
// configures the modem for TCP/IP socket type data transfers
// it has no connection with the actual socket data transfers.
//
// The GprsOperations task-thread is a shared resource and
// communicates with the client tasks through a queued
// gprsControl structure.
//
//  Generally to use the GSRP modem the client must:
//  1.  Instantiate and fill out a gsrpControl structure.
//  2.  Wait for the modem to to power up and configure.
//  3.  Communicate with the server as required.
//      15 minutes maximum.
//      Indicate done with the GSRP modem.
//  
//  The modem’s power is normally off.
//  Details:
//  1.  Instantiate a modem control structure.
//  2.  Initialize the control structure:
//    A.  gprsOperation gprsStatus:
//      a.  Must initially be set to GPRS_POWER_ON.
//      b.  Used to indicate the status of the modem’s
//          power on, registration and configuration
//    B.  gprsClientOperation
//      a.  Must initially be set to GPRS_CLIENT_WAIT.
//    C.  server
//      a.  “your server url”
//    D.  Port
//      a.  Generally 80
//    E.  gprspStatus clientStatus
//      a.  Update to indicate the status of the 
//          server communication.
//    F.  result 
//      a.  Optionally check for error during the 
//          modem’s power on, registration and 
//          configuration or normal modem power down.
//  3.  Push the control structure on to the GSRP 
//      mopdem shared resource FIFO.
//    A.  PushGprs(struct gprsControl *controlPtr);
//  4.  Wait for the gprsStatus to indicate 
//      GPRS_DATA_EXCHANGE or GPRS_ERROR.
//  5.  Indicate server communication in progress
//      with gprsStatus = GPRS_CLIENT_TRANSFER
//  6.  Use the following to communicate with the
//      server as required:
//      A.  Serial1.print() or Serial1.println()
//          if(Serial1.available() != 0)
//          {
//            char charIn = Serial1.read);
//            .
//            .
//            .
//  7.  Set gprsStatus = GPRS_CLIENT_DONE to 
//      indicate the modem is no longer required.
//  8.  Optionally monitor gprsStatus for 
//      GPRS_POWER_OFF to indicate a normal modem
//      power down sequence.
//

#include <avr/pgmspace.h>
#include "Definitions.h"

// The gprsAtReq_n arrays are stored in program memory and contain
// the AT commands for configuring the Quectel M10 modem to
// provide TCP/IP data packet communication service over the GSM.
//****************************************************************
//*  NOTE:   The Quictel M10 modem serial input echo defaults to *
//*          the echo-on mode.                                   *
//****************************************************************
// 
// The number of steps required to configure the modem and connect
// to a server are:
#define GPRS_CONFIGURATION_STEPS      15
prog_char gprs_atRequest_0[] PROGMEM = "AT\r\0";
prog_char gprs_atRequest_14[] PROGMEM = "ATE0\r\0";
prog_char gprs_atRequest_1[] PROGMEM = "AT+CGREG?\r\0";
prog_char gprs_atRequest_2[] PROGMEM = "AT+IFC=1,1\r\0";
prog_char gprs_atRequest_3[] PROGMEM = "AT+CMGF=1\r\0";
prog_char gprs_atRequest_4[] PROGMEM = "AT+CLIP=1\r\0";
prog_char gprs_atRequest_5[] PROGMEM = "AT+COLP=1\r\0";
prog_char gprs_atRequest_6[] PROGMEM = "AT+CGATT=1\r\0";
prog_char gprs_atRequest_7[] PROGMEM = "AT+QIFGCNT=0\r\0";
prog_char gprs_atRequest_8[] PROGMEM = "AT+QICSGP=1,\"epc.tmobile.com\",\"\",\"\"\r\0";
prog_char gprs_atRequest_9[] PROGMEM = "AT+QIMUX=0\r\0";
prog_char gprs_atRequest_10[] PROGMEM = "AT+QIMODE=1\r\0";
prog_char gprs_atRequest_11[] PROGMEM = "AT+QINDI=1\r\0";
prog_char gprs_atRequest_12[] PROGMEM = "AT+QIREGAPP\r\0";
prog_char gprs_atRequest_13[] PROGMEM = "AT+QIACT\r\0";

// The following define the indexes for associating the AT
// requests strings and their addresses.

#define GPRS_AT         0
#define GPRS_ATE0       1
#define GPRS_CGREG      2
#define GPRS_IFC        3
#define GPRS_CMGF       4
#define GPRS_CLIP       5
#define GPRS_COLP       6
#define GPRS_CGATT      7
#define GPRS_QIFGCNT    8 
#define GPRS_QICSGP     9
#define GPRS_QIMUX      10
#define GPRS_QIMODE     11
#define GPRS_QINDI      12
#define GPRS_QIREGAPP   13
#define GPRS_QIACT      14

// the gprsRequestPtrs[] contains an ordered set of pointers
// to the GPRS AT commands above.
PROGMEM const char *gprsRequestPtrs[GPRS_CONFIGURATION_STEPS] =
{   
  gprs_atRequest_0,
  gprs_atRequest_1,
  gprs_atRequest_2,
  gprs_atRequest_3,
  gprs_atRequest_4,
  gprs_atRequest_5,
  gprs_atRequest_6,
  gprs_atRequest_7,
  gprs_atRequest_8,
  gprs_atRequest_9,
  gprs_atRequest_10,
  gprs_atRequest_11,
  gprs_atRequest_12,
  gprs_atRequest_13,
  gprs_atRequest_14
};

// The expected responses from the modem configuration
// commands depends on whether the modem's serial input
// echo is on or off. The default after power on is on.
prog_char gprs_okResponse_0[] PROGMEM = "\r\nOK\r\\n\0";
//
// Local definitions go here.
//
#define GPRS_PWRKEY_WAIT                    1
#define GPRS_AUTOBAUDING_WAIT               2
#define GPRS_CONFIGURE_NEXT_AT_COMMAND      3
#define GPRS_WAIT_VERIFY_AT_COMMAND         4
#define GPRS_CLIENT_TRANSFERING             5
#define GPRS_TURN_POWER_OFF                 6
#define GPRS_WAIT_AFTER_ERR                 7

#define GPRS_PWRKEY_ON                  12000    // 12 seconds.
#define GPRS_PWRKEY_OFF                 600      // 600 mSeconds.
#define GPRS_AUTOBAUD_TIMEOUT           10000    // 10 Seconds.
#define GPRS_CONFIGURE_STEP_TIMEOUT     20000    // 20 seconds.

// GPRS registeration error numbers.
#define GPRS_AUTOBAUD_CMD_ERR               0
#define GPRS_AUTOBAUD_TIMEOUT_ERR           1
#define GPRS_REGISTRATION_TIMEOUT           3

// The following define the state of the M10 modem's
// transmitted data parsing state machine.
#define GPRS_PARSE_BEGIN          0
#define GPRS_PARSE_TEXT           1
#define GPRS_PARSE_INITIAL_CR     2
#define GPRS_PARSE_INITIAL_CRLF   3
#define GPRS_PARSE_ONLY_LF        4
#define GPRS_PARSE_ERROR          5
//
// Local globals go here.
//
// gsrpExpectResponse is set true after an AT command 
// is output to the modem.
bool gsrpExpectResponse;
// gprsResponses is set to an empty string prior to any
// AT command eing output to the modem. All characters 
// received from  the modem are appended to this string.
String gprsResponses;
//
int gprsAtResponseCount;
// gsrpResponseCount is incremented if the following
// sequence is detected in the modem's serial output
// stream:
//        <CR><LF><"..."><CR><LF>
// Where <"..."> is one or more printable characters.
//
int gprsOtherResponseCount;
// gprsOtherResponseCount is incremented if the following
// sequence is detected in the modem's serial output
// Stream.
//        <...> <CR>
// Where <"..."> is one or more printable characters.
//
// gsrpResponseCount is the sum of gprsAtResponseCount 
// and gprsOtherResponseCount.
int gsrpResponseCount;
//
// gprsConfigureStep is the current state of 
// the GprsOperations() state machine.
int gprsConfigureStep;
//
int gsrpCmdLength;
//
// gprsModemOutParseState is the current state of 
// the modem serial output parsing state machine.
int gprsModemOutParseState;
//
// gprsValidTextCount counts the number of characters
// in the  <...> text string.
int gprsValidTextCount;
int gprsErrorCount;

sysLogControl gprsSysLogControl;
gprsControl *currentGprsOperation;
gprsOperation gprsOpStatusToReport;

// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
unsigned long gprsTimeoutPeriod;
unsigned long gprsPreviousMillis;


// Enter here only from the exec.
void GprsOperations(void)      
{  
  // Enter here from the executive after being scheduled
  // by some client wanting to use the GSM modem to access
  // some internet server. Continue to enter here until
  // client's request is completed error or no error,
  // any error message has been written to the system 
  // log file, the modem power has been removed and the
  // ModemOperations() queue is tested for empty.
  //
  // If we are NOT currently in the initial 
  // TASK_INIT_STATE or the client-transfering-state 
  // GPRS_CLIENT_TRANSFERING then this task must handle
  // the serial input from the modem in addition to
  // registering on the GMS and connecting to a server.
  // The following if statement is parsed and executed 
  // left to right. The call to .available()
  // is not made if the left term is false.
  if(tasksState[GPRS_TASK] != GPRS_CLIENT_TRANSFERING
          && tasksState[GPRS_TASK] != TASK_INIT_STATE
                          && Serial1.available() != 0)
  {
    // Handle modem serial input here when the modem
    // is in the command mode. When the modem is in 
    // the data mode this task is in the 
    // GPRS_CLIENT_TRANSFERING state and all modem
    // serial interchanges are handled by the client.

    // The modem has varied responses but they can be
    // classified by format:
    //  1.  <...> <CR>
    //    A.  Examples:
    //      a.  AT<CR>
    //          1.  Modem serial input echo on.
    //      b.  +CME ERROR: 100<CR>
    //  2.  <CR><LF><...><CR><LF>
    //    A.  Examples:
    //      a.  <CR><LF><OK><CR><LF>
    //      b.  <CR><LF><ERROR><CR><LF>
    //      c.  <CR><LF><NORMAL POWER DOWN><CR><LF>
    //      c.  <CR><LF><+PDP DEACT><CR><LF>
    // 
    // Get the character from the modem.
    char m10Input = Serial1.read();
    // Only allow CR, LF, or printable characters.
    if(m10Input == '\r' || m10Input == '\r' 
          || (m10Input >= ' ' && m10Input <= '~'))
    {
      // Add the character to the modem input string.
      gprsResponses+=m10Input;
      switch(gprsModemOutParseState)
      {
        case GPRS_PARSE_BEGIN:
          // It is the 1st character of an echo'ed 
          // AT command, an AT command response or
          // an unsoliticed message. We know it's a
          // valid char from the modem.
          // It can be 1st char of a <...> or a <CR>
          if(m10Input != '\r')
            gprsModemOutParseState = GPRS_PARSE_INITIAL_CR;
          else
          {
            // It's the 1st character of<...>.
            gprsValidTextCount++;
            gprsModemOutParseState = GPRS_PARSE_TEXT;
          }
        break;

        case GPRS_PARSE_TEXT:
          // Only additional text or <CR> is allowed 
          // in this state. We know it's valid text
          // the only invalid char is <LF>
          if(m10Input != '\n')
            // Stay in this state parsing more text.
            // It's already been added to the modem
            // input string.
            gprsValidTextCount++;
          else if(m10Input == '\r')
          {
            // It's <...><CR> 
            gprsOtherResponseCount++;
            gprsValidTextCount = 0;
            gprsModemOutParseState = GPRS_PARSE_BEGIN;
          }
          else
            gprsModemOutParseState = GPRS_PARSE_ERROR;
        break;

        case GPRS_PARSE_INITIAL_CR:
          // The 1st character of a new modem output
          // a <CR> and only a <LF> can follow it.
          if(m10Input == '\r')
            gprsModemOutParseState = GPRS_PARSE_INITIAL_CRLF;
          else
            gprsModemOutParseState = GPRS_PARSE_ERROR;
        break;

        case GPRS_PARSE_INITIAL_CRLF:
          // We have the <CR><LF> portion of a 
          //<CR><LF><...><CR><LF> only text is allowed
          // to follow before the <CR>.
          if(m10Input == '\n')
            gprsModemOutParseState = GPRS_PARSE_ERROR;          
          if(m10Input == '\r')
            gprsModemOutParseState = GPRS_PARSE_ONLY_LF;          
          // else stay in this state.          
        break;

        case GPRS_PARSE_ONLY_LF:
          // We have <CR><LF><...><CR> and we allow
          // <LF>.
          if(m10Input == '\n') 
          {
            gprsAtResponseCount++;
            gprsValidTextCount = 0;
            gprsModemOutParseState = GPRS_PARSE_BEGIN;
          }
          else
            gprsModemOutParseState = GPRS_PARSE_ERROR;
        break;

        case GPRS_PARSE_ERROR:
            gprsValidTextCount = 0;
            gprsErrorCount++;
            gprsModemOutParseState = GPRS_PARSE_ERROR;        
        break;

        default:
        break;
      }  // End of switch(gprsModemOutParseState)
    }    // End of if(m10Input == '\r' ...)

    else
      gprsErrorCount++;
    
    // Look for CR alone. gprsCrReceived may be end of echo or look for following LF
    // Look for CR LF may be begining or ending of response.
    // Valid responses:
  }
  
  // Execute in the current state.
  switch(tasksState[GPRS_TASK])
  {
      case TASK_INIT_STATE:
        // We ar here because:
        //  1.  A client pushed a request in the GprsOperations()'s
        //      queue with a call to PushGprs(struct gprsControl 
        //      *controlPtr)
        //  2.  A client has successfully completed it's 
        //      communications with it's server and we are supposed
        //      check the queue for more requests.
        //  3.  The clients request was not successfully completed
        //      because there was an error:
        //    A.  The client was notified.
        //    B.  An error message was written to the system log
        //        describing error.
        //    C.  This task waited for the log to be written.
        //    D.  The power was removed from the modem
        // See if there are any requests for the GSM resource.
        if((currentGprsOperation = PopGprs()) != QUEUE_EMPTY)
        {
          // Enter here to power up and register the Quectel
          // M10 Cellular Engine. In any case don't let the
          // processor sleep during any portion of this 
          // thread's processes.
          sei(); 
          keepAwakeFlags |= (1<<GPRS_TASK);
          cli();
          // Turn the modem on to begin the process of 
          // registering the modem with the cellular tower.
          // Assert the M10's PWRKEY signal.
          digitalWrite(GPRS_M10_PWRKEY, HIGH);
          // Preset status for a successful registration
          // and connection.
          gprsOpStatusToReport = GPRS_POWER_OFF ;
          // Delay with the PWRKEY modem signal asserted.
          gprsSetTimeout(GPRS_PWRKEY_ON);
          tasksState[GPRS_TASK] = GPRS_PWRKEY_WAIT;
        }
        else
        {
          // The modem queue is empty. Put the modem in the
          // AT command mode.Close the internet connection 
          // in case it is open.
          // WE MAY NOT WANT TO DO THE FOLLOWING???????????
          // Have we told the client that
          Serial1.println("+++AT+QICLOSE");    
          taskScheduled[GPRS_TASK] = false;
        }
        break;

      case GPRS_PWRKEY_WAIT:
        // Has the PWRKEY signal been asserted long enough?
        if(gprsTimeout())
        {
          // Negate the M10's PWRKEY signal.
          digitalWrite(GPRS_M10_PWRKEY, LOW);
          // The first thing that the modem does after being
          // powered on is to automatically set the baud rate.
          // In order to accomplish autobauding the modem
          // must receive some serial input.           
          // Set the time to wait for the "OK" response.
          gprsSetTimeout(GPRS_AUTOBAUD_TIMEOUT);
          // Prepare to receive a response from the
          // from the modem and send an AT request to 
          // the modem so that the modem can set the
          // baud rate.
          setToReceiveResponse();
          Serial1.print(gprsRequestPtrs[GPRS_AT]);
          tasksState[GPRS_TASK] = GPRS_AUTOBAUDING_WAIT;        
        }
        // Remain in this state.
        break;
        
      case GPRS_AUTOBAUDING_WAIT:
        // We are here to wait for the modem to respond "OK"
        // to the AT command that we sent to the modem so
        // it could automatically set it's baud rate.
        //
        // The responses should be:
        //    AT<cr>              what we sent (since echo is on).
        //    <cr><lf>OK<cr><lf>  What modem responded.
        //  1.  Have we received the responses yet?
        //    A.  No. Have we waited too long?
        //      a. No.
        //      b. Yes.
        //    B.  Yes. Are they what we expect.
        //      a.  No.
        //      b.  Yes.
        if(gsrpResponseCount == 2)
        {
          if((gprsResponses.lastIndexOf(gprsRequestPtrs[GPRS_AT]) == 0) &&
           (gprsResponses.lastIndexOf(gprs_okResponse_0)) == 3)
           {
           // The response to the AT command to allow the modem
           // to automatically determine the baud rate was as 
           // expected continue with the modem configuration 
           // proceedure.
           gprsConfigureStep = GPRS_ATE0;
           // Prepare to receive a response from the
           // from the modem and send an AT request to 
           // the modem so that the modem can set the
           // baud rate.
           tasksState[GPRS_TASK] = GPRS_CONFIGURE_NEXT_AT_COMMAND;        
           }
          else
          {
            // There was some kind of error in the responses
            // to the AT command to allow autobauding. The
            // modem is not available.
            GprsReportError(GPRS_AUTOBAUD_CMD_ERR);
          }    
        }
        else if(gprsTimeout())
          // We have timed out waiting for the responses to
          // the autobauding AT command.
          GprsReportError(GPRS_AUTOBAUD_TIMEOUT_ERR);
        // else continue to wait in this state.
        break;

      case GPRS_CONFIGURE_NEXT_AT_COMMAND:
        //
        // The modem configuration consists of 
        // GPRS_CONFIGURATION_STEPS. In each step the 
        // modem is given the next of the required 
        // configuration commands and the response(s) to 
        // the command is(are) verified. The current step
        // is indicated by gprsConfigureStep
        setToReceiveResponse();
        // Increment gprsConfigureStep to indicate the 
        // next step in the modem configuration and 
        // output the AT command to the modem.
        Serial1.print(gprsRequestPtrs[++gprsConfigureStep]);
        // Wait a limited time for the response.
        gprsSetTimeout(GPRS_CONFIGURE_STEP_TIMEOUT);
        // We may need this in verifying the response.
        gsrpCmdLength = sizeof(gprsRequestPtrs[gprsConfigureStep]);
        tasksState[GPRS_TASK] = GPRS_WAIT_VERIFY_AT_COMMAND;
      break;
      
      case GPRS_WAIT_VERIFY_AT_COMMAND://**************************************************************************************************************
        // We are here to wait for the response to the last
        // modem configuration AT command.
        //
        // The modem may be outputting characters and the
        // parsing state machine may be in any state.
        if(!gprsTimeout())
        {
          // We have not timed out yet. Have we received an
          // AT command response from the modem?
          if(gsrpResponseCount != 0)
          {
            // A valid modem AT command response consists of 
            // the following sequence:
            //       <CR><LF><"..."><CR><LF>
            // There may be one or more responses to an AT command
            // depending on the AT command sent to the modem. There
            // may also be responses like:
            //       "..."<CR>
            if(gprsConfigureStep == GPRS_CGREG )
            {
              // The AT command was to get the registration status.
              // this command has three responses the echo, the 
              // status response and the "OK"response.
              //    AT+CGREG?<CR>                Because echo is on.
              //    <CR><LF>+CGREG: 0,x<CR><LF>  We ask for status.
              //        x = 1, 2 or 3
              //    <CR><LF>OK<CR><LF>           Normal OK response.
              //     
              if(gsrpResponseCount == 3)
              {
                //  Verify the responses.
                //?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
                
              }
            }
            else if(gprsConfigureStep == GPRS_ATE0)
            {
              // We have not timed out, We have a response to the
              // GPRS_AT modem configuration AT command. Echo is
              // still on. The responses should be:*********************************************************************************************************
              //        ATE0<CR>
              //        <CR><LF><"OK"><CR><LF>
              // See if we received both responses.
              if(gsrpResponseCount == 2)
              {
                //  Verify the responses to the .
                //?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
              }
              
            }
            else
            {
              // We have not timed out, We have a response to the
              // next modem configuration AT command. Echo is off
              // and the command is supposed to have a single "OK"
              // response.
              //????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
            }
          }  // End of if(gsrpResponseCount != 0) and not timed out.
        }  // End of   if(!gprsTimeout())
        else
        {
         // We are configuring the modem with AT commands
         // and have timed out waiting for one of them to
         // respond. Log the error to the System Log and
         // let the client
         // know what has happened.
          GprsReportError(GPRS_REGISTRATION_TIMEOUT);
        }
      break;
      
      case GPRS_WAIT_AFTER_ERR:
        // There has been an error in registering to GSM or 
        // connecting to server, the system log has been
        // written and we are waiting for the SystemLog task
        // to complete writting to the SD file.
        if(gprsSysLogControl.stat == SUCCESS)
        {
          // Let the client know that there was an error 
          // in registration or connection.
          gprsOpStatusToReport = GPRS_OPERATION_ERROR;
          // Power off the modem.
          digitalWrite(GPRS_M10_PWRKEY, HIGH);
          // Delay with the PWRKEY modem signal asserted.
          gprsSetTimeout(GPRS_PWRKEY_OFF);
          tasksState[GPRS_TASK] = GPRS_TURN_POWER_OFF;
        }
        // else continue to wait.
      
      break;
      
      case GPRS_TURN_POWER_OFF:
        // We are done using the modem, we have asserted the
        // modem's PWRKEY for GPRS_PWRKEY_OFF for 
        // GPRS_PWRKEY_OFFmSec's to power down the modem,
        // and are waiting to negate the PWRKEY.
        if(gprsTimeout())
        {
          // Allow sleep as far as this task is concerned.
          sei(); 
          keepAwakeFlags &= (!(1<<GPRS_TASK));
          cli();
          // Let the client know how things turned out.
          currentGprsOperation->gprsStatus = gprsOpStatusToReport;
          // Go check for modem requests in FIFO queue.
          tasksState[GPRS_TASK] = TASK_INIT_STATE;
         }
        // else continuw to wait.
        break;
        
      default:
      
      break;
  }
}  // End of void GprsOperations(void)

//
// Prepare to receive a response from the modem.
void setToReceiveResponse()
{
  // Show that we are expecting a response from the modem.
  gsrpExpectResponse = true;        
  // Show that no responses have been parsed
  // from the modem output..
  gsrpResponseCount = 0;
  gprsAtResponseCount = 0;      // Counts all AT responses.
  gprsOtherResponseCount = 0;   // Other valid responses 
  gprsValidTextCount = 0;       // Counts only chars in <...>.
  gprsErrorCount = 0;

  // Clear the string where responses are stored.
  gprsResponses = ""; }

// Tasks requiring a time out unique to the task use a "uniqued"
// version of Ben's monitor timeout methods, monitorSetTimeout() &
// monitorTimeout(), i.e. change monitor to the task's unique
// name.
void gprsSetTimeout( unsigned long t)
{
 gprsTimeoutPeriod = t;
 gprsPreviousMillis = millis();
}
boolean gprsTimeout() 
{
  unsigned long elapsedTime;
  unsigned long currentMillis = millis();  
  //check if time has rolled over
  if ( currentMillis < gprsPreviousMillis)
  {
    elapsedTime = MAX_UNSIGNED_LONG - gprsPreviousMillis + currentMillis;
    if (elapsedTime >= gprsTimeoutPeriod)
      return true; //timeout period has been reached
    else
      return false; //timeout period has not been reached
  }
  //time has not rolled over, simply compute elaspedTime
  else
  {
    elapsedTime = currentMillis - gprsPreviousMillis;

    if (elapsedTime >= gprsTimeoutPeriod)
      return true; //timeout period has been reached
    else
      return false; //timeout period has not been reached
  }
}


// Report GPRS operation error. All GSM errors are fatal.
// 
void GprsReportError(int errorNo)
{
      // Fill out the system log control structure.
      cnslSysLogControl.msgIdx = GPRS_REGISTER_MSG; // Set index to message.
      cnslSysLogControl.stat = NOT_STARTED;                 // Status.      
      cnslSysLogControl.type = SYSLOG_INTEGER;      
      cnslSysLogControl.validParnmCount = SYSLOG_2_PARAMETER;      
      cnslSysLogControl.parameter.intParm[0] = errorNo;
      cnslSysLogControl.parameter.intParm[1] = tasksState[GPRS_TASK];
      // Indicate the error type in the clients GSM modem
      // control structure.
      // Push the system log request onto the FIFO queue.
      cli();    // Interrupts off.
      PushSysLog(&gprsSysLogControl);
      sei();    // Interrupts on. 
      // Wait a limited time for the SystemLog task to write
      // the error message.
      tasksState[GPRS_TASK] = GPRS_WAIT_AFTER_ERR;
}


            // When you receive a <CR> and we are not in a 
            // valid modem AT command response sequence then 
            // don't wait longer than ??? milliseconds for a
            // <LF>. ???????????????????????????????????????????????????????????
            //  How long do you wait for the character following
            //  a <CR> before deciding if it's a valid AT command
            //   sequence.??????????????????????????????????????????????????????
            // We do not know what a ULR response looks like.??????????????????????????????????????????????

