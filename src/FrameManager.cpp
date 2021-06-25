#include "GlobalVars.hpp"
#include "FrameManager.hpp"
#include "MessageManager.hpp"
#include "BitManager.hpp"


bool isSending = false;

int bitCounter = 0;
int byteCounter = 0;
uint8_t byteConcat = 0;

int currentSendingFrameObjIndex = 0;
int sendingFrameObjIndex = 0;
int receivingFrameObjIndex = -1;
Frame sendingFrameObjList[25];
Frame receivingFrameObjList[25];

Frame sendingFrameObj;
Frame receivingFrameObj;

//FrameManagerState currentSendingState = preambule;
//FrameManagerState currentReceivingState = preambule;

bool readyToSendFrame = false;

//uint8_t currentByteBuffer[87];


void sendDataFrame(uint8_t* messageToSend, uint8_t messageSize, uint8_t ACKFlag) {

    uint8_t *byteArray = new uint8_t[messageSize+7];
    Serial.printlnf("RECEIVED DATA SIZE %d", messageSize);
    sendingFrameObj = Frame();

    // Set ACK messages received ACK to true to not loop ACKs to ACK messages
    if(ACKFlag != 0b00000000){
        sendingFrameObj.receivedACK = true;
    }

    // Set at first/doesnt change (preamb, start)
    byteArray[0] = sendingFrameObj.preambule; 
    byteArray[1] = sendingFrameObj.start;

    // Add flag if is ACK to previous message
    sendingFrameObj.typeFlag = ACKFlag;
    byteArray[2] = sendingFrameObj.typeFlag; 

    // Get message length
    sendingFrameObj.messageLength = messageSize;
    byteArray[3] = sendingFrameObj.messageLength;

    // Setup arrays
    sendingFrameObj.setupArrays();

    // Add message byte per byte
    for(int i = 0; i < messageSize; i++){
        sendingFrameObj.message[i] = messageToSend[i];
        byteArray[i+4] = sendingFrameObj.message[i]; 
    }
        
    // Calculate CRC
    uint16_t crc16Result = crc16(sendingFrameObj.message, messageSize);
    sendingFrameObj.crc16[0] = (crc16Result & 0xFF00) >> 8;
    sendingFrameObj.crc16[1] = crc16Result & 0xFF;
    byteArray[messageSize-1+5] = sendingFrameObj.crc16[0]; 
    byteArray[messageSize-1+6] = sendingFrameObj.crc16[1]; 

    // Send end
    byteArray[messageSize-1+7] = sendingFrameObj.end; 

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

void resendDataFrame() {
    // If ACK not received by the time we get new message to send, 
    sendingFrameObj.frameOutputSpeed += 2;
    currentSendingFrameObjIndex--;
};

void resetCounters(){
    //currentReceivingState = preambule;
    bitCounter = 0;
    byteCounter = 0;
}

void receiveBit(uint8_t bitReceived, bool isFullByte){

    Serial.printlnf("Received %d", bitReceived);

    if(bitCounter == 0){
        receivingFrameObj = Frame();
    }

    //Serial.printlnf("Received bit %d", (int)bitReceived);
    if (isFullByte){
        bitCounter = 8;
        byteConcat = bitReceived;
    }
    else{
        bitCounter++;
        byteConcat = (byteConcat << 1) | bitReceived;
    }
    
    if (!(bitCounter % 8)){
        //receiveData(byteConcat);
        receivingFrameObj.byteArray[byteCounter] = byteConcat;

        if(byteCounter == 3){
            //Serial.printlnf("Message Length: %d", byteConcat);
            receivingFrameObj.messageLength = byteConcat;
        }

        if(byteCounter >= receivingFrameObj.messageLength-1+7){
            Serial.println("Ending transmission1");
            endTransmission();
        }
        //Serial.printlnf("Byte Counter: %d", byteCounter);
        byteCounter++;
    }
};


void endTransmission(){

    Serial.println("Ending transmission2");

    //receivingFrameObj.spliceByteArray();

    uint16_t fullCRC16 =  receivingFrameObj.crc16[0] << 8 | receivingFrameObj.crc16[1];
    uint16_t crc16Result = crc16(&receivingFrameObj.message[0], receivingFrameObj.messageLength);

    if(compareCRC16(crc16Result, fullCRC16))
        receivingFrameObj.crcCorrect = true;

    receivingFrameObjList[receivingFrameObjIndex] = receivingFrameObj;
    receivingFrameObjIndex++;

    receiveMessage(receivingFrameObj.message, receivingFrameObj.typeFlag);

}

// void receiveData(uint8_t byteReceived) {

//     byteCounter++;
//     //Serial.printlnf("Received byte %d at spot %d", byteReceived, byteCounter);
    
//     switch(currentReceivingState){

//         case preambule:
//             {   
//                 receivingFrameObjIndex++;
//                 receivingFrameObj = Frame();

//                 const char stageName[] = "Preambule";
//                 //Serial.printlnf("Stage: %s", stageName);
//                 if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].preambule, 1);}
//                 currentReceivingState = start;
//                 break;
//             }

//         case start:
//             {
//                 receivingFrameObjList[receivingFrameObjIndex].start = byteReceived;
//                 const char stageName[] = "Start";
//                 //Serial.printlnf("Stage: %s", stageName);
//                 if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].start, 1);}
//                 currentReceivingState = entete;
//                 break;
//             }

//         case entete:
//             {
//                 if (byteCounter < 4){
//                     const char stageName[] = "Entete (typeFlags)";
//                     //Serial.printlnf("Stage: %s", stageName);
//                     if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].typeFlag, 1);}
//                     receivingFrameObjList[receivingFrameObjIndex].typeFlag = byteReceived;
//                 }
//                 else{
//                     const char stageName[] = "Entete (length)";
//                     //Serial.printlnf("Stage: %s", stageName);
//                     if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].messageLength, 1);}
//                     if (byteReceived > 80)
//                         byteReceived = 80;
//                     receivingFrameObj.messageLength = byteReceived;
//                     //receivingFrameObjList[receivingFrameObjIndex].setupArrays();
//                     currentReceivingState = message;

//                     // delete[] receivingFrame.message;
//                     // receivingFrame.message = new uint8_t[receivingFrame.messageLength];

//                 }
//                 break;
//             }
            
//         case message:
//             {
//                 const char stageName[] = "Message";
//                 receivingFrameObj.message[byteCounter-5] = byteReceived;
//                 //receivingFrameObj.message[byteReceived];

//                 if (byteCounter < receivingFrameObj.messageLength + 4){
//                     if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[byteCounter-5], &sendingFrameObjList[receivingFrameObjIndex].message[byteCounter-5], 1);}
//                 }
//                 else{
//                     if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[byteCounter-5], &sendingFrameObjList[receivingFrameObjIndex].message[byteCounter-5], 1);}
//                     if (isVerbose) {compareReadData(stageName, &receivingFrameObj.message[0], &sendingFrameObjList[receivingFrameObjIndex].message[0], receivingFrameObj.messageLength);}
//                     currentReceivingState = controle;
//                 }
//                 break;  
//             }  
        
//         case controle:
//             {
//                 const char stageName[] = "CRC";

//                 // CRC first half
//                 if (byteCounter < receivingFrameObj.messageLength + 6){
//                     receivingFrameObj.crc16[0] = byteReceived;
//                     if (isVerbose) {compareReadData(stageName, &receivingFrameObj.crc16[0], &sendingFrameObjList[receivingFrameObjIndex].crc16[0], 1);}
//                 }
//                 else{
//                     // Gerer CRC16
//                     receivingFrameObj.crc16[1] = byteReceived;
//                     if (isVerbose) {compareReadData(stageName, &receivingFrameObj.crc16[1], &sendingFrameObjList[receivingFrameObjIndex].crc16[1], 1);}

//                     // uint16_t fullCRC16 =  receivingFrameObj.crc16[0] << 8 | receivingFrameObj.crc16[1];
//                     // uint16_t crc16Result = crc16(&receivingFrameObj.message[0], receivingFrameObj.messageLength);

//                     // if(compareCRC16(crc16Result, fullCRC16))
//                     //     receivingFrameObj.crcCorrect = true;

//                     currentReceivingState = end;
//                 }
//                 break;
//             }
            

//         case end:
//             {
//                 const char stageName[] = "End";
//                 //Serial.printlnf("Stage: %s", stageName);
//                 if (isVerbose) {compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].end, 1);}
//                 byteCounter = 0;
//                 currentReceivingState = preambule;

//                 calcCRCNSend();
//                 break;
//             }

//     }

// };

// Calculate CRC seperately after reading
void calcCRCNSend(){
    uint16_t fullCRC16 =  receivingFrameObj.crc16[0] << 8 | receivingFrameObj.crc16[1];
    uint16_t crc16Result = crc16(&receivingFrameObj.message[0], receivingFrameObj.messageLength);

    if(compareCRC16(crc16Result, fullCRC16))
        receivingFrameObj.crcCorrect = true;

    receivingFrameObjList[receivingFrameObjIndex] = receivingFrameObj;

    receiveMessage(&receivingFrameObjList[receivingFrameObjIndex].message[0], receivingFrameObjList[receivingFrameObjIndex].typeFlag);
}

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
