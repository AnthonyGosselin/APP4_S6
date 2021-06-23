#pragma once
#include "MessageManager.ino"

SYSTEM_THREAD(ENABLED);

bool isVerbose = true;

void setup() {
	Serial.begin(9600);

    BitManagerSetup();

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");
}

void loop() {

    delay(5000);
    uint8_t message1[4] = {0b01010101, 0b10101010, 0b00001111, 0b11110000};
    uint8_t messageSize = 4;
    sendMessage(message1, messageSize);


	// Do we really need to make a thread if we are not using main thread (do interrupts run on main thread??)

    // Call messages to send here, split by long delays?
    // char* message1 = [H, e, l, l, o, , W, o, r, l, d, !];
    // ex. msgManager.sendMessage(message1);
    // delay(10000); OR os_thread_delay_until(&lastMessageTime, 10000)
}
