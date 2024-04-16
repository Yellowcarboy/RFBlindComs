/*
  RFBlindsComs - Arduino libary for remote control outlet switches
*/
//#define RECEIVE_ATTR ICACHE_RAM_ATTR

#include "RFBlindComs.h"

#include <ArduinoLog.h>

volatile char RFBlindComs::nReceivedValue [50];
volatile int RFBlindComs::nReceiverInterrupt = -1;
volatile bool RFBlindComs::nReceivedValueReady = false;

RFBlindComs::RFBlindComs() {
  this->nTransmitterPin = -1;
  this->setRepeatTransmit(10);
  RFBlindComs::nReceiverInterrupt = -1;
  RFBlindComs::nReceivedValue;
  RFBlindComs::nReceivedValueReady = false;
}

/**
 * Sets Repeat Transmits
 */
void RFBlindComs::setRepeatTransmit(int nRepeatTransmit) {
  this->nRepeatTransmit = nRepeatTransmit;
}

/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RFBlindComs::enableTransmit(int nTransmitterPin) {
  this->nTransmitterPin = nTransmitterPin;
  pinMode(this->nTransmitterPin, OUTPUT);
}

/**
  * Disable transmissions
  */
void RFBlindComs::disableTransmit() {
  this->nTransmitterPin = -1;
}


/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void RFBlindComs::send(const char* command) {
  if (this->nTransmitterPin == -1)
    return;

  // make sure the receiver is disabled while we transmit
  int nReceiverInterrupt_backup = RFBlindComs::nReceiverInterrupt;
  if (nReceiverInterrupt_backup != -1) {
    this->disableReceive();
  }


  for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
    this->sendCommand(command);
  }

  // Disable transmit after sending (i.e., for inverted protocols)
  digitalWrite(this->nTransmitterPin, LOW);

  // enable receiver again if we just disabled it
  if (nReceiverInterrupt_backup != -1) {
    this->enableReceive(nReceiverInterrupt_backup);
  }
}

/**
 * Enable receiving data
 */
void RFBlindComs::enableReceive(int interrupt) {
  RFBlindComs::nReceiverInterrupt = interrupt;
  this->enableReceive();
}

void RFBlindComs::enableReceive() {
  if (RFBlindComs::nReceiverInterrupt != -1) {
    RFBlindComs::nReceivedValueReady = false;
    attachInterrupt(RFBlindComs::nReceiverInterrupt, handleInterruptx, CHANGE);
  }
}

/**
 * Disable receiving data
 */
void RFBlindComs::disableReceive() {
  detachInterrupt(RFBlindComs::nReceiverInterrupt);
  RFBlindComs::nReceiverInterrupt = -1;
}

bool RFBlindComs::available() {
  return RFBlindComs::nReceivedValueReady;
}

void RFBlindComs::resetAvailable() {
  RFBlindComs::nReceivedValueReady = false;
}

volatile char* RFBlindComs::getReceivedValue() {
  return RFBlindComs::nReceivedValue;
}

#define RECEIVE_ATTR ICACHE_RAM_ATTR

void RECEIVE_ATTR RFBlindComs::handleInterruptx() {

  static unsigned int i = 0;
  static unsigned long lastTime = 0;
  static bool AGC1 = false;
  static bool AGC2 = false;
  static bool AGC3 = false;
  static int pinState = 0;

  const char log1 = '1';
  const char log0 = '0';

  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (AGC1 && AGC2 && AGC3){
    pinState = digitalRead(RFBlindComs::nReceiverInterrupt); 
    //Log.trace(F("data Duration %d" CR),duration);
    if (pinState ==0){
      if (duration > 500 && duration < 800) { // Found 1
        RFBlindComs::nReceivedValue[i] = log1;
        i++;      
      } else if (duration > 200 && duration < 400) { // Found 0
        RFBlindComs::nReceivedValue[i] = log0;
        i++;
      } else { // Unrecognized bit, finish
          //Log.trace(F("Reset %d " CR),i);
          AGC1 = false;
          AGC2 = false;
          AGC3 = false;
          i = 0;
      }

      if (i == MARKISOL_COMMAND_BIT_ARRAY_SIZE){
        //set value
        //RFBlindComs::nReceivedValue = command;
        RFBlindComs::nReceivedValueReady = true;
        Log.trace(F("Found RF %s" CR), RFBlindComs::nReceivedValue);
      }
    }
  }

  // *********************************************************************
  // Wait for the first AGC:
  // *********************************************************************
  // HIGH between 4500-6000 us
  // *********************************************************************
  if(duration > 4500 && duration < 6000){
    AGC1 = true;
    AGC2 = false;
    AGC3 = false;
    i = 0;
    //Log.trace(F("Init %d " CR),i);
    //Log.trace(F("AC1 Duration %d" CR),duration);
  } 

  // *********************************************************************
  // Wait for the second AGC:
  // *********************************************************************
  // LOW between 2300-2600 us
  // *********************************************************************
  if(duration > 2300 && duration < 2600 && AGC1){
    AGC2 = true;
    AGC3 = false;
    //Log.trace(F("AC2 Duration %d" CR),duration);
  } 
  
  // *********************************************************************
  // Wait for the third AGC:
  // *********************************************************************
  // HIGH between 1100-1900 us
  // *********************************************************************
  if(duration > 1100 && duration < 1900 && AGC1 && AGC2){
    AGC3 = true;
    //Log.trace(F("AC3 Duration %d" CR),duration);
  } 
  
  lastTime = time;  
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void RFBlindComs::transmitHigh(int delay_microseconds) {
  digitalWrite(this->nTransmitterPin, HIGH);
  //PORTB = PORTB D13high; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
void RFBlindComs::transmitLow(int delay_microseconds) {
  digitalWrite(this->nTransmitterPin, LOW);
  //PORTB = PORTB D13low; // If you wish to use faster PORTB calls instead
  delayMicroseconds(delay_microseconds);
}
void RFBlindComs::errorLog(String message) {
  Serial.println(message);
}

void RFBlindComs::sendCommand(const char* command) {

  if (command == NULL) {
    this->errorLog("sendMarkisolCommand(): Command array pointer was NULL, cannot continue.");
    return;
  }
 
  
  this->doMarkisolTribitSend(command);

  // Radio silence at the end of last command.
  // It's better to go a bit over than under minimum required length:
  this->transmitLow(MARKISOL_RADIO_SILENCE);


  // Disable output to transmitter to prevent interference with
  // other devices. Otherwise the transmitter will keep on transmitting,
  // disrupting most appliances operating on the 433.92MHz band:
  digitalWrite(this->nTransmitterPin, LOW);
}

void RFBlindComs::doMarkisolTribitSend(const char* command) {
  const char log1 = '1';
  const char log0 = '0';

  // Starting (AGC) bits:
  this->transmitHigh(MARKISOL_AGC1_PULSE);
  this->transmitLow(MARKISOL_AGC2_PULSE);
  this->transmitHigh(MARKISOL_AGC3_PULSE);
  char c[50];
  strcpy(c, command);

  // Transmit command:
  for (int i = 0; i < MARKISOL_COMMAND_BIT_ARRAY_SIZE; i++) {

      // If current bit is 0, transmit LOW-HIGH-LOW (010):
      if (c[i] == log0) {
        this->transmitLow(MARKISOL_PULSE_SHORT);
        this->transmitHigh(MARKISOL_PULSE_SHORT);
        this->transmitLow(MARKISOL_PULSE_SHORT);
      }

      // If current bit is 1, transmit LOW-HIGH-HIGH (011):
      if (c[i] == log1) {
        this->transmitLow(MARKISOL_PULSE_SHORT);
        this->transmitHigh(MARKISOL_PULSE_LONG);
      }   
   }
}
