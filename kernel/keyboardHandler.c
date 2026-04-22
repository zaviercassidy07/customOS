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
        print("HEAP TEST START\n\n");

		#define N 64
		uintptr_t* ptrs[N];
		size_t sizes[N];

		// Allocate variable sizes
		for(int i = 0; i < N; i++)
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

			if(!p)
				break;

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