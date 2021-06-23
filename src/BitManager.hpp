#pragma once
#include <stdint.h>

extern int outputPin;
extern int inputPin;

extern bool BIT1;
extern bool BIT0;

enum InputState {
    initial,
    output0,
    waitShort,
    output1,
    waitLong,
};


enum StateDuration {
    shortPeriod, 
    longPeriod, 
    veryLongPeriod
};

extern InputState CurrentInputState;

// Transmission speed variables
extern int transmissionSpeed;
extern int speedInterrupts;
extern unsigned long firstSpeedInterruptTime;
extern int transmissionSpeeds[3];   // Transmission speed per bit (in ms)

extern bool skippedLastInputEvent;

extern bool inputCurrentStateHigh;
extern int lastChangeTime;
extern int inputClockPeriod;
extern int outputClockPeriod;

extern system_tick_t lastThreadTime;
extern system_tick_t lastMessageTime;

void BitManagerSetup();
bool getTransmissionSpeed();
// Function used to change state and perform necessary actions right away (like outputing)
void changeInputState(InputState);
void inputEvent();
void output(PinState);
void sendBitsManchester(bool[], int);
void outputThread();

