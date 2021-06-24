#pragma once
#include <stdint.h>

enum FrameManagerState { preambule, start, entete, message, controle, end };

// struct frame {
//     uint8_t preambule = 0b01010101;
//     uint8_t startEnd = 0b01111110;
//     uint8_t typeFlag = 0b00000000;
//     uint8_t messageLength = 0b00000001;
//     uint8_t* message = new uint8_t[80]{0};
//     uint8_t crc16[2];
// };

class Frame{
public:
    bool isSender = false;

    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000001;
    uint8_t message[80];
    uint8_t crc16[2];
    bool crcCorrect = false;

    bool* bitArray = nullptr;
    int bitArraySize;

    ~Frame(){
        // if(message != nullptr)
        //     delete[] message;
        if(bitArray != nullptr)
            delete[] bitArray;
    };

    void setupArrays(){
        //message = new uint8_t[messageLength];

        if (isSender){
            bitArraySize = (messageLength + 7) * 8;
            bitArray = new bool[bitArraySize];
        }
    }

};

//extern bool isSending;

extern int currentSendingFrameObjIndex;
extern int sendingFrameObjIndex;
extern int receivingFrameObjIndex;
extern Frame sendingFrameObjList[100];
extern Frame receivingFrameObjList[100];

// extern frame sendingFrame;
// extern frame receivingFrame;

extern FrameManagerState currentSendingState;
extern FrameManagerState currentReceivingState;


void sendDataFrame(uint8_t*, uint8_t, bool);
void receiveBit(uint8_t);
void resetCounters();
void receiveData(uint8_t);
void calcCRCNSend();
uint16_t crc16(const uint8_t* data_p, uint8_t length);
bool compareCRC16(uint16_t, uint16_t);
bool compareReadData(const char*, uint8_t*, uint8_t*, int);
