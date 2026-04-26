#include "types.h"

#include "printUtils.h"
#include "memUtils.h"

#include "keyboardHandler.h"
#include "shell.h"

extern char buffer[128];

uint8_t* savedScreen;

statuses_t sysStatus;

void main()
{
    initPMM();
    initHeap();

    savedScreen = (uint8_t*)malloc(4002);

    print("Kernel Loaded\n");

    sysStatus.shell = 1;

    __asm__ volatile ("hlt");

    return;
}