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
        print("\n");
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
        printChar('\b');
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
        print("\n");
    }
    else if(compareArray(command, "clear") == 1)
    {
        clearScreen();
    }
    else if(compareArray(command, "pmm") == 1)
    {
        print("Begin PMM Test\n");

        print("Initial bitmap: \n");
        dumpPmmBitmap(0, 40);
        void* p1 = pmmAlloc();
        void* p2 = pmmAlloc();
        void* p3 = pmmAlloc();
        print("\nP1 Addr: ");
        printHex((uintptr_t)p1, 1);
        print("\nP2 Addr: ");
        printHex((uintptr_t)p2, 1);
        print("\nP3 Addr: ");
        printHex((uintptr_t)p3, 1);
        print("\nBitmap: \n");
        dumpPmmBitmap(0, 40);
        print("\n");
        pmmFreePage(p1);
        pmmFreePage(p2);
        pmmFreePage(p3);
        print("Pages freed. Bitmap: \n");
        dumpPmmBitmap(0, 40);
        print("\n");
        void* p4 = pmmAlloc();
        print("P4 Addr (expected P1): ");
        printHex((uintptr_t)p4, 1);
        print("\nCurrent bitmap: \n");
        dumpPmmBitmap(0, 40);
        pmmFreePage(p4);
        print("\nCleared bitmap: \n");
        dumpPmmBitmap(0, 40);

        print("\nEnd PMM Test\n");
    }
    else if(compareArray(command, "malloc") == 1)
    {
        print("Begin malloc test\n");

        char* a = (char*)malloc(2048);
        uintptr_t* b = (uintptr_t*)malloc(1024);
        uintptr_t* c = (uintptr_t*)malloc(2048);
        uintptr_t* d = (uintptr_t*)malloc(12);
        uintptr_t* e = (uintptr_t*)malloc(5000);
        uintptr_t* f = (uintptr_t*)malloc(30);

        print("A: ");
        printHex((uintptr_t)a, 1);
        print("\nB: ");
        printHex((uintptr_t)b, 1);
        print("\nC: ");
        printHex((uintptr_t)c, 1);
        print("\nD: ");
        printHex((uintptr_t)d, 1);
        print("\nE: ");
        printHex((uintptr_t)e, 1);
        print("\nF: ");
        printHex((uintptr_t)f, 1);

        strCopy("A CONTENT", a);

        print("\nA content test: ");
        print(a);

        free((uintptr_t)c);
        char* g = (char*)malloc(512);
        strCopy("G TEST", g);

        print("\nG Addr (expect C): ");
        printHex((uintptr_t)g, 1);
        print("\nG content: ");
        print(g);

        print("\nEnd malloc test\n");
    }
    else
    {
        print("Not recognized\n");
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