/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/GitAPP/APP4_S6/src/MessageManager.ino"
#include "FrameManager.ino"

void sendMessage(uint8_t* messageToSend);
void receiveMessage(uint8_t* messageReceived);
bool compareReadMessage(bool isString, uint8_t *bytesRead, uint8_t *byteCompare, uint8_t length);
#line 3 "c:/GitAPP/APP4_S6/src/MessageManager.ino"
class MessageManager {

public:
    FrameManager frameManager = FrameManager();

private:

    uint8_t* lastMessageSent;
    bool isVerbose = true;

public:

    void sendMessage(uint8_t* messageToSend) {
        lastMessageSent = messageToSend;
        frameManager.sendData(messageToSend);
    };

    void receiveMessage(uint8_t* messageReceived) {
        // Have to find way to poll / get message at transmission end
        uint8_t* receivedMessage = frameManager.receivingFrame.message;
        compareReadMessage(true, receivedMessage, lastMessageSent, frameManager.receivingFrame.messageLength);
    };


    bool compareReadMessage(bool isString, uint8_t *bytesRead, uint8_t *byteCompare, uint8_t length){

        bool isSame;
        if (isString){
            char* msgRead = (char*)bytesRead;
            char* msgCompare = (char*)byteCompare;

            isSame = !strcmp(msgRead, msgCompare);
            // if (isSame)
            //     Serial.printlnf("SUCCES: Received message: \t %s.", msgRead);
            // else
            //     Serial.printlnf("ERROR: Received message: \t Expected %s, Received %s.", msgCompare, msgRead);
        }
        else{
            unsigned long receivedSum = 0;
            unsigned long compareSum = 0;
            for (int i=0; i < length; i++){
                receivedSum += bytesRead[i];
                compareSum += byteCompare[i];
            }

            isSame = receivedSum == compareSum;
            // if (isSame)
            //     Serial.printlnf("SUCCES: Received message: \t Expected %d, Received %d.", compareSum, receivedSum);
            // else
            //     Serial.printlnf("ERROR: Received message: \t Expected %d, Received %d.", compareSum, receivedSum);
        }
        
        return isSame;

    };

};
