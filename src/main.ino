#pragma once
#include "MessageManager.ino"

SYSTEM_THREAD(ENABLED);

// void outputThread();

// int outputPin = D6;
// int inputPin = D7;

// bool BIT1 = true;
// bool BIT0 = false;

// enum InputState {
//     initial,
//     output0,
//     waitShort,
//     output1,
//     waitLong,
// } CurrentInputState;


// enum StateDuration {
//     shortPeriod, 
//     longPeriod, 
//     veryLongPeriod
// };

// MessageManager msgManager;

bool isVerbose = true;

// // Transmission speed variables
// int transmissionSpeed = 0;
// int speedInterrupts = 0;
// unsigned long firstSpeedInterruptTime = 0;

// bool skippedLastInputEvent = false;

// bool inputCurrentStateHigh = false;
// int lastChangeTime = 0;
// int inputClockPeriod = 500;
// int outputClockPeriod = 500;

// system_tick_t lastThreadTime = 0;
// system_tick_t lastMessageTime = 0;

// int bitCounter = 0;

void setup() {
	Serial.begin(9600);

    BitManagerSetup();

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");
    //Thread thread("outputThread", outputThread);
}

void loop() {

    delay(5000);
    uint8_t message1[4] = {0b01010101, 0b10101010, 0b00001111, 0b11110000};
    uint8_t messageSize = 4;
    sendMessage(message1, messageSize);


	// Do we really need to make a thread if we are not using main thread (do interrupts run on main thread??)

    // Call messages to send here, split by long delays?
    // char* message1 = [H, e, l, l, o, , W, o, r, l, d, !];
    // ex. msgManager.sendMessage(message1);
    // delay(10000); OR os_thread_delay_until(&lastMessageTime, 10000)
}

