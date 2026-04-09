#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include <stdint.h>

char buffer[128];
uint8_t bufferIndex = 0;

void keyboardHandler_c (uint8_t scancode);

char keyMap[128] = {0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 59,  39, '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 32};

#endif