#ifndef ATA_H
#define ATA_H

#include "types.h"
#include "utils.h"
#include "printUtils.h"

// 0x1F7 is status port
static inline void waitBSY()
{
    while(inb(0x1F7) & 0x80); // wait until drive not busy
}
static inline void waitDRQ()
{
    while(!(inb(0x1F7) & 0x08)); // wait until data is ready
}
static inline int checkBus()
{
    if(inb(0x1F7) == 0xFF)
    {
        print("ATA: Something isn't working\n");
        return 0;
    }
    return 1;
}

void readSectors(uintptr_t lba, size_t amount, uint16_t* location);

#endif