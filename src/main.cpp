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

bool isVerbose = false;

system_tick_t messageTime = 0;

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
        
        os_thread_delay_until(&messageTime, 2000);
        Serial.println("\n\nLoop");

        // uint8_t message1[4] = {1, 0, 1, 0}; // :)
        // uint8_t message1Size = 4;
        // sendMessage(message1, message1Size, false);

        char* message2 = "Hello, World!";
        uint8_t message2Size = 14;
        sendMessage((uint8_t*)message2, message2Size, false);

        os_thread_delay_until(&messageTime, 10000);

        // char* message3 = "Hello! My name is Etienne. Nice to meet you! I like microwaved my pop tarts!";
        // uint8_t message3Size = 79;
        // sendMessage((uint8_t*)message3, message3Size, false);

        char* message3 = "Hello! My name is Etienne. Nice to meet you!";
        uint8_t message3Size = 79;
        sendMessage((uint8_t*)message3, message3Size, false);

        os_thread_delay_until(&messageTime, 10000);
    }
}
