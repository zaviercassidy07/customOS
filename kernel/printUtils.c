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
    if(cursorX == 80)
    {
        return;
    }
    if(character == 0)
    {
        cursorX--;
    }

    char* pos = (char*)(uint64_t)(0xB8000 + (cursorY*80 + cursorX)*2);
    pos[0] = character;
    pos[1] = 0x0F;

    if(character != 0)
    {
        cursorX++;
    }
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

void clearScreen()
{
    uint16_t* pos = (uint16_t*)(0xB8000); //do this instead of char as it leaves the text colour byte normal
    for(int i = 0; i < 80*25; i++)
    {
        pos[i] = 0x0F20;
    }

    cursorX = 0;
    cursorY = 0;
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