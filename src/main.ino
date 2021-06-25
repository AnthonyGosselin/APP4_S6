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
        
        os_thread_delay_until(&messageTime, 2000);

        char* message2 = "Hello, Mars!";
        uint8_t message2Size = 13;
        sendMessage((uint8_t*)message2, message2Size, false);

        os_thread_delay_until(&messageTime, 12000);

        char* message3 = "Hello, World!";
        uint8_t message3Size = 14;
        sendMessage((uint8_t*)message3, message3Size, false);

        os_thread_delay_until(&messageTime, 10000);
    }
}
