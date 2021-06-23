#include "GlobalVars.hpp"
#include "BitManager.hpp"
#include "FrameManager.hpp"

SYSTEM_THREAD(ENABLED);

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
int inputClockPeriod = 1;
int outputClockPeriod = 50;

system_tick_t lastThreadTime = 0;
system_tick_t lastMessageTime = 0;

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
        transmissionSpeed = elapsedTime/7/2; // ?? /2 ???

        speedInterrupts = 0; // Reset counter

        Serial.printlnf("Clock speed: %d", transmissionSpeed);
        inputClockPeriod = transmissionSpeed; // Set global clock speed variable

        return true;
    }
};

// Function used to change state and perform necessary actions right away (like outputing)
void changeInputState(InputState newInputState) {
    switch (newInputState) {
        case output0:
            // Register that a 0 has been read
            Serial.println("READ: 0");
            receiveBit(0b00000000);
            break;
        case output1:
            // Register that a 1 has been read
            Serial.println("READ: 1");
            receiveBit(0b00000001);
            break;
    }
    CurrentInputState = newInputState; // Change to new state for next event
}

void inputEvent() {

    int duration = millis() - lastChangeTime;
    lastChangeTime = millis();

    // If 80% higher than one clock period: must be two periods (AKA: long period)
    int longPeriodMin = inputClockPeriod * 1.8;
    int longPeriodMax = inputClockPeriod * 2.2;
    int shortPeriodMin = inputClockPeriod * 0.8;

    if(duration < shortPeriodMin) {
        Serial.printlnf("Rejecting too short impulse of %d ms, smaller than min %d ms", duration, shortPeriodMin);
        return;
    }

    //Compute transmission speed at the beginning of each frame
    if (currentReceivingState == preambule) {
        bool speedComputeComplete = getTransmissionSpeed();
        if (speedComputeComplete) {
            // Done receiving preambule bits
            receiveData((uint8_t)0b01010101); // Notify msgManager that preambule has been received
        }
        return;
    }

    // Determine newStateDuration (time since last change event)
    StateDuration newStateDuration;
    if (duration > longPeriodMax) {
        newStateDuration = veryLongPeriod;

        CurrentInputState = initial;
        Serial.println("Very long period detected: setting back to 'initial' input state");
    }
    else if (duration >= longPeriodMin && duration < longPeriodMax) {
        newStateDuration = longPeriod;
    }
    else if (duration >= shortPeriod && duration < longPeriodMin) {
        newStateDuration = shortPeriod;
    }
    
    
    // Printing (debug)
    if(BitMEFVerbose){Serial.printlnf("Read %s impulse duration: %d ms -> #%d (CurrentInputState: %d)", inputCurrentStateHigh ? "HIGH" : "LOW", duration, newStateDuration, CurrentInputState);}
    inputCurrentStateHigh = !inputCurrentStateHigh;

    // STATE MACHINE: Decode Manchester
    switch (CurrentInputState) {
        case initial:
            if (newStateDuration == shortPeriod || newStateDuration == veryLongPeriod || newStateDuration == longPeriod) {
                changeInputState(output0);
            } 
            else {
                Serial.println("ERROR: initial state got longPeriod");
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
                Serial.printlnf("ERROR: expected shortPeriod in wait state got #%d", newStateDuration);
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
                Serial.println("UNDEFINED behaviour for veryLongPeriod in inputState 'output1'");
            }

            break;
        
        case waitLong:
            if (newStateDuration != shortPeriod) {
                Serial.printlnf("ERROR: expected shortPeriod in wait state got #%d", newStateDuration);
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
    for (int i = 0; i < bitCount; i++) {
        if (bits[i] == BIT1) {
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
        if ((i+1)%8 == 0)
            Serial.println("---------");
    }
}

void outputThread() {
    //delay(5000);
    Serial.println("Starting output loop");
    while(true) {


        if(readyToSendFrame){
            Serial.println("Ready to send frame!");
            sendBitsManchester(bitArray, bitArraySize);
            readyToSendFrame = false;
            Serial.println("---------");
            // CurrentInputState = initial;
        }
        else {
            Serial.println("Not ready to send frame...");
        }
        
        os_thread_delay_until(&lastThreadTime, 2000);
	}
}

