#pragma once
#include <stdint.h>

void sendMessage(uint8_t*, uint8_t, uint8_t=0b00000000);
void receiveMessage(uint8_t*, uint8_t);
bool compareReadMessage(bool, uint8_t*, uint8_t*, uint8_t);