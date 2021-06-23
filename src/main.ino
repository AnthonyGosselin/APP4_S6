#include "MessageManager.hpp"
#include "BitManager.hpp"
#include "GlobalVars.hpp"

SYSTEM_THREAD(ENABLED);

bool isVerbose = false;

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
        Serial.println("\n\nLoop: sending new message");

        // uint8_t message1[4] = {1, 0, 1, 0};
        // uint8_t message1Size = 4;
        // sendMessage(message1, message1Size);

        char* message2 = "Hello World!";
        uint8_t message2Size = 13;
        sendMessage((uint8_t*)message2, message2Size);
    }
}
