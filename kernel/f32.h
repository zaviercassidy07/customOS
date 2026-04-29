#ifndef F32_H
#define F32_H

#include "types.h"

#include "utils.h"
#include "memUtils.h"

#include "drivers/ata.h"

typedef struct f32BS_t
{
    uint8_t  jump[3];
    uint8_t  oemName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t  numFATs;
    uint16_t rootEntryCount;    // 0 for FAT32
    uint16_t totalSectors16;    // 0 for FAT32
    uint8_t  media;
    uint16_t fatSize16;         // 0 for FAT32
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    // FAT32 extended BPB
    uint32_t fatSize32;
    uint16_t extFlags;
    uint16_t fsVersion;
    uint32_t rootCluster;
    uint16_t fsInfoSector;
    uint16_t backupBootSector;
    uint8_t  reserved[12];
    uint8_t  driveNumber;
    uint8_t  reserved1;
    uint8_t  bootSignature;
    uint32_t volumeID;
    uint8_t  volumeLabel[11];
    uint8_t  fsType[8];
} __attribute__((packed)) f32BS_t;

typedef struct dirEntry_t
{
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t createTimeTenth;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t accessDate;
    uint16_t clusterHigh;
    uint16_t modTime;
    uint16_t modDate;
    uint16_t clusterLow;
    uint32_t fileSize;
} __attribute__((packed)) dirEntry_t;

void initF32(uint32_t start);

uint32_t clusterToSector(uint32_t cluster);
uint32_t nextCluster(uint32_t cluster);

dirEntry_t* findFile(char* name);
void readFile(dirEntry_t* entry, uint8_t* dest);

#endif