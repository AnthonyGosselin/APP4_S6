#include "Particle.h"

SYSTEM_THREAD(ENABLED);

void outputThread();

Thread thread("outputThread", outputThread);

int outputPin = D6;
int inputPin = D7;

bool BIT1 = true;
bool BIT0 = false;

enum InputState {
    initial,
    output0,
    output1,
    wait,
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
}

void loop() {
	// Do we really need to make a thread if we are not using main thread (do interrupts run on main thread??)
}

void inputEvent() {
    int duration = millis() - lastChangeTime;
    lastChangeTime = millis();
    
    // Printing (debug)
    Serial.printlnf("Read %s impulse duration: %d ms", inputCurrentStateHigh ? "HIGH" : "LOW", duration);
    inputCurrentStateHigh = !inputCurrentStateHigh;

    // If 80% higher than one clock period: must be two periods (AKA: long period)
    int longPeriodMin = clockPeriod * 1.8;
    int longPeriodMax = clockPeriod * 2.2;

    // Determine newStateDuration
    StateDuration newStateDuration;
    if (duration > longPeriodMax) {
        newStateDuration = veryLongPeriod;
    }
    else if (duration < longPeriodMax && duration > longPeriodMin) {
        newStateDuration = longPeriod;
    }
    else {
        newStateDuration = shortPeriod;
    }

    // STATE MACHINE
    switch (CurrentInputState) {
        case initial:
            if (newStateDuration == shortPeriod || newStateDuration == veryLongPeriod) {
                CurrentInputState = output0;
            } 
            else {
                Serial.println("ERROR: initial state got longPeriod");
            }
            break;

        case output0:
            Serial.println("READ: 0");
            if (newStateDuration == shortPeriod) {
                CurrentInputState = wait;
            }
            else if (newStateDuration == longPeriod) {
                CurrentInputState = output1;
            }
            // veryLongPeriod -> stay in output0
            break;

        case wait:
            if (newStateDuration != shortPeriod) {
                Serial.printlnf("ERROR: expected shortPeriod in wait state got state #%d", newStateDuration);
            }
            CurrentInputState = output0;
            break;
        
        case output1:
            Serial.println("READ: 1");
            if (newStateDuration != longPeriod) {
                Serial.printlnf("ERROR: expected longPeriod in output1 state got state #%d", newStateDuration);
            }
            CurrentInputState = output0;
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
            output(HIGH);
            output(LOW);
        }
        else {
            //Serial.println("Sending 0");
            // Send 0 in Manchester
            output(LOW);
            output(HIGH);
        }
    }
}

void outputThread() {
    while(true) {

        bool bitsToSend[] = {BIT0, BIT0, BIT1, BIT0, BIT1, BIT0};
        sendBitsManchester(bitsToSend, 6);
        Serial.println("---------");
        os_thread_delay_until(&lastThreadTime, 2000);
        
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