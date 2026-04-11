#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;

uint8_t cursorX = 0;
uint8_t cursorY = 0;

void print(char* string);
void printChar(char character);
void newLine();
void clearScreen();
void moveCursor();

#endif