#include "MessageManager.cpp"

SYSTEM_THREAD(ENABLED);

void outputThread();

int outputPin = D6;
int inputPin = D7;

bool BIT1 = true;
bool BIT0 = false;

enum InputState {
    initial,
    output0,
    waitShort,
    output1,
    waitLong,
} CurrentInputState;


enum StateDuration {
    shortPeriod, 
    longPeriod, 
    veryLongPeriod
};

MessageManager msgManager;

// Transmission speed variables
int transmissionSpeed = 0;
int speedInterrupts = 0;
unsigned long firstSpeedInterruptTime = 0;

bool skippedLastInputEvent = false;

bool inputCurrentStateHigh = false;
int lastChangeTime = 0;
int inputClockPeriod = 500;
int outputClockPeriod = 500;

system_tick_t lastThreadTime = 0;
system_tick_t lastMessageTime = 0;

void setup() {
	Serial.begin(9600);

    pinMode(outputPin, OUTPUT);
    pinMode(inputPin, INPUT);

    digitalWrite(outputPin, LOW);

    attachInterrupt(inputPin, inputEvent, CHANGE);
    CurrentInputState = initial;

    msgManager = MessageManager();

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");
    Thread thread("outputThread", outputThread);
}

void loop() {
	// Do we really need to make a thread if we are not using main thread (do interrupts run on main thread??)

    // Call messages to send here, split by long delays?
    // char* message1 = [H, e, l, l, o, , W, o, r, l, d, !];
    // ex. msgManager.sendMessage(message1);
    // delay(10000); OR os_thread_delay_until(&lastMessageTime, 10000)
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
        transmissionSpeed = elapsedTime/7/2;

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
            msgManager.frameManager.receiveBit(0b0);
            break;
        case output1:
            // Register that a 1 has been read
            Serial.println("READ: 1");
            msgManager.frameManager.receiveBit(0b1);
            break;
    }
    CurrentInputState = newInputState; // Change to new state for next event
}

void inputEvent() {

    // Compute transmission speed at the beginning of each frame
    if (msgManager.frameManager.currentReceivingState == preambule) {
        bool speedComputeComplete = getTransmissionSpeed();
        if (speedComputeComplete) {
            // Done receiving preambule bits
            msgManager.frameManager.receiveData(0b01010101); // Notify msgManager that preambule has been received
        }
        return;
    }
    //----------

    int duration = millis() - lastChangeTime;
    lastChangeTime = millis();

    // If 80% higher than one clock period: must be two periods (AKA: long period)
    int longPeriodMin = inputClockPeriod * 1.8;
    int longPeriodMax = inputClockPeriod * 2.2;
    int shortPeriodMin = inputClockPeriod * 0.8;

    // Determine newStateDuration (time since last change event)
    StateDuration newStateDuration;
    if (duration > longPeriodMax) {
        newStateDuration = veryLongPeriod;
    }
    else if (duration >= longPeriodMin && duration < longPeriodMax) {
        newStateDuration = longPeriod;
    }
    else if (duration >= shortPeriod && duration < longPeriodMin) {
        newStateDuration = shortPeriod;
    }
    else {
        Serial.printlnf("Rejecting too short impulse of %d ms", duration);
        return;
    }
    
    // Printing (debug)
    Serial.printlnf("Read %s impulse duration: %d ms -> #%d (CurrentInputState: %d)", inputCurrentStateHigh ? "HIGH" : "LOW", duration, newStateDuration, CurrentInputState);
    inputCurrentStateHigh = !inputCurrentStateHigh;

    // STATE MACHINE: Decode Manchester
    switch (CurrentInputState) {
        case initial:
            if (newStateDuration == shortPeriod || newStateDuration == veryLongPeriod) {
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
    }
}

void outputThread() {
    delay(3000);
    Serial.println("Starting output loop");
    while(true) {

        bool bitsToSend[] = {BIT0, BIT1, BIT1, BIT1, BIT0, BIT1, BIT0, BIT1};
        sendBitsManchester(bitsToSend, 8);
        Serial.println("---------");
        os_thread_delay_until(&lastThreadTime, 2000);
        CurrentInputState = initial;
	}
}

