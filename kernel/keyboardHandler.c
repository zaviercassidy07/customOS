#include "keyboardHandler.h"

extern int commandReady;

char buffer[128];
uint8_t bufferIndex = 0;

char keyMap[128] = {0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 59,  39, '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, 32};

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
        print("HEAP TEST START\n\n");

		uintptr_t* ptrs[64];
		size_t sizes[64];

		// Allocate variable sizes
		for(int i = 0; i < 64; i++)
		{
			sizes[i] = 128 + (i * 32);
			ptrs[i] = (uintptr_t*)malloc(sizes[i]);

			if(!ptrs[i])
			{
				print("ALLOC FAIL\n");
				return;
			}
		}

		print("ALLOC OK\n");

		// Free middle region
		for(int i = 16; i < 48; i++)
		{
			free((uintptr_t)ptrs[i]);
		}

		print("FREED MID\n");

		// Track freed region range
		uintptr_t freeStart = (uintptr_t)ptrs[16];
		uintptr_t freeEnd   = (uintptr_t)ptrs[47];

		// Allocate larger blocks
		int reused = 0;
		for(int i = 0; i < 16; i++)
		{
			uintptr_t* p = (uintptr_t*)malloc(2048);
            printHex((uintptr_t)p, 1); print("\n");

			if(!p)
            {
				break;
            }

			// Check if allocation landed inside freed region
			if((uintptr_t)p >= freeStart && (uintptr_t)p <= freeEnd)
			{
				reused++;
			}
		}

		// Output
		print("REUSE: ");
		printHex(reused, 1);

        print("\n\nHEAP TEST END\n");
    }
    else if(compareArray(command, "read") == 1)
    {
        uint64_t* read = (uint64_t*)malloc(512);;
        readBytes(53760, 8, (uint8_t*)read);
        print("Read test (expect 0x6969696969696969): "); printHex((uintptr_t)read[0], 1); print("\n");
    }
    else if(compareArray(command, "conv") == 1)
    {
        uint64_t out = convInt(options);
        printHex(out, 1);
        print("\n");
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