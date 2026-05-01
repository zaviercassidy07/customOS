#include "f32.h"

f32BS_t* bs;

uint32_t fatStart;
uint32_t dataStart;
uint32_t rootCluster;
uint8_t sectorsPerCluster;
uint32_t partStart;

dirEntry_t foundEntry;

void initF32(uint32_t start)
{
    partStart = start;

    bs = (f32BS_t*)malloc(512);
    readSectors(partStart, 1, (uint16_t*)bs);

    sectorsPerCluster = bs->sectorsPerCluster;
    fatStart = partStart + bs->reservedSectors;
    dataStart = fatStart + (bs->numFATs * bs->fatSize32);
    rootCluster = bs->rootCluster;
}

uint32_t clusterToSector(uint32_t cluster)
{
    return dataStart + (cluster - 2) * bs->sectorsPerCluster; // There are two unusable clusters that are counted in index, so thats why -2
}

uint32_t nextCluster(uint32_t cluster)
{
    uint32_t fatOffset = cluster * 4; // 4 bytes per entry
    uint32_t fatSector = fatStart + (fatOffset / bs->bytesPerSector); // get sector of cluster
    uint32_t entryOffset = fatOffset % bs->bytesPerSector; // get index of next cluster

    uint8_t* sectorBuf = (uint8_t*)malloc(512);
    readSectors(fatSector, 1, (uint16_t*)sectorBuf); // load it

    uint32_t val = *(uint32_t*)(sectorBuf + entryOffset) & 0x0FFFFFFF; // value at this cluster is the next cluster index
    free((uintptr_t)sectorBuf);
    return val;
}

uint32_t nextFreeCluster()
{
    uint8_t* sectorBuf = (uint8_t*)malloc(512);
    if(!sectorBuf)
    {
        return 0;
    }

    for(uint32_t fatSector = fatStart; fatSector < fatStart + bs->fatSize32; fatSector++)
    {
        readSectors(fatSector, 1, (uint16_t*)sectorBuf);
        uint32_t* entries = (uint32_t*)sectorBuf;

        for(int i = 0; i < 128; i++)
        {
            if((entries[i] & 0x0FFFFFFF) == 0)
            {
                uint32_t cluster = (fatSector - fatStart) * (bs->bytesPerSector / 4) + i;
                if(cluster >= 2)
                {
                    free((uintptr_t)sectorBuf);
                    return cluster;
                }
            }
        }
    }
    free((uintptr_t)sectorBuf);

    return 0; // disk full
}

dirEntry_t* findFile(char* name, int startCluster) // 0 is placeholder for route cluster in start
{
    uint8_t* clusterBuf = (uint8_t*)malloc(4096);

    uint32_t cluster;
    if(startCluster == 0)
    {
        cluster = rootCluster;
    }
    else
    {
        cluster = startCluster;
    }

    char fName[8];
    char ext[3];
    memSet(fName, 0, 8);
    memSet(ext, 0, 3);
    splitN(name, '.', fName, ext);

    while(cluster < 0x0FFFFFF8)
    {
        uint32_t sector = clusterToSector(cluster);
        readSectors(sector, bs->sectorsPerCluster, (uint16_t*)clusterBuf);

        dirEntry_t* entries = (dirEntry_t*)clusterBuf;
        int count = (bs->sectorsPerCluster * bs->bytesPerSector) / 32; // directory is 32 bytes, so this is dirs per cluster

        for(int i = 0; i < count; i++)
        {
            if(entries[i].name[0] == 0x00) // if no more entries
            {
                free((uintptr_t)clusterBuf);
                return NULL;
            }
            if(entries[i].name[0] == 0xE5) // if entry deleted
            {
                continue;
            }
            if(entries[i].attributes == 0x0F) // if long file name (not supported)
            {
                continue;
            }

            if(compareArraySize(fName, entries[i].name, 8) == 1 && compareArraySize(ext, entries[i].ext, 3) == 1)
            {
                foundEntry = entries[i];
                free((uintptr_t)clusterBuf);
                return &foundEntry;
            }
        }

        cluster = nextCluster(cluster);
    }
    free((uintptr_t)clusterBuf);
    return NULL;
}

void readFile(char* name, uint8_t* dest)
{
    dirEntry_t* entry = findFile(name, 0);
    if(!entry)
    {
        print("File doesn't exist.\n");
        return;
    }

    uint32_t cluster = ((uint32_t)entry->clusterHigh << 16) | entry->clusterLow;
    uint32_t remaining = entry->fileSize;
    uint32_t clusterBytes = bs->sectorsPerCluster * bs->bytesPerSector;

    uint8_t* clusterBuf = (uint8_t*)malloc((entry->fileSize + 4095) & ~0xFFF);

    while(cluster < 0x0FFFFFF8 && remaining > 0)
    {
        uint32_t sector = clusterToSector(cluster);
        readSectors(sector, bs->sectorsPerCluster, (uint16_t*)clusterBuf);

        uint32_t bytesRead;
        if(remaining < clusterBytes)
        {
            bytesRead = remaining;
        }
        else
        {
            bytesRead = clusterBytes;
        }

        for(int i = 0; i < bytesRead; i++)
        {
            dest[i] = clusterBuf[i];
        }

        dest += bytesRead;
        remaining -= bytesRead;

        cluster = nextCluster(cluster);
    }

    free((uintptr_t)clusterBuf);
}

void createFile(char* name)
{
    uint32_t new = nextFreeCluster();
    uint32_t dir = rootCluster;

    uint32_t fatOffset = new * 4; 
    uint32_t fatSector = fatStart + (fatOffset / bs->bytesPerSector); 
    uint32_t entryOffset = fatOffset % bs->bytesPerSector; 

    uint32_t* tmp = (uint32_t*)malloc(4);
    tmp[0] = 0x0FFFFFFF;
    writeBytes(fatSector * 512 + entryOffset, 4, (uint8_t*)tmp);
    writeBytes((fatSector + bs->fatSize32) * 512 + entryOffset, 4, (uint8_t*)tmp);
    free((uintptr_t)tmp);

    dirEntry_t* entry = (dirEntry_t*)malloc(32);
    memSet((uint8_t*)entry, 0, 32);
    splitN(name, '.', (char*)entry, (char*)entry + 8);

    entry->attributes = 0x20;
    entry->clusterHigh = (new >> 16) & 0xFFFF;
    entry->clusterLow = new & 0xFFFF;
    entry->fileSize = 4096;

    uint32_t* clusterBuf = (uint32_t*)malloc(4096);
    readSectors(clusterToSector(dir), 8, (uint16_t*)clusterBuf);
    for(int i = 0; i < 128; i++)
    {
        dirEntry_t* data = (dirEntry_t*)((uint8_t*)clusterBuf + (i * 32));
        if(data->name[0] == 0x00 || data->name[0] == 0xE5)
        {
            strCopySize((char*)entry, (char*)((uint8_t*)clusterBuf + i * 32), 32);
            break;
        }
    }
    writeSectors(clusterToSector(dir), 8, (uint16_t*)clusterBuf);

    free((uintptr_t)clusterBuf);
    free((uintptr_t)entry);
    return;
}

void writeFile(char* name, int amount, uint8_t* src)
{
    dirEntry_t* file = findFile(name, 0);
    if(!file)
    {
        print("File doesn't exist.\n");
        return;
    }

    uint32_t cluster = ((uint32_t)file->clusterHigh << 16) | file->clusterLow;

    writeBytes((uint64_t)clusterToSector((uint32_t)cluster) * 512, amount, (uint8_t*)src);
    return;
}