#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef unsigned long long uintptr_t;
typedef unsigned long long size_t;

extern void print(char* string);
extern void printLine(char* string, uint8_t line);
extern void printHex(uintptr_t str, int pfx);
extern void printLineHex(uintptr_t str, int pfx, uint8_t line);
extern void printChar(char character);
extern void clearScreen();

extern void* pmmAlloc();
extern void pmmFreePage(void* addr);
extern void dumpPmmBitmap(int start, int end);

extern void* malloc(size_t size);
extern void free(uintptr_t addr);

extern int compareArray(char arr1[], char arr2[]);
extern void strCopy(char* str, char* addr);

extern int commandReady;

char buffer[128];
uint8_t bufferIndex = 0;

void keyboardHandler_c(uint8_t scancode);
void processBuffer();
void clearBuffer();

char keyMap[128] = {0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 59,  39, '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 32};

#endif