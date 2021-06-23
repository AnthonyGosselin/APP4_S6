/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/GitAPP/APP4_S6/src/main.ino"
#include "MessageManager.hpp"
#include "BitManager.hpp"
#include "GlobalVars.hpp"

void setup();
void loop();
#line 5 "c:/GitAPP/APP4_S6/src/main.ino"
SYSTEM_THREAD(ENABLED);

bool isVerbose = true;

void messageThread();

void setup() {
	Serial.begin(9600);

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");

    Serial.println("STARTED PROGRAM");

    Thread thread("messageThread", messageThread);

    BitManagerSetup();

    Serial.println("STARTED PROGRAM");  
}

void loop() {

}

void messageThread() {
    while (true) {
        delay(5000);
        Serial.println("Loop");

        uint8_t message1[4] = {0b01010101, 0b10101010, 0b00001111, 0b11110000};
        uint8_t message1Size = 4;
        sendMessage(message1, message1Size);

        //char* message2 = "Hello World!";
        // uint8_t message2Size = 13;
        // sendMessage((uint8_t*)message2, message2Size);

        delay(500000);
    }
}
