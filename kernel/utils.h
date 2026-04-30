#ifndef UTILS_H
#define UTILS_H

#include "types.h"

// As these are static inline, they get defined in the header
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port)); //different syntax because c asm is weird
}
static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile ("outw %w0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
static inline uint16_t inw(uint16_t port)
{
    uint16_t result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

int compareArray(char* str1, char* str2);
int compareArraySize(char* str1, char* str2, int amount);

void memSet(uint8_t* addr, char c, size_t amount);

void strCopy(char* str, char* addr);
void strCopySize(char* str, char* addr, int amount);
size_t strLen(char* str);
void split(char* in, char splitter, char* out1, char* out2);
void splitN(char* in, char splitter, char* out1, char* out2);

int count(char* str, char c);

uint64_t convInt(char* input);

#endif