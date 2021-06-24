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
    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000001;
    uint8_t* message;
    uint8_t* crc16;
    bool crcCorrect = false;

    bool* bitArray;
    uint8_t bitArraySize;

    ~Frame(){
        if(message)
            delete[] message;
        if(bitArray)
            delete[] bitArray;
    };


    // Frame(uint8_t flag=0b00000000, uint8_t mLength=0., uint8_t* msg, uint8_t crc[2]){
    //     typeFlag = flag;
    //     messageLength = flag;
    //     crc16 = crc;
    //     message = msg;
    // }

};

extern bool isSending;

extern int currentSendingFrameObjIndex = 0;
extern int sendingFrameObjIndex = -1;
extern int receivingFrameObjIndex = -1;
extern Frame sendingFrameObjList[10];
extern Frame receivingFrameObjList[10];

// extern frame sendingFrame;
// extern frame receivingFrame;

extern FrameManagerState currentSendingState;
extern FrameManagerState currentReceivingState;


void sendDataFrame(uint8_t*, uint8_t, bool);
void receiveBit(uint8_t);
void receiveData(uint8_t);
uint16_t crc16(const uint8_t* data_p, uint8_t length);
bool compareCRC16(uint16_t, uint16_t);
bool compareReadData(const char*, uint8_t*, uint8_t*, int);
