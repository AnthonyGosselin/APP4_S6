#include "Particle.h"

enum FrameManagerState { preambule, start, entete, message, controle, end };

struct frame {
    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000000;
    uint8_t* message;
    uint8_t crc16[2];
    bool crcCorrect = false;
};

class FrameManager {

private:
    bool isSending = false;
    bool isVerbose = true;

    // Transmission speed variables
    int transmissionSpeed = 0;
    int speedInterrupts = 0;
    unsigned long firstSpeedInterruptTime = 0;

    int bitCounter = 0;
    int byteCounter = 0;
    uint8_t byteConcat = 0;

public:
    frame sendingFrame;
    frame receivingFrame;

    FrameManagerState currentSendingState = preambule;
    FrameManagerState currentReceivingState = preambule;

    int transmissionSpeeds[3] = {1, 10, 100};   // Transmission speed per bit (in ms)

    bool* sendData(uint8_t* messageToSend) {

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
        sendingFrame.message = messageToSend;
        for(int i = 0; i < messageSize; i++)
            byteArray[i+4] = sendingFrame.message[i]; 
        
        // Calculate CRC
        uint16_t crc16Result = crc16(sendingFrame.message, (uint8_t)sizeof(sendingFrame.message));
        sendingFrame.crc16[0] = crc16Result & 0xFF;
        sendingFrame.crc16[1] = crc16Result & 0xFF00;
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
        bitCounter++;
        byteConcat = (byteConcat << 1) | bitReceived;

        if (!(bitCounter%8)){
            receiveData(byteConcat);
        }
    };

    void receiveData(uint8_t byteReceived) {

        byteCounter++;

        switch(currentReceivingState){

            case preambule:
                if (isVerbose) {compareReadData("Preambule", &byteReceived, &sendingFrame.preambule);}
                currentReceivingState = start;

            case start:
                
                receivingFrame.startEnd = byteReceived;
                if (isVerbose) {compareReadData("Start", &byteReceived, &sendingFrame.startEnd);}
                currentReceivingState = entete;

            case entete:

                if (byteCounter < 4){
                    if (isVerbose) {compareReadData("Entete (type+flags)", &byteReceived, &sendingFrame.typeFlag);}
                    receivingFrame.typeFlag = byteReceived;
                }
                else{
                    if (isVerbose) {compareReadData("Entete (length):", &byteReceived, &sendingFrame.messageLength);}
                    receivingFrame.messageLength = byteReceived;
                    currentReceivingState = message;
                }
                
            case message:

                receivingFrame.message[byteCounter-5] = byteReceived;

                if (sizeof(receivingFrame.message) >= receivingFrame.messageLength){
                    if (isVerbose) {compareReadData("Message", receivingFrame.message, sendingFrame.message);}
                    currentReceivingState = controle;
                }    
         
            case controle:

                // CRC first half
                if (byteCounter < sizeof(receivingFrame.message)+5){
                    receivingFrame.crc16[0] = byteReceived;
                }
                else{
                    // Gerer CRC16
                    receivingFrame.crc16[1] = byteReceived;
                    uint16_t fullCRC16 =  receivingFrame.crc16[0] << 16 | receivingFrame.crc16[1];
                    uint16_t crc16Result = crc16(sendingFrame.message, (uint8_t)sizeof(sendingFrame.message));

                    if(compareCRC16(crc16Result, fullCRC16))
                        receivingFrame.crcCorrect = true;

                    currentReceivingState = end;
                }
                

            case end:

                if (isVerbose) {compareReadData("End", &byteReceived, &sendingFrame.startEnd);}
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
    };

    bool compareCRC16(uint16_t crc16Result, uint16_t fullCRC16){
        bool isSameValue = crc16Result == fullCRC16;
        if (isSameValue)
            Serial.printlnf("CRC SUCCES: \t Calculated %d, Received %d.", crc16Result, fullCRC16);
        else
            Serial.printlnf("CRC ERROR: \t Calculated %d, Received %d.", crc16Result, fullCRC16);

        return isSameValue;
    };

    bool compareReadData(char* stage, uint8_t *bytesRead, uint8_t *bytesCompare){

        int receivedSum = 0;
        int compareSum = 0;
        for (int i=0; i < sizeof(bytesRead); i++){
            receivedSum += bytesRead[i];
            compareSum += bytesCompare[i];
        }

        bool isSameValue = receivedSum == compareSum;
        if (isSameValue)
            Serial.printlnf("SUCCES: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        else
            Serial.printlnf("ERROR: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);

        return isSameValue;
    };

};
