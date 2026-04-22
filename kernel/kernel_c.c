typedef unsigned long long uintptr_t;

#include "printUtils.h"
#include "memUtils.h"

#include "keyboardHandler.h"

int commandReady = 0;

void main()
{
    initPMM();
    initHeap();

    print("Kernel Loaded\n");

    while(1 == 1)
    {
        if(commandReady == 1)
        {
            processBuffer();
            commandReady = 0;
        }
        else
        {
            __asm__ volatile ("hlt");
        }
    }

    return;
}