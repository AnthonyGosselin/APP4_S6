#include "Particle.h"

SYSTEM_THREAD(ENABLED);

void threadFunction(void);

Thread thread("testThread", threadFunction);

volatile int counter = 0;
unsigned long lastReport = 0;

void setup() {
	Serial.begin(9600);
}

void loop() {
	if (millis() - lastReport >= 1000) {
		lastReport = millis();

		Serial.printlnf("counter=%d", counter);
	}
}


// void threadFunction(void) {
// 	while(true) {
// 		counter++;
// 	}
// 	// You must not return from the thread function
// }

void threadFunction(void) {
	while(true) {
		counter++;

		os_thread_yield();
	}
	// You must not return from the thread function
}