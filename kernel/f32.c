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

dirEntry_t* findFile(char* name)
{
    uint8_t* clusterBuf = (uint8_t*)malloc(4096);

    uint32_t cluster = rootCluster;

    while(cluster < 0x0FFFFFF8)
    {
        uint32_t sector = clusterToSector(cluster);
        readSectors(sector, bs->sectorsPerCluster, (uint16_t*)clusterBuf);

        dirEntry_t* entries = (dirEntry_t*)clusterBuf;
        int count = (bs->sectorsPerCluster * bs->bytesPerSector) / 32; // directory is 32 bytes, so this is dirs per cluster

        char fName[8];
        char ext[3];
        memSet(fName, 0, 8);
        memSet(ext, 0, 3);
        splitN(name, '.', fName, ext);

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

void readFile(dirEntry_t* entry, uint8_t* dest)
{
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