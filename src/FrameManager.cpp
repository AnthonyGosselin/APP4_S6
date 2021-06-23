#include "GlobalVars.hpp"
#include "FrameManager.hpp"
#include "MessageManager.hpp"
#include "BitManager.hpp"


bool isSending = false;

int bitCounter = 0;
int byteCounter = 0;
uint8_t byteConcat = 0;

frame sendingFrame;
frame receivingFrame;

FrameManagerState currentSendingState = preambule;
FrameManagerState currentReceivingState = preambule;

bool readyToSendFrame = false;
bool *bitArray = new bool[1];
uint8_t bitArraySize;

bool crcCorrect = false;

void sendDataFrame(uint8_t* messageToSend, uint8_t messageSize, bool isACK) {

    uint8_t *byteArray = new uint8_t[messageSize+7];
    delete[] bitArray;
    bitArray = new bool[(messageSize+7)*8];
    bitArraySize = 0;

    // Set at first/doesnt change (preamb, start)
    byteArray[0] = sendingFrame.preambule; 
    byteArray[1] = sendingFrame.startEnd;

    // Add flag if is ACK to previous message
    if(isACK){
        byteArray[2] = 0b00000001; 
    }
    else{
        byteArray[2] = 0b00000000;
    }
    
    // Get message length
    sendingFrame.messageLength = messageSize;
    byteArray[3] = sendingFrame.messageLength;
    //Serial.printlnf("Message length: %d, %d, %d", messageSize, sendingFrame.messageLength, byteArray[3]);

    // Add message byte per byte
    for(int i = 0; i < messageSize; i++){
        sendingFrame.message[i] = messageToSend[i];
        byteArray[i+4] = sendingFrame.message[i]; 
    }
        
    // Calculate CRC
    uint16_t crc16Result = crc16(sendingFrame.message, messageSize);
    sendingFrame.crc16[0] = (crc16Result & 0xFF00) >> 8;
    sendingFrame.crc16[1] = crc16Result & 0xFF;
    byteArray[messageSize-1+5] = sendingFrame.crc16[0]; 
    byteArray[messageSize-1+6] = sendingFrame.crc16[1]; 

    // Send end
    byteArray[messageSize-1+7] = sendingFrame.startEnd; 

    // Bytes to bits
    for (int i=0; i<messageSize+7; i++){
        uint8_t bitMask = 0b10000000;
        for(int j=0; j < 8; j++){
            bitArray[i*8+j] = byteArray[i] & bitMask;
            bitMask >>= 1;
            bitArraySize++;
        }
    }

    delete[] byteArray;
    readyToSendFrame = true;
};

void receiveBit(uint8_t bitReceived){

    //Serial.printlnf("Received bit %d", (int)bitReceived);
    bitCounter++;
    byteConcat = (byteConcat << 1) | bitReceived;
    //Serial.printlnf("Received bit number %d", bitCounter);
    //Serial.printlnf("Received concat byte %d", (int)byteConcat);
    
    if (!(bitCounter%8)){
        receiveData(byteConcat);
    }
};

void receiveData(uint8_t byteReceived) {

    //Serial.printlnf("Received byte %d", (int)byteReceived);
    byteCounter++;

    switch(currentReceivingState){

        case preambule:
            {
                const char stageName[] = "Preambule";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrame.preambule, 1);}
                currentReceivingState = start;
                break;
            }

        case start:
            {
                receivingFrame.startEnd = byteReceived;
                const char stageName[] = "Start";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrame.startEnd, 1);}
                currentReceivingState = entete;
                break;
            }

        case entete:
            {
                if (byteCounter < 4){
                    const char stageName[] = "Entete (typeFlags)";
                    //Serial.printlnf("Stage: %s", stageName);
                    if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrame.typeFlag, 1);}
                    receivingFrame.typeFlag = byteReceived;
                }
                else{
                    const char stageName[] = "Entete (length)";
                    //Serial.printlnf("Stage: %s", stageName);
                    if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrame.messageLength, 1);}
                    receivingFrame.messageLength = byteReceived;
                    currentReceivingState = message;

                    // delete[] receivingFrame.message;
                    // receivingFrame.message = new uint8_t[receivingFrame.messageLength];

                }
                break;
            }
            
        case message:
            {
                const char stageName[] = "Message";
                receivingFrame.message[byteCounter-5] = byteReceived;

                if (byteCounter < receivingFrame.messageLength + 4){
                    if (isVerbose) {compareReadData(stageName, &receivingFrame.message[byteCounter-5], &sendingFrame.message[byteCounter-5], 1);}
                }
                else{
                    if (isVerbose) {compareReadData(stageName, &receivingFrame.message[byteCounter-5], &sendingFrame.message[byteCounter-5], 1);}
                    if (isVerbose) {compareReadData(stageName, receivingFrame.message, sendingFrame.message, receivingFrame.messageLength);}
                    currentReceivingState = controle;
                }
                break;  
            }  
        
        case controle:
            {
                const char stageName[] = "CRC";

                // CRC first half
                if (byteCounter < receivingFrame.messageLength + 6){
                    receivingFrame.crc16[0] = byteReceived;
                    if (isVerbose) {compareReadData(stageName, &receivingFrame.crc16[0], &sendingFrame.crc16[0], 1);}
                }
                else{
                    // Gerer CRC16
                    receivingFrame.crc16[1] = byteReceived;
                    if (isVerbose) {compareReadData(stageName, &receivingFrame.crc16[1], &sendingFrame.crc16[1], 1);}
                    uint16_t fullCRC16 =  receivingFrame.crc16[0] << 8 | receivingFrame.crc16[1];
                    uint16_t crc16Result = crc16(receivingFrame.message, receivingFrame.messageLength);

                    if(compareCRC16(crc16Result, fullCRC16))
                        crcCorrect = true;

                    currentReceivingState = end;
                }
                break;
            }
            

        case end:
            {
                const char stageName[] = "End";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrame.startEnd, 1);}
                byteCounter = 0;
                currentReceivingState = preambule;

                //if(receivingFrame.crcCorrect)
                receiveMessage(receivingFrame.message);
                // else{
                //     char* errorMsg = "CRC Error: message flushed.";
                //     receiveMessage((uint8_t*)errorMsg);
                // }

                break;
            }

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
    if (isSameValue){
        WITH_LOCK(Serial){
            Serial.printlnf("CRC SUCCES: \t Calculated %d, Received %d.", crc16Result, fullCRC16);
        }
    }
    else{
        WITH_LOCK(Serial){
            Serial.printlnf("CRC ERROR: \t Calculated %d, Received %d.", crc16Result, fullCRC16);
        }
    }
        
    return isSameValue;
};

bool compareReadData(const char* stage, uint8_t* bytesRead, uint8_t* bytesCompare, int length){

    int receivedSum = 0;
    int compareSum = 0;
    for (int i=0; i < length; i++){
        receivedSum += bytesRead[i];
        compareSum += bytesCompare[i];
    }

    bool isSameValue = receivedSum == compareSum;
    if (isSameValue){
        WITH_LOCK(Serial){
            Serial.printlnf("SUCCES: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        }
    }
    else{
        WITH_LOCK(Serial){
            Serial.printlnf("ERROR: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        }
    }

    return isSameValue;
};
