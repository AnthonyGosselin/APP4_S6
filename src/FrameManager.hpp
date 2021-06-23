#pragma once
#include <stdint.h>

enum FrameManagerState { preambule, start, entete, message, controle, end };

struct frame {
    uint8_t preambule = 0b01010101;
    uint8_t startEnd = 0b01111110;
    uint8_t typeFlag = 0b00000000;
    uint8_t messageLength = 0b00000001;
    uint8_t* message = new uint8_t[80];
    uint8_t crc16[2];
    bool crcCorrect = false;
};

extern bool isSending;

extern int bitCounter;
extern int byteCounter;
extern uint8_t byteConcat;

extern frame sendingFrame;
extern frame receivingFrame;

extern FrameManagerState currentSendingState;
extern FrameManagerState currentReceivingState;

extern bool readyToSendFrame;
extern bool *bitArray;
extern uint8_t bitArraySize;

void sendDataFrame(uint8_t*, uint8_t);
void receiveBit(uint8_t);
void receiveData(uint8_t);
uint16_t crc16(const uint8_t* data_p, uint8_t length);
bool compareCRC16(uint16_t, uint16_t);
bool compareReadData(const char*, uint8_t*, uint8_t*, int);
