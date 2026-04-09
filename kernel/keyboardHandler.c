#include "keyboardHandler.h"

void keyboardHandler_c (uint8_t scancode)
{
    extern char keyMap[128];
    extern void printChar(char character);

    char character = keyMap[scancode];

    if(character == 13)
    {
        extern void newLine();
        newLine();
        return;
    }

    printChar(character);

    return;
}