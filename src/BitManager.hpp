#pragma once
#include <stdint.h>

enum InputState {
    initial,
    output0,
    waitShort,
    output1,
    waitLong,
};

void BitManagerSetup();
bool getTransmissionSpeed();
// Function used to change state and perform necessary actions right away (like outputing)
void changeInputState(InputState);
void inputEvent();
void output(PinState);
void sendBitsManchester(bool[], int);
void outputThread();