#include "MessageManager.hpp"
#include "FrameManager.hpp"
#include "GlobalVars.hpp"

uint8_t* lastMessageSent;

void sendMessage(uint8_t* messageToSend, uint8_t messageSize, bool isACK) {
    if (!isACK){
        lastMessageSent = messageToSend;
    }
    sendDataFrame(messageToSend, messageSize, isACK);
    
};

void receiveMessage(uint8_t* messageReceived) {
    compareReadMessage(true, messageReceived, lastMessageSent, receivingFrame.messageLength);
    // WITH_LOCK(Serial){
    //     Serial.printlnf("Received message: \t Expected \"%s\", Received \"%s\".", lastMessageSent, messageReceived);
    // }
};

bool compareReadMessage(bool isString, uint8_t *bytesRead, uint8_t *byteCompare, uint8_t length){

    bool isSame;
    if (isString){
        char* msgRead = (char*)bytesRead;
        char* msgCompare = (char*)byteCompare;

        isSame = !strcmp(msgRead, msgCompare);
        if (isSame){
            WITH_LOCK(Serial){
                Serial.printlnf("SUCCESS: Received message: \t \"%s\".", msgRead);
            }
        }
        else{
            WITH_LOCK(Serial){
                Serial.printlnf("ERROR: Received message: \t Expected \"%s\", Received \"%s\".", msgCompare, msgRead);
            }
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
        if (isSame){
            WITH_LOCK(Serial){
                Serial.printlnf("SUCCESS: Received message: \t Expected %lu, Received %lu.", compareSum, receivedSum);
            }
        }
        else{
            WITH_LOCK(Serial){
                Serial.printlnf("ERROR: Received message: \t Expected %lu, Received %lu.", compareSum, receivedSum);
            }
        }
    }
    
    return isSame;

};
