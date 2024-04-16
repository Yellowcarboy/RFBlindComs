/*
  RFBlindComms - Arduino libary for remote control outlet switches
*/

#ifndef _RFBlindComs_h
#define _RFBlindComs_h

#include "Arduino.h"

#include <stdint.h>


#define MARKISOL_AGC1_PULSE                   4885  // 216 samples
#define MARKISOL_AGC2_PULSE                   2450  // 108 samples
#define MARKISOL_AGC3_PULSE                   1700  // 75 samples
#define MARKISOL_RADIO_SILENCE                5057  // 223 samples

#define MARKISOL_PULSE_SHORT                  340   // 15 samples
#define MARKISOL_PULSE_LONG                   680   // 30 samples

#define MARKISOL_COMMAND_BIT_ARRAY_SIZE       41    // Command bit count

class RFBlindComs {

  public:
    RFBlindComs();
    
    void send(const char* command);
    
    void enableReceive(int interrupt);
    void enableReceive();
    void disableReceive();
    bool available();
    void resetAvailable();

    volatile char* getReceivedValue();
 
    void enableTransmit(int nTransmitterPin);
    void disableTransmit();
    void setRepeatTransmit(int nRepeatTransmit);

  private:

    volatile static char nReceivedValue[50];
    volatile static bool nReceivedValueReady;
    static void handleInterruptx();
    volatile static int nReceiverInterrupt;
    void sendCommand(const char* command);

    int nTransmitterPin;
    int nRepeatTransmit;
    
    void transmitHigh(int delay_microseconds);
    void transmitLow(int delay_microseconds);
    void errorLog(String message);
    void doMarkisolTribitSend(const char* command);
};
#endif
