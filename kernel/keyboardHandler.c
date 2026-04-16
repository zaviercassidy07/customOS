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
    else if(compareArray(command, "heap") == 1)
    {
        clearScreen();
        print("HEAP TEST START\n");

        //alloc tests
        print("\n[1] Sequential alloc\n");

        void* a = malloc(256);
        void* b = malloc(256);
        void* c = malloc(256);

        print("A: "); printHex((uintptr_t)a, 1); print("\n");
        print("B: "); printHex((uintptr_t)b, 1); print("\n");
        print("C: "); printHex((uintptr_t)c, 1); print("\n");

        //free + reuse
        print("\n[2] Free + reuse\n");

        free((uintptr_t)b);
        void* b2 = malloc(200);

        print("B2: "); printHex((uintptr_t)b2, 1); print("\n");

        //merge test
        print("\n[3] Merge test\n");

        free((uintptr_t)a);
        free((uintptr_t)b2);
        free((uintptr_t)c);

        void* big = malloc(700);
        print("BIG: "); printHex((uintptr_t)big, 1); print("\n");

        //fragmentation test
        print("\n[4] Fragmentation\n");

        void* f1 = malloc(128);
        void* f2 = malloc(128);
        void* f3 = malloc(128);
        void* f4 = malloc(128);

        free((uintptr_t)f1);
        free((uintptr_t)f3);

        void* f5 = malloc(200);

        print("F5: "); printHex((uintptr_t)f5, 1); print("\n");

        //Heap stress
        print("\n[5] Stress alloc\n\n");
        uint64_t allocated = 1156;
        void* last = (uintptr_t)0;

        for(int i = 0; i < 242920; i++) //just under 1GB total
        {
            last = malloc(4096);

            if(!last)
            {
                print("ALLOC FAIL\n");
                break;
            }

            if(i % 512 == 0)
            {
                printLineHex((uintptr_t)last, 1, 17);
            }

            allocated += 4096;
        }
        print("Total allocated: ");
        printHex(allocated, 1);
        print("\n\nHEAP TEST END\n");
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