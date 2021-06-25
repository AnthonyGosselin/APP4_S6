#pragma once
#include <stdint.h>

// enum FrameManagerState { preambule, start, entete, message, controle, end };

extern uint8_t currentByteBuffer[87];

class Frame{
public:
    uint8_t preambule = 0b01010101;
    uint8_t start = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000001;
    uint8_t message[80];
    uint8_t crc16[2];
    uint8_t end = 0b01111110;
    bool crcCorrect = false;

    uint8_t byteArray[87];
    bool* bitArray = nullptr;
    int bitArraySize;

    ~Frame(){
        if(bitArray != nullptr)
            delete[] bitArray;
    };

    void setupArrays(){

        bitArraySize = (messageLength + 7) * 8;
        bitArray = new bool[bitArraySize];
    }

    void spliceByteArray(){
        preambule = currentByteBuffer[0];
        start = currentByteBuffer[1];
        typeFlag = currentByteBuffer[2];
        //messageLength = byteArray[3];

        for(int i=0; i < messageLength; i++)
            message[i] = currentByteBuffer[i+4];
        
        crc16[0] = currentByteBuffer[messageLength-1+5];
        crc16[1] = currentByteBuffer[messageLength-1+6];

        end = currentByteBuffer[messageLength-1+7];

        // if(isVerbose){
        //     compareReadData(stageName, &byteReceived, &sendingFrameObjList[receivingFrameObjIndex].preambule, 1);
        // }
        
    }

};

//extern bool isSending;

extern int bitCounter;

extern int currentSendingFrameObjIndex;
extern int sendingFrameObjIndex;
extern int receivingFrameObjIndex;
extern Frame sendingFrameObjList[50];
extern Frame receivingFrameObjList[50];



// extern frame sendingFrame;
// extern frame receivingFrame;

//extern FrameManagerState currentSendingState;
//extern FrameManagerState currentReceivingState;


void sendDataFrame(uint8_t*, uint8_t, bool);
void startTransmission();
void endTransmission();
void receiveBit(uint8_t, bool);
void resetCounters();
void receiveData(uint8_t);
void calcCRCNSend();
uint16_t crc16(const uint8_t* data_p, uint8_t length);
bool compareCRC16(uint16_t, uint16_t);
bool compareReadData(const char*, uint8_t*, uint8_t*, int);
