typedef unsigned long long uintptr_t;

#include "printUtils.h"
#include "memUtils.h"

#include "keyboardHandler.h"
#include "shell.h"

int commandReady = 0;
extern char buffer[128];

void main()
{
    initPMM();
    initHeap();

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