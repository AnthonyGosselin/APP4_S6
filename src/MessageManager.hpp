#pragma once
#include <stdint.h>

extern uint8_t* lastMessageSent;

void sendMessage(uint8_t*, uint8_t);
void receiveMessage(uint8_t*);
bool compareReadMessage(bool, uint8_t*, uint8_t*, uint8_t);
