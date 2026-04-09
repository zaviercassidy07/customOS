#include "printUtils.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port)); //different syntax because c asm is weird
}

void print(char* string)
{
    while(string[0] != 0)
    {
        printChar(string[0]);
        string++;
    }
    return;
}

void printChar(char character)
{
    char* pos = (char*)(0xB8000 + (cursorY*80 + cursorX)*2);
    pos[0] = character;
    pos[1] = 0x0F;

    cursorX++;
    moveCursor();

    return;
}

void newLine()
{
    cursorX = 0;
    cursorY++;

    moveCursor();

    return;
}

void moveCursor()
{
    uint16_t pos = cursorY * 80 + cursorX;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF)); //filters out high byte so only low byte is set

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF)); //moves high byte to low then does same thing

    return;
}