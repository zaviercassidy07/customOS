#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include "types.h"

#include "utils.h"

void print(char* string);
void printLine(char* string, uint8_t line);
void printHex(uintptr_t str, int pfx);
void printLineHex(uintptr_t str, int pfx, uint8_t line);
void printChar(char character);
void newLine();

void saveScreen(uint8_t* addr);
void restoreScreen(uint8_t* addr);
void clearScreen();

void moveCursor();

#endif