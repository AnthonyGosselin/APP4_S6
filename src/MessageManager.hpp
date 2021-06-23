#pragma once
#include <stdint.h>

void sendMessage(uint8_t*, uint8_t, bool);
void receiveMessage(uint8_t*);
bool compareReadMessage(bool, uint8_t*, uint8_t*, uint8_t);