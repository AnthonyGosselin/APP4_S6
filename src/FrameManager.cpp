#include "GlobalVars.hpp"
#include "FrameManager.hpp"
#include "MessageManager.hpp"
#include "BitManager.hpp"


bool isSending = false;

int bitCounter = 0;
int byteCounter = 0;
uint8_t byteConcat = 0;

// frame sendingFrame;
// frame receivingFrame;

int currentSendingFrameObjIndex = 0;
int sendingFrameObjIndex = 0;
int receivingFrameObjIndex = 0;
Frame sendingFrameObjList[2];
Frame receivingFrameObjList[2];

Frame sendingFrameObj;
Frame receivingFrameObj;

FrameManagerState currentSendingState = preambule;
FrameManagerState currentReceivingState = preambule;

bool readyToSendFrame = false;


void sendDataFrame(uint8_t* messageToSend, uint8_t messageSize, bool isACK) {

    uint8_t *byteArray = new uint8_t[messageSize+7];
    sendingFrameObj = Frame();
    sendingFrameObj.isSender = true;

    // Set at first/doesnt change (preamb, start)
    byteArray[0] = sendingFrameObj.preambule; 
    byteArray[1] = sendingFrameObj.startEnd;

    // Add flag if is ACK to previous message
    if(isACK){
        sendingFrameObj.typeFlag = 0b00000001;
        byteArray[2] = sendingFrameObj.typeFlag; 
    }
    else{
        sendingFrameObj.typeFlag = 0b00000000;
        byteArray[2] = sendingFrameObj.typeFlag; 
    }
    
    // Get message length
    sendingFrameObj.messageLength = messageSize;
    byteArray[3] = sendingFrameObj.messageLength;

    // Setup arrays
    sendingFrameObj.setupArrays();

    // Add message byte per byte
    for(int i = 0; i < messageSize; i++){
        sendingFrameObj.message[messageToSend[i]];
        byteArray[i+4] = sendingFrameObj.message[i]; 
    }
        
    // Calculate CRC
    uint16_t crc16Result = crc16(sendingFrameObj.message, messageSize);
    sendingFrameObj.crc16[0] = (crc16Result & 0xFF00) >> 8;
    sendingFrameObj.crc16[1] = crc16Result & 0xFF;
    byteArray[messageSize-1+5] = sendingFrameObj.crc16[0]; 
    byteArray[messageSize-1+6] = sendingFrameObj.crc16[1]; 

    // Send end
    byteArray[messageSize-1+7] = sendingFrameObj.startEnd; 

    // Bytes to bits
    for (int i=0; i<messageSize+7; i++){
        uint8_t bitMask = 0b10000000;
        for(int j=0; j < 8; j++){
            sendingFrameObj.bitArray[i*8+j] = byteArray[i] & bitMask;
            bitMask >>= 1;
        }
    }


    delete[] byteArray;
    sendingFrameObjList[sendingFrameObjIndex] = sendingFrameObj;
    sendingFrameObjIndex++;
};

void receiveBit(uint8_t bitReceived){

    //Serial.printlnf("Received bit %d", (int)bitReceived);
    bitCounter++;
    byteConcat = (byteConcat << 1) | bitReceived;
    
    if (bitCounter >= 8){
        receiveData(byteConcat);
        bitCounter = 0;
    }
};

void receiveData(uint8_t byteReceived) {

    
    byteCounter++;
    Serial.printlnf("Received byte %d at spot %d", byteReceived, byteCounter);
    

    switch(currentReceivingState){

        case preambule:
            {   
                receivingFrameObj = Frame();

                const char stageName[] = "Preambule";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].preambule, 1);}
                currentReceivingState = start;
                break;
            }

        case start:
            {
                receivingFrameObjList[receivingFrameObjIndex].startEnd = byteReceived;
                const char stageName[] = "Start";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].startEnd, 1);}
                currentReceivingState = entete;
                break;
            }

        case entete:
            {
                if (byteCounter < 4){
                    const char stageName[] = "Entete (typeFlags)";
                    //Serial.printlnf("Stage: %s", stageName);
                    if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].typeFlag, 1);}
                    receivingFrameObjList[receivingFrameObjIndex].typeFlag = byteReceived;
                }
                else{
                    const char stageName[] = "Entete (length)";
                    //Serial.printlnf("Stage: %s", stageName);
                    if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].messageLength, 1);}
                    if (byteReceived > 80)
                        byteReceived = 80;
                    receivingFrameObj.messageLength = byteReceived;
                    receivingFrameObjList[receivingFrameObjIndex].setupArrays();
                    currentReceivingState = message;

                    // delete[] receivingFrame.message;
                    // receivingFrame.message = new uint8_t[receivingFrame.messageLength];

                }
                break;
            }
            
        case message:
            {
                const char stageName[] = "Message";
                receivingFrameObj.message[byteCounter-5] = byteReceived;
                //receivingFrameObj.message[byteReceived];

                if (byteCounter < receivingFrameObj.messageLength + 4){
                    if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[byteCounter-5], &sendingFrameObjList[receivingFrameObjIndex].message[byteCounter-5], 1);}
                }
                else{
                    if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[byteCounter-5], &sendingFrameObjList[receivingFrameObjIndex].message[byteCounter-5], 1);}
                    if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[0], &sendingFrameObjList[receivingFrameObjIndex].message[0], receivingFrameObj.messageLength);}
                    currentReceivingState = controle;
                }
                break;  
            }  
        
        case controle:
            {
                const char stageName[] = "CRC";

                // CRC first half
                if (byteCounter < receivingFrameObj.messageLength + 6){
                    receivingFrameObj.crc16[0] = byteReceived;
                    if (isVerbose) {compareReadData(stageName, &receivingFrameObj.crc16[0], &sendingFrameObjList[receivingFrameObjIndex].crc16[0], 1);}
                }
                else{
                    // Gerer CRC16
                    receivingFrameObj.crc16[1] = byteReceived;
                    if (isVerbose) {compareReadData(stageName, &receivingFrameObj.crc16[1], &sendingFrameObjList[receivingFrameObjIndex].crc16[1], 1);}
                    uint16_t fullCRC16 =  receivingFrameObj.crc16[0] << 8 | receivingFrameObj.crc16[1];
                    uint16_t crc16Result = crc16(&receivingFrameObj.message[0], receivingFrameObj.messageLength);

                    if(compareCRC16(crc16Result, fullCRC16))
                        receivingFrameObj.crcCorrect = true;

                    currentReceivingState = end;
                }
                break;
            }
            

        case end:
            {
                const char stageName[] = "End";
                //Serial.printlnf("Stage: %s", stageName);
                if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].startEnd, 1);}
                byteCounter = 0;
                currentReceivingState = preambule;

                receivingFrameObjList[receivingFrameObjIndex] = receivingFrameObj;

                //if(receivingFrame.crcCorrect)
                receiveMessage(&receivingFrameObjList[receivingFrameObjIndex].message[0]);
                // else{
                //     char* errorMsg = "CRC Error: message flushed.";
                //     receiveMessage((uint8_t*)errorMsg);
                // }

                receivingFrameObjIndex++;

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
            Serial.printlnf("CRC SUCCESS: \t Calculated %d, Received %d.", crc16Result, fullCRC16);
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
            Serial.printlnf("SUCCESS: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        }
    }
    else{
        WITH_LOCK(Serial){
            Serial.printlnf("ERROR: Received for %s: \t Expected %d, Received %d.", stage, compareSum, receivedSum);
        }
    }

    return isSameValue;
};
