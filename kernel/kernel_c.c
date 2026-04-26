#include "types.h"

#include "printUtils.h"
#include "memUtils.h"

#include "keyboardHandler.h"
#include "shell.h"

int commandReady = 0;
extern char buffer[128];

uint8_t* savedScreen;

void main()
{
    initPMM();
    initHeap();

    savedScreen = (uint8_t*)malloc(4002);

    print("Kernel Loaded\n");

    while(1 == 1)
    {
        if(commandReady == 1)
        {
            shell(buffer);
            commandReady = 0;
        }
        else
        {
            __asm__ volatile ("hlt");
        }
    }

    return;
}