#include "Particle.h"

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

bool skippedLastInputEvent = false;

bool inputCurrentStateHigh = false;
int lastChangeTime = 0;
int clockPeriod = 500;

system_tick_t lastThreadTime = 0;

bool readFirstBit = false;
bool lastBitRead = BIT0; // First bit must be 0

void setup() {
	Serial.begin(9600);

    pinMode(outputPin, OUTPUT);
    pinMode(inputPin, INPUT);

    digitalWrite(outputPin, LOW);

    attachInterrupt(inputPin, inputEvent, CHANGE);
    CurrentInputState = initial;

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");
    Thread thread("outputThread", outputThread);
}

void loop() {
	// Do we really need to make a thread if we are not using main thread (do interrupts run on main thread??)
}

// Function used to change state and perform necessary actions right away (like outputing)
void changeInputState(InputState newInputState) {
    switch (newInputState) {
        case output0:
            // Register that a 0 has been read
            Serial.println("READ: 0");
            break;
        case output1:
            // Register that a 1 has been read
            Serial.println("READ: 1");
            break;
    }
    CurrentInputState = newInputState; // Change to new state for next event
}

void inputEvent() {
    int duration = millis() - lastChangeTime;
    lastChangeTime = millis();

    // If 80% higher than one clock period: must be two periods (AKA: long period)
    int longPeriodMin = clockPeriod * 1.8;
    int longPeriodMax = clockPeriod * 2.2;
    int shortPeriodMin = clockPeriod * 0.8;

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
// OUTPUT 
// ----------


void output(PinState level) {
    digitalWrite(outputPin, level);
    os_thread_delay_until(&lastThreadTime, clockPeriod);
}

void sendBitsManchester(bool bits[], int bitCount) {
    for (int i = 0; i < bitCount; i++) {
        if (bits[i] == BIT1) {
            //Serial.println("Sending 1");
            // Send 1 in Manchester
            //Serial.println("SEND: HIGH");
            output(HIGH);
            output(LOW);
        }
        else {
            //Serial.println("Sending 0");
            // Send 0 in Manchester
            //Serial.println("SEND: LOW");
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
        
        // digitalWrite(outputPin, HIGH);
        // // Serial.println("High");
		// os_thread_delay_until(&lastThreadTime, 1000);
        // digitalWrite(outputPin, LOW);
        // // Serial.println("Low");
        // os_thread_delay_until(&lastThreadTime, 1000);
	}
}

/*

// BEAST CODE A ETIENNE:

// Vars
bool speedCalculated = false;
int speedInterrupts = 0;
unsigned long preambStart = 0;
int transmissionSpeed = 0;

// Speed calc
if (!speedCalculated){

    if (speedInterrupts < 7){
      if (speedInterrupts == 0)
        preambStart = millis();
      speedInterrupts++;
    }
    else{
      // Get time since last interrupt
      unsigned long currentTime = millis();
      int elapsedTime = currentTime - preambStart;
      
      // Calculate speed by meaning
      //elapsedTime += elapsedTime/7    // Get full length (cuts of both ends)
      transmissionSpeed = elapsedTime/7/2;

      //speedCalculated = true;
      speedInterrupts = 0;

      Serial.printlnf("%d", transmissionSpeed);
    }
    //Serial.printlnf("%d", speedInterrupts);
  }

  */