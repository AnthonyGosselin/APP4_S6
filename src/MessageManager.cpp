
enum ManagerState { preambule, start, entete, message, controle, end };
uint8_t preambuleByte = 0b01010101;
uint8_t startEndByte = 0b01111110;

class MessageManager {

private:
    bool isSender = false;
    ManagerState currentState = preambule;

    // Transmission speed variables
    int transmissionSpeed = 0;
    int speedInterrupts = 0;
    unsigned long firstSpeedInterruptTime = 0;

    int bitsRead = 0;
    uint8_t byteRecieved;

    uint8_t typeFlagByte = 0b00000000;      // To change depending on what we want 
    uint8_t messageLengthByte = 0b00000000;
    uint8_t* messageBytes;

public:
    MessageManager(bool sender): isSender(sender) {};

    void sendData() {
        switch(currentState){

            case preambule:
                sendManchester(&preambuleByte);
                currentState = start;

            case start:
                sendManchester(&startEndByte);
                currentState = entete;

            case entete:
                sendManchester(&typeFlagByte);

                uint8_t messageLength = sizeof(messageBytes);
                sendManchester(&messageLength);

                currentState = message;

            case message:
                sendManchester(messageBytes);
                currentState = controle;

            case controle:

                // PROCEDER au CRC16
                // uint8_t crc16Result = CRC 
                // sendManchester(crc16Result);
                currentState = end;

            case end:
                sendManchester(&startEndByte);
                // Destroy object? Or reset? Depending if we create new object all the time or repurpose after finished tramme
        }

        return;
    };

    void receiveData(int bitRead) {

        bitsRead++;
        int bytesRead = bitsRead / 8;
        int bitsLeftInByte = 8 - (bitsRead % 8);

        byteRecieved <<= bitRead;

        switch(currentState){

            case preambule:
                if (getTransmissionSpeed()){
                    currentState = start;
                }       

            case start:
                
                if (!bitsLeftInByte){
                    bool isCompareOk = compareReadData(&byteRecieved, &startEndByte);
                    currentState = entete;
                }

            case entete:

                if (!bitsLeftInByte){
                    if (bytesRead < 4){
                        typeFlagByte = byteRecieved;
                    }
                    else{
                        messageLengthByte = byteRecieved;
                        currentState = message;
                    }
                }
                
            case message:

                if (!bitsLeftInByte){
                    messageBytes[bytesRead-4] = byteRecieved;

                    if (bytesRead >= messageLengthByte + 4){
                    currentState = controle;
                    }
                }
                
         
            case controle:

                // Gerer CRC16
                currentState = end;

            case end:

                if (!bitsLeftInByte){
                    bool isCompareOk = compareReadData(&byteRecieved, &startEndByte);
                    // Destroy object? Or reset? Depending if we create new object all the time or repurpose after finished tramme
                }
  
        }

        return;
    };

    bool getTransmissionSpeed(){
        if (speedInterrupts < 7){
            if (speedInterrupts == 0)
                firstSpeedInterruptTime = millis();
            speedInterrupts++;
            return false;
        }
        else{
        // Get time since last interrupt
        unsigned long currentTime = millis();
        int elapsedTime = currentTime - firstSpeedInterruptTime;
        
        // Calculate speed by meaning
        transmissionSpeed = elapsedTime/7/2;

        speedInterrupts = 0;

        Serial.printlnf("%d", transmissionSpeed);

        return true;
        }
    };

    bool compareReadData(uint8_t *bytesRead, uint8_t *byteCompare){

    };

    // void readManchester(uint8_t *bytesToRead, uint8_t *byteCompare){

    // };

    void sendManchester(uint8_t *bytesToSend){

    };

};

// void MessageManager::processData(uint8_t *bytesToProcess){

    
// }

// void MessageManager::readData(uint8_t *bytesToRead){
    
// }

// MessageManager::MessageManager(bool sender){
//     MessageManager.isSender = sender;

// }