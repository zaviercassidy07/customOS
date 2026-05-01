#include "types.h"

#include "printUtils.h"
#include "memUtils.h"
#include "f32.h"

#include "keyboardHandler.h"
#include "shell.h"

uint8_t* savedScreen;

statuses_t sysStatus;

void main()
{
    initPMM();
    initHeap();
    initF32(105);

    savedScreen = (uint8_t*)malloc(4002);

    print("Kernel Loaded\n");

    sysStatus.shell = 1;

    __asm__ volatile ("hlt");

    return;
}