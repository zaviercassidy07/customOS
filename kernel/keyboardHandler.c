#include "keyboardHandler.h"

void keyboardHandler_c(uint8_t scancode)
{
    char character = keyMap[scancode];

    if(character == 0 || bufferIndex == 80) //unmapped key or end of line
    {
        return;
    }
    if(character == 13) //enter
    {
        newLine();
        commandReady = 1; //apparently interupts need to be small, moved rest of function into main
        return;
    }
    if(character == 8) //backspace
    {
        if(bufferIndex == 0)
        {
            return;
        }

        bufferIndex--;
        printChar(0);
        buffer[bufferIndex] = 0;
        return;
    }

    buffer[bufferIndex] = character;
    bufferIndex++;

    printChar(character);

    return;
}

void processBuffer()
{
    int commandIndex = 0;

    char command[16];
    char options[112];

    for(int i = 0; i <  bufferIndex; i++)
    {
        if(buffer[i] == ' ')
        {
            commandIndex = i;
            break;
        }
    }
    if(commandIndex == 0)
    {
        commandIndex = bufferIndex;
    }

    for(int i = 0; i < commandIndex; i++)
    {
        command[i] = buffer[i];
    }
    command[commandIndex] = 0;

    if(commandIndex != bufferIndex)
    {
        for(int i = commandIndex + 1; i < bufferIndex; i++)
        {
            options[i - (commandIndex + 1)] = buffer[i];
        }
        options[bufferIndex - (commandIndex + 1)] = 0;
    }

    if(compareArray(command, "echo") == 1)
    {
        print(options);
        newLine();
    }
    else if(compareArray(command, "clear") == 1)
    {
        clearScreen();
    }
    else
    {
        print("Not recognized");
        newLine();
    }

    clearBuffer();
    return;
}

void clearBuffer()
{
    for(int i = 0; i < bufferIndex; i++)
    {
        buffer[i] = 0;
    }
    bufferIndex = 0;
    return;
}