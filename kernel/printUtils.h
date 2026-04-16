#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;
typedef unsigned long long uintptr_t;

uint8_t cursorX = 0;
uint8_t cursorY = 0;

const char hex[] = "0123456789ABCDEF";

void print(char* string);
void printLine(char* string, uint8_t line);
void printHex(uintptr_t str, int pfx);
void printLineHex(uintptr_t str, int pfx, uint8_t line);
void printChar(char character);
void newLine();
void clearScreen();
void moveCursor();

#endif