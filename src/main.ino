// #include "MessageManager.hpp"
// #include "BitManager.hpp"
// #include "GlobalVars.hpp"

//SYSTEM_THREAD(ENABLED);

bool isVerbose = true;

void setup() {
	Serial.begin(9600);

    Serial.println("STARTED PROGRAM");

    //BitManagerSetup();

    Serial.println("STARTED PROGRAM");

    waitFor(Serial.isConnected, 30000);
    Serial.println("Serial connected: starting");
}

void loop() {

    delay(5000);

    // uint8_t message1[4] = {0b01010101, 0b10101010, 0b00001111, 0b11110000};
    // uint8_t message1Size = 4;
    // sendMessage(message1, message1Size);

    char* message2 = "Hello World!";
    uint8_t message2Size = 13;
    //sendMessage((uint8_t*)message2, message2Size);

    delay(15000);

}
