
enum ManagerState { preambule, start, entete, message, controle, end };

struct frame {
    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000000;
    uint8_t message[6] = {0b00000000, 0b11111111, 0b01010101, 0b10101010, 0b00001111, 0b11110000};
    uint8_t crc16[2];
};

bool verbose = true;

class MessageManager {

private:
    frame sendingFrame;
    frame receivingFrame;

    bool isSending = false;

    // Transmission speed variables
    int transmissionSpeed = 0;
    int speedInterrupts = 0;
    unsigned long firstSpeedInterruptTime = 0;

    int bitCounter = 0;
    int byteCounter = 0;
    uint8_t byteConcat = 0;

public:
    ManagerState currentSendingState = preambule;
    ManagerState currentReceivingState = preambule;
    //MessageManager(bool sender): isSender(sender) {};

    bool* sendData() {

        bool* outputBitArray;
        uint8_t* byteArray;

        // Set at first/doesnt change (preamb, start, type/flag)
        byteArray[0] = sendingFrame.preambule; 
        byteArray[1] = sendingFrame.startEnd; 
        byteArray[2] = sendingFrame.typeFlag; 

        // Calculate message length
        uint8_t messageSize = sizeof(sendingFrame.message);
        byteArray[3] = messageSize; 

        // Add message byte per byte
        for(int i = 0; i < messageSize; i++)
            byteArray[i+4] = sendingFrame.message[i]; 
        
        // Calculate CRC

        //sendingFrame.crc16[0] = crc16Result & 0xFF
        //sendingFrame.crc16[1] = crc16Result & 0xFF00
        byteArray[messageSize-1+5] = sendingFrame.crc16[0]; 
        byteArray[messageSize-1+6] = sendingFrame.crc16[1]; 

        // Send end
        byteArray[messageSize-1+7] = sendingFrame.startEnd; 


       // Bytes to bits
       for (int i=0; i<sizeof(byteArray); i++){
           uint8_t bitMask = 0b10000000;
           for(int j=0; j < 8; j++){
               outputBitArray[i*8+j] = byteArray[i] & bitMask;
               bitMask >> 1;
           }
       }

       return outputBitArray;
    };

    void receiveBit(bool bitReceived){
        bitReceived++;
        byteConcat = (byteConcat << 1) | bitReceived;

        if (!(bitReceived%8)){
            receiveData(byteConcat);
        }
    };

    void receiveData(uint8_t byteReceived) {

        byteCounter++;

        switch(currentReceivingState){

            case preambule:
                if (verbose) {compareReadData("Preambule", &byteReceived, &sendingFrame.startEnd);}
                currentReceivingState = start;

            case start:
                
                receivingFrame.startEnd = byteReceived;
                if (verbose) {compareReadData("Start", &byteReceived, &sendingFrame.startEnd);}
                currentReceivingState = entete;

            case entete:

                if (byteCounter < 3){
                    if (verbose) {compareReadData("Entete (type+flags)", &byteReceived, &sendingFrame.typeFlag);}
                    receivingFrame.typeFlag = byteReceived;
                }
                else{
                    if (verbose) {compareReadData("Entete (length):", &byteReceived, &sendingFrame.messageLength);}
                    receivingFrame.messageLength = byteReceived;
                    currentReceivingState = message;
                }
                
            case message:

                receivingFrame.message[byteCounter-3] = byteReceived;

                if (sizeof(receivingFrame.message) >= receivingFrame.messageLength){
                    if (verbose) {compareReadData("Message", receivingFrame.message, sendingFrame.message);}
                    currentReceivingState = controle;
                }    
         
            case controle:

                // Gerer CRC16
                // receivingFrame.crc16[0] = CRCRESULT & 0xFF
                // receivingFrame.crc16[1] = CRCRESULT & 0xFF00
                if (verbose) {compareReadData("Controle", &byteReceived, sendingFrame.crc16);}
                currentReceivingState = end;

            case end:

                if (verbose) {compareReadData("End", &byteReceived, &sendingFrame.startEnd);}
                byteCounter = 0;
                currentReceivingState = start;

        }

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

    uint16_t crc16(const uint8_t* data_p, uint8_t length){
        unsigned char x;
        unsigned short crc = 0xFFFF;

        while (length--){
            x = crc >> 8 ^ *data_p++;
            x ^= x>>4;
            crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
        }
        return crc;
    }

    bool compareReadData(char* stage, uint8_t *bytesRead, uint8_t *byteCompare){

        int receivedSum = 0;
        int compareSum = 0;
        for (int i=0; i < sizeof(bytesRead); i++){
            receivedSum += bytesRead[i];
            compareSum += byteCompare[i];
        }

        bool isSameValue = receivedSum == compareSum;
        if (isSameValue)
            Serial.printlnf("SUCCES: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        else
            Serial.printlnf("ERROR: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);

        return isSameValue;
    };

};
