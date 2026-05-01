#include "shell.h"

extern uint8_t* savedScreen;

void shell(char* input)
{
    char command[16];
    char options[112];

    split(input, ' ', command, options);

    if(compareArray(command, "echo") == 1)
    {
        print(options);
        print("\n");
    }
    else if(compareArray(command, "clear") == 1)
    {
        clearScreen();
    }
    else if(compareArray(command, "save") == 1)
    {
        saveScreen(savedScreen);
    }
    else if(compareArray(command, "restore") == 1)
    {
        restoreScreen(savedScreen);
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
        char* read = (char*)malloc(4096);
        char name[12];
        strCopySize(options, name, 12);

        readFile(name, read);
        print("Data: "); print(read); print("\n");
        free((uintptr_t)read);
    }
    else if(compareArray(command, "write") == 1)
    {
        char* write = (char*)malloc(128);
        char name[12];
        split(options, ' ', name, write);

        size_t bytes = strLen(write);

        writeFile(name, bytes, write);
        print("Attempted to store: "); print(write); print("\n");
        free((uintptr_t)write);
    }
    else if(compareArray(command, "conv") == 1)
    {
        uint64_t out = convInt(options);
        printHex(out, 1);
        print("\n");
    }
    else if(compareArray(command, "count") == 1)
    {
        printHex((uint64_t)count(options, '/'), 1);
        print("\n");
    }
    else if(compareArray(command, "touch") == 1)
    {
        createFile(options);
        print("Done\n");
    }
    else
    {
        print("Not recognized\n");
    }

    clearBuffer();
    return;
}