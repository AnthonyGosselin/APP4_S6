#include "MessageManager.hpp"
#include "FrameManager.hpp"
#include "GlobalVars.hpp"

uint8_t* lastMessageSent;
// uint8_t* lastMessageSize;
// uint8_t* lastMessageACKFlag;

void sendMessage(uint8_t* messageToSend, uint8_t messageSize, uint8_t ACKFlag) {
    // If first message, dont check invalid list or ACK
    if(sendingFrameObjIndex == 0){
        Serial.println("Sending first frame");
        sendDataFrame(messageToSend, messageSize, ACKFlag);
        lastMessageSent = messageToSend;
    }
    else{
        // If not first message, check if last message ACK was received
        if(sendingFrameObjList[sendingFrameObjIndex-1].receivedACK){
            // If message ACK received, go on with next message
            if (ACKFlag == 0b00000000){
                lastMessageSent = messageToSend;
            }
            sendDataFrame(messageToSend, messageSize, ACKFlag);
        }
        else{
            // If ACK not received, lower transmission speed and retry
            sendingFrameObjList[sendingFrameObjIndex-1].frameOutputSpeed += 2;
            resendDataFrame();
        }
    }
    
};

void receiveMessage(uint8_t* messageReceived, uint8_t typeFlag) {
    
    if(compareReadMessage(true, messageReceived, sendingFrameObjList[receivingFrameObjIndex].message, receivingFrameObjList[receivingFrameObjIndex].messageLength)){
        // Send back ACK after message received if message is not an ACK
        //Serial.printlnf("Typeflag %d", typeFlag);
        if(typeFlag == 0b00000000){
            Serial.printlnf("Sending ACK for message %d", typeFlag);
            char* ACKMessage = "ACK";
            uint8_t ACKMessageSize = 4;
            sendDataFrame((uint8_t*)ACKMessage, ACKMessageSize, (uint8_t)receivingFrameObjIndex);
        }
        else{   // Do x if ACK received
            Serial.printlnf("ACK received for message %d", typeFlag);
            sendingFrameObjList[typeFlag].receivedACK = true;
        }
    }

};

bool compareReadMessage(bool isString, uint8_t *bytesRead, uint8_t *byteCompare, uint8_t length){

    bool isSame;
    if (isString){
        char* msgRead = (char*)bytesRead;
        char* msgCompare = (char*)byteCompare;

        isSame = !strcmp(msgRead, msgCompare);
        WITH_LOCK(Serial){
            Serial.printlnf("Received message: \t \"%s\".", msgRead);
        }
    }
    else{
        unsigned long receivedSum = 0;
        unsigned long compareSum = 0;
        for (int i=0; i < length; i++){
            receivedSum += bytesRead[i];
            compareSum += byteCompare[i];
        }

        isSame = receivedSum == compareSum;
        WITH_LOCK(Serial){
            Serial.printlnf("Received message: \t Received %lu.", receivedSum);
        }
    }
    
    return isSame;

};
