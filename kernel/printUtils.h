#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include <stdint.h>

uint8_t cursorX = 0;
uint8_t cursorY = 0;

void print(char* string);
void printChar(char character);
void newLine();
void moveCursor();

#endif