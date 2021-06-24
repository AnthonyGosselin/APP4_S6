#include "MessageManager.hpp"
#include "BitManager.hpp"
#include "GlobalVars.hpp"

SYSTEM_THREAD(ENABLED);

bool isVerbose = false;
bool insertBitError = false;

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
        os_thread_delay_until(&messageTime, 10000);
        Serial.println("\n\nLoop");

        // uint8_t message1[4] = {1, 0, 1, 0}; // :)
        // uint8_t message1Size = 4;
        // sendMessage(message1, message1Size);

        char* message2 = "Hello, World!";
        uint8_t message2Size = 14;
        sendMessage((uint8_t*)message2, message2Size, false);
    }
}
