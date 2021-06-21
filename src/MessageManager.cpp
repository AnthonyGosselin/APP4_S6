
enum ManagerState { preambule, start, entete, message, controle, end };

struct tramme {
    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000000;
    uint8_t* message;
    uint8_t crc16;
};

bool verbose = true;

class MessageManager {

private:
    tramme sendingTramme;
    tramme receivingTramme;

    ManagerState currentSendingState = preambule;
    ManagerState currentReceivingState = preambule;

    bool isSending = false;

    // Transmission speed variables
    int transmissionSpeed = 0;
    int speedInterrupts = 0;
    unsigned long firstSpeedInterruptTime = 0;

    int byteCounter = 0;

public:
    //MessageManager(bool sender): isSender(sender) {};

    void sendData() {
        switch(currentSendingState){

            case preambule:
                isSending = true;
                sendManchester(&sendingTramme.preambule);
                currentSendingState = start;

            case start:
                sendManchester(&sendingTramme.startEnd);
                currentSendingState = entete;

            case entete:
                sendManchester(&sendingTramme.typeFlag);

                uint8_t messageLength = sizeof(&sendingTramme.message);
                sendManchester(&messageLength);

                currentSendingState = message;

            case message:
                sendManchester(sendingTramme.message);
                currentSendingState = controle;

            case controle:

                // PROCEDER au CRC16
                // sendingTramme.crc16 = CRC 
                // sendManchester(crc16Result);
                currentSendingState = end;

            case end:
                sendManchester(&sendingTramme.startEnd);
                // Destroy object? Or reset? Depending if we create new object all the time or repurpose after finished tramme
                isSending = false;
                currentSendingState = preambule;
        }

        return;
    };

    void receiveData(uint8_t byteReceived) {

        byteCounter++;
        // int bytesRead = bitsRead / 8;
        // int bitsLeftInByte = 8 - (bitsRead % 8);

        switch(currentReceivingState){

            // case preambule:
            //     if (getTransmissionSpeed()){
            //         currentReceivingState = start;
            //     }       

            case start:
                
                receivingTramme.startEnd = byteReceived;
                if (verbose) {compareReadData(&byteReceived, &sendingTramme.startEnd);}
                currentReceivingState = entete;

            case entete:

                if (byteCounter < 3){
                    if (verbose) {compareReadData(&byteReceived, &sendingTramme.typeFlag);}
                    receivingTramme.typeFlag = byteReceived;
                }
                else{
                    if (verbose) {compareReadData(&byteReceived, &sendingTramme.messageLength);}
                    receivingTramme.messageLength = byteReceived;
                    currentReceivingState = message;
                }
                
            case message:

                receivingTramme.message[byteCounter-3] = byteReceived;

                if (sizeof(receivingTramme.message) >= receivingTramme.messageLength){
                    if (verbose) {compareReadData(receivingTramme.message, sendingTramme.message);}
                    currentReceivingState = controle;
                }    
         
            case controle:

                // Gerer CRC16
                // receivingTramme.crc16 = CRCRESULT
                if (verbose) {compareReadData(&byteReceived, &sendingTramme.crc16);}
                currentReceivingState = end;

            case end:

                if (verbose) {compareReadData(&byteReceived, &sendingTramme.startEnd);}
                byteCounter = 0;
                currentReceivingState = start;
  
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

        // compare and print to serial
    };

    void sendManchester(uint8_t *bytesToSend){

    };

};
