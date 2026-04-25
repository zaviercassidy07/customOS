#include "ata.h"

void readSectors(uintptr_t lba, size_t amount, uint16_t* location)
{
    if(checkBus() == 0)
    {
        return;
    }
    waitBSY();

    outb(0x1F6, 0x40); //drive select port, gets 0x40 as placeholder since 48 bits contains drive

    outb(0x1F2, (amount >> 8) & 0xFF);
    outb(0x1F3, (lba >> 24) & 0xFF); // send upper 32 bits in ascending order
    outb(0x1F4, (lba >> 32) & 0xFF);
    outb(0x1F5, (lba >> 40) & 0xFF);

    outb(0x1F2, amount & 0xFF);
    outb(0x1F3, lba & 0xFF); // then send lower 32 bits
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);

    outb(0x1F7, 0x24); // EXTENDED READ command

    for(int i = 0; i < amount; i++)
    {
        waitBSY();
        waitDRQ();

        for(int j = 0; j < 256; j++) // sector is 512b, but we're working in words so only 256 words
        {
            location[j] = inw(0x1F0); // data is streamed in on this port until completion
        }
        location += 256;
    }
}

void readBytes(uint64_t byteOffset, size_t amount, uint8_t* location)
{
    uint64_t offset = byteOffset % 512;
    uint64_t lba = byteOffset / 512;
    size_t sectors = (offset + amount + 511) / 512;

    uint8_t* tmp = (uint8_t*)malloc(sectors * 512);
    readSectors(lba, sectors, (uint16_t*)tmp);

    for(size_t i = 0; i < amount; i++)
    {
        location[i] = tmp[offset + i];
    }

    free((uintptr_t)tmp);
}

void writeSectors(uint64_t lba, size_t amount, uint16_t* source)
{ //very similar to read, except outw instead of inw
    if(checkBus() == 0)
    {
        return;
    }
    waitBSY();

    outb(0x1F6, 0x40); //drive select port, gets 0x40 as placeholder since 48 bits contains drive

    outb(0x1F2, (amount >> 8) & 0xFF);
    outb(0x1F3, (lba >> 24) & 0xFF); // send upper 32 bits in ascending order
    outb(0x1F4, (lba >> 32) & 0xFF);
    outb(0x1F5, (lba >> 40) & 0xFF);

    outb(0x1F2, amount & 0xFF);
    outb(0x1F3, lba & 0xFF); // then send lower 32 bits
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);

    outb(0x1F7, 0x34); // WRITE command

    for(int i = 0; i < amount; i++)
    {
        waitBSY();
        waitDRQ();

        for(int j = 0; j < 256; j++)
        {
            outw(0x1F0, source[j]);
        }

        source += 256;
    }

    waitBSY();
    outb(0x1F7, 0xEA); //flush cache
    waitBSY();
}

void writeBytes(uint64_t byteOffset, size_t amount, uint8_t* source)
{
    uint64_t offset = byteOffset % 512;
    uint64_t lba = byteOffset / 512;
    size_t sectors = (offset + amount + 511) / 512;

    if(offset == 0 && amount % 512 == 0) // if its a perfect sector, we can skip read operation as no data to preserve
    {
        writeSectors(lba, sectors, (uint16_t*)source);
        return;
    }

    uint8_t* tmp = (uint8_t*)malloc(sectors * 512);
    readSectors(lba, sectors, (uint16_t*)tmp);

    for(size_t i = 0; i < amount; i++)
    {
        tmp[offset + i] = source[i];
    }

    writeSectors(lba, sectors, (uint16_t*)tmp);

    free((uintptr_t)tmp);
}