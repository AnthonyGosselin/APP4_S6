#include "GlobalVars.hpp"
#include "BitManager.hpp"
#include "FrameManager.hpp"

#include <stdlib.h>     /* srand, rand */

SYSTEM_THREAD(ENABLED);

enum StateDuration {
    shortPeriod, 
    longPeriod, 
    veryLongPeriod
};

bool BitMEFVerbose = false;

int outputPin = D6; 
int inputPin = D7;

bool BIT1 = true;
bool BIT0 = false;

InputState CurrentInputState;

// Transmission speed variables
int transmissionSpeed = 0;
int speedInterrupts = 0;
unsigned long firstSpeedInterruptTime = 0;
int transmissionSpeeds[3] = {1, 10, 100};   // Transmission speed per bit (in ms)

bool skippedLastInputEvent = false;

bool inputCurrentStateHigh = false;
int lastChangeTime = 0;
int inputClockPeriod = 1; // Will be updated depending on preambule
const int outputClockPeriod = 7; // Fixed output clock

// If 80% higher than one clock period: must be two periods (AKA: long period)
float longPeriodMin;
float longPeriodMax;
float shortPeriodMin;

system_tick_t lastThreadTime = 0;
system_tick_t lastMessageTime = 0;

bool isRecevingData = false;

void BitManagerSetup() {

    pinMode(outputPin, OUTPUT_OPEN_DRAIN);
    pinMode(inputPin, INPUT_PULLUP);

    digitalWrite(outputPin, LOW);

    attachInterrupt(inputPin, inputEvent, CHANGE, 1);
    CurrentInputState = initial;

    Thread thread("outputThread", outputThread);
}


// ----------
// HANDLE INPUT 
// ----------


bool getTransmissionSpeed(){

    // Discard first preambule event if it comes after a HIGH input state (because first symbol must be '0')
    // Occurs when bit gets switched to '0' at the end of the transmission of a previous message.
    if (!inputCurrentStateHigh && speedInterrupts == 0) {
        if (BitMEFVerbose) {
            Serial.println("Discarding interrupt (high to low) for speed calculation");
        }

        return false;
    }

    if (BitMEFVerbose) {
        Serial.printlnf("Speed interupt count: %d", speedInterrupts);
    }

    if (speedInterrupts < 7){
        if (speedInterrupts == 0)
            firstSpeedInterruptTime = millis();
        speedInterrupts++;
        return false;
    }
    else{
        // Get time since last interrupt
        unsigned long currentTime = millis();
        int elapsedTime = currentTime - firstSpeedInterruptTime;
        
        // Calculate speed by meaning
        transmissionSpeed = elapsedTime/7/2;

        speedInterrupts = 0; // Reset counter

        WITH_LOCK(Serial){
            Serial.printlnf("Clock speed: %d ms", transmissionSpeed);
        }
        inputClockPeriod = transmissionSpeed; // Set global clock speed variable

        // If 80% higher than one clock period: must be two periods (AKA: long period)
        longPeriodMin = inputClockPeriod * 1.5;
        longPeriodMax = inputClockPeriod * 2.5;
        shortPeriodMin = inputClockPeriod * 0.5;

        return true;
    }
};

// Function used to change state and perform necessary actions right away (like outputing)
void changeInputState(InputState newInputState) {
    switch (newInputState) {
        case output0:
            //Register that a 0 has been read
            if(BitMEFVerbose){
                WITH_LOCK(Serial){
                   Serial.println("READ: 0");
                }
            }
            receiveBit(0b00000000);
            break;
        case output1:
            //Register that a 1 has been read
            if(BitMEFVerbose){
                WITH_LOCK(Serial){
                    Serial.println("READ: 1");
                }
            }
            receiveBit(0b00000001);
            break;
    }
    CurrentInputState = newInputState; // Change to new state for next event
}

void inputEvent() {

    if(!isRecevingData){
        return;
    }

    int duration = millis() - lastChangeTime;
    lastChangeTime = millis();

    if(duration < shortPeriodMin) {
        // if(BitMEFVerbose){
        //     Serial.printlnf("Rejecting too short impulse of %d ms, smaller than min %f ms", duration, shortPeriodMin);
        // }
        return;
    }

    // Reset higher state machine if long pause
    if (duration > 5*longPeriodMax) {
        //Serial.println("RESETING");
        resetCounters();
    }

    inputCurrentStateHigh = digitalRead(inputPin) == HIGH; //!inputCurrentStateHigh;

    //Compute transmission speed at the beginning of each frame
    if (currentReceivingState == preambule) {
        bool speedComputeComplete = getTransmissionSpeed();
        if (speedComputeComplete) {
            // Done receiving preambule bits
            receiveData((uint8_t)0b01010101); // Notify msgManager that preambule has been received
            CurrentInputState = initial;
        }
        return;
    }

    // Determine newStateDuration (time since last change event)
    StateDuration newStateDuration;
    if (duration > longPeriodMax) {
        newStateDuration = veryLongPeriod;

        CurrentInputState = initial;
        //if(BitMEFVerbose){Serial.println("Very long period detected: setting back to 'initial' input state");}
    }
    else if (duration >= longPeriodMin && duration < longPeriodMax) {
        newStateDuration = longPeriod;
    }
    else if (duration >= shortPeriod && duration < longPeriodMin) {
        newStateDuration = shortPeriod;
    }
    
    // Printing (debug)
    //if(BitMEFVerbose){Serial.printlnf("Read %s impulse duration: %d ms -> #%d (CurrentInputState: %d)", !inputCurrentStateHigh ? "HIGH" : "LOW", duration, newStateDuration, CurrentInputState);}

    // STATE MACHINE: Decode Manchester
    switch (CurrentInputState) {
        case initial:
            if (newStateDuration == shortPeriod || newStateDuration == veryLongPeriod || newStateDuration == longPeriod) {
                changeInputState(output0);
            } 
            else {
                //if(BitMEFVerbose){Serial.println("ERROR: initial state got longPeriod");}
            }
            break;

        case output0:
            if (newStateDuration == shortPeriod) {
               changeInputState(waitShort);
            }
            else if (newStateDuration == longPeriod) {
               changeInputState(output1);
            }
            // veryLongPeriod -> stay in output0
            break;

        case waitShort:
            if (newStateDuration != shortPeriod) {
                //if(BitMEFVerbose){Serial.printlnf("ERROR: expected shortPeriod in wait state got #%d", newStateDuration);}
            }
            changeInputState(output0);
            break;
        
        case output1:
            if (newStateDuration == shortPeriod) {
                changeInputState(waitLong);
            }
            else if (newStateDuration == longPeriod) {
                changeInputState(output0);
            }
            else {
                //if(BitMEFVerbose){Serial.println("UNDEFINED behaviour for veryLongPeriod in inputState 'output1'");}
            }

            break;
        
        case waitLong:
            if (newStateDuration != shortPeriod) {
                //if(BitMEFVerbose){Serial.printlnf("ERROR: expected shortPeriod in wait state got #%d", newStateDuration);}
            }
            changeInputState(output1);
            break;
    }
}


// ----------
// HANDLE OUTPUT 
// ----------


void output(PinState level) {
    digitalWrite(outputPin, level);
    os_thread_delay_until(&lastThreadTime, outputClockPeriod);
}

void sendBitsManchester(bool bits[], int bitCount) {
    int errorBitIndex = rand() % bitCount + 50;

    for (int i = 0; i < bitCount; i++) {

        bool bitValue = bits[i];
        if (insertBitError && i == errorBitIndex) { 
            Serial.printlnf("Error bit at index %d (%d -> %d)", i, bitValue, !bitValue);
            bitValue = !bitValue;
        }

        if (bitValue == BIT1) {
            // Send 1 in Manchester
            //Serial.println("SEND: 1");
            output(HIGH);
            output(LOW);
        }
        else {
            // Send 0 in Manchester
            //Serial.println("SEND: 0");
            output(LOW);
            output(HIGH);
        }
        // if ((i+1)%8 == 0){
        //     if(isVerbose){
        //         WITH_LOCK(Serial){
        //             Serial.println("---------");
        //         }
        //     }
        // }
    }

    digitalWrite(outputPin, LOW); // Bring back to low at the end of the message for next message
}

void outputThread() {
    //delay(5000);
    //Serial.println("Starting output loop");
    while(true) {

        // Check if more items in list than processed, if yes, process item at currentIndex
        if(currentSendingFrameObjIndex < sendingFrameObjIndex){
            Serial.println("Ready to send frame!");
            isRecevingData = true;
            //Serial.printlnf("Current sending %d, total sending %d", currentSendingFrameObjIndex, sendingFrameObjIndex);

            sendBitsManchester(sendingFrameObjList[currentSendingFrameObjIndex].bitArray, sendingFrameObjList[currentSendingFrameObjIndex].bitArraySize);

            isRecevingData = false;
            digitalWrite(outputPin, LOW);
            currentSendingFrameObjIndex++;
        }
        else {
            //Serial.println("Not ready to send frame...");
            Serial.print(".");
        }

        // if(readyToSendFrame){
        //     Serial.println("Ready to send test frame!");

        //     int length = 5;
        //     bool bitsToSend[length] = {false, true, false, false, true};
        //     sendBitsManchester(bitsToSend, length);
        //     readyToSendFrame = false;
        // }
        // else {
        //     Serial.println("Not ready to send test frame...");
        // }
        
        os_thread_delay_until(&lastThreadTime, 200);
	}
}

