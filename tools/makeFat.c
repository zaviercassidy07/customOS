#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Image config
#define SECTOR_SIZE 512
#define SECTORS_TOTAL 65536 // 32MB
#define RESERVED 32 // reserved sectors before FAT
#define NUM_FATS 2
#define SECTORS_PER_CLUSTER 8 // 4KB clusters
#define FAT_SIZE 512 // sectors per FAT table
#define ROOT_CLUSTER 2 // root dir starts at cluster 2

// Computed layout
#define FAT1_START RESERVED
#define FAT2_START (FAT1_START + FAT_SIZE)
#define DATA_START (FAT2_START + FAT_SIZE)
// cluster N starts at sector: DATA_START + (N-2)*SECTORS_PER_CLUSTER

#define IMAGE_SIZE (SECTORS_TOTAL * SECTOR_SIZE)

// File content
#define TEST_CONTENT "F32 TEST 123456 :) :/ ;) asdf asdf test womp womp"
#define TEST_CLUSTER 3 // file data goes in cluster 3

typedef struct f32BS_t
{
    uint8_t  jump[3];
    uint8_t  oemName[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t  numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t  media;
    uint16_t fatSize16;
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

// write a sector to file at given LBA
void writeSector(FILE* f, uint32_t lba, void* data)
{
    fseek(f, lba * SECTOR_SIZE, SEEK_SET);
    fwrite(data, SECTOR_SIZE, 1, f);
}

// read a sector from file
void readSector(FILE* f, uint32_t lba, void* data)
{
    fseek(f, lba * SECTOR_SIZE, SEEK_SET);
    fread(data, SECTOR_SIZE, 1, f);
}

// get the sector where cluster N's data lives
uint32_t clusterToSector(uint32_t cluster)
{
    return DATA_START + (cluster - 2) * SECTORS_PER_CLUSTER;
}

int main()
{
    FILE* f = fopen("fat.img", "wb");
    if(!f)
    {
        perror("fopen");
        return 1;
    }

    // zero the whole image first
    uint8_t zeroBuf[SECTOR_SIZE];
    memset(zeroBuf, 0, SECTOR_SIZE);
    for(int i = 0; i < SECTORS_TOTAL; i++)
        writeSector(f, i, zeroBuf);

    // get a bs struct
    uint8_t bootBuf[SECTOR_SIZE];
    memset(bootBuf, 0, SECTOR_SIZE);
    f32BS_t* bs = (f32BS_t*)bootBuf;

    bs->jump[0] = 0xEB; // jump instruction
    bs->jump[1] = 0x58; // placeholder, it diverts bios to bootloader normally, but since this isnt bootable it does nothing
    bs->jump[2] = 0x90; // no op
    memcpy(bs->oemName, "customOS", 8);
    bs->bytesPerSector = SECTOR_SIZE;
    bs->sectorsPerCluster = SECTORS_PER_CLUSTER;
    bs->reservedSectors = RESERVED; // convention is 32, amount of sectors between boot and first FAT table
    // note, if fat is bootable, bootloader code often goes in reserved
    bs->numFATs = NUM_FATS; // how many versions of table, 2 is normal as it has 1 backup
    bs->rootEntryCount = 0; // old versions had fixed root size, now its 0 as placeholder
    bs->totalSectors16 = 0;
    bs->media = 0xF8; // type of disk, f8 is non removable
    bs->fatSize16 = 0; // again, old backward compatible code
    bs->sectorsPerTrack = 0; // this and heads used for floppy geometry, never used by me and dont matter for hard disk
    bs->numHeads = 0;
    bs->hiddenSectors = 0;
    bs->totalSectors32 = SECTORS_TOTAL;
    bs->fatSize32 = FAT_SIZE; // sectors used by one fat
    bs->extFlags = 0;
    bs->fsVersion = 0;
    bs->rootCluster = ROOT_CLUSTER;
    bs->fsInfoSector = 1;
    bs->backupBootSector = 6; // FAT demands backup, 6 is convention
    bs->driveNumber = 0x80; // default drive num for hard drives
    bs->bootSignature = 0x29; // 29 indicates that next couple values are present
    bs->volumeID = 0x69696969;
    memcpy(bs->volumeLabel, "F32 TST VOL", 11);
    memcpy(bs->fsType, "FAT32", 5);

    // if boot sector, write the boot sig here
    // we're not doing boot sector so don't worry about it

    writeSector(f, 0, bootBuf);

    // Each FAT entry is a uint32, index is cluster num
    // [0]: 0x0FFFFFF8, Fs then media sector, just convention
    // [1]: Info sector
    // [2]: our root
    // [3]: our file
    // setting an entry to 0x0FFFFFFF means its the last entry in file
    // if [3] was = to 4, that would mean the file goes over cluster 3 and 4

    uint8_t fatBuf[SECTOR_SIZE];
    memset(fatBuf, 0, SECTOR_SIZE);

    uint32_t* fatEntries = (uint32_t*)fatBuf;
    fatEntries[0] = 0x0FFFFFF8;   // media descriptor
    fatEntries[1] = 0x0FFFFFFF;   // reserved
    fatEntries[2] = 0x0FFFFFFF;   // root dir: end of chain
    fatEntries[3] = 0x0FFFFFFF;   // TEST.TXT: end of chain

    // write first sector of both FAT copies (2 is just backup)
    writeSector(f, FAT1_START, fatBuf);
    writeSector(f, FAT2_START, fatBuf);

    // Root dir lives at cluster 2, which is sector DATA_START + 0
    uint8_t dirBuf[SECTOR_SIZE];
    memset(dirBuf, 0, SECTOR_SIZE);

    dirEntry_t* entry = (dirEntry_t*)dirBuf;

    // Root contains the file info, clusterHigh/low is address of actual data
    memcpy(entry->name, "TEST", 4);
    memcpy(entry->ext, "TXT", 3);
    entry->attributes = 0x20; // code for normal file
    entry->createTime = 0x0000; // none of this metadata really matters
    entry->createDate = 0x0000;
    entry->accessDate = 0x0000;
    entry->modTime = 0x0000;
    entry->modDate = 0x0000;
    entry->clusterHigh = (TEST_CLUSTER >> 16) & 0xFFFF;
    entry->clusterLow = (TEST_CLUSTER) & 0xFFFF;
    entry->fileSize = strlen(TEST_CONTENT);

    writeSector(f, clusterToSector(ROOT_CLUSTER), dirBuf);

    // write actual data where it is expected to be
    uint8_t fileBuf[SECTOR_SIZE];
    memset(fileBuf, 0, SECTOR_SIZE);
    memcpy(fileBuf, TEST_CONTENT, strlen(TEST_CONTENT));

    writeSector(f, clusterToSector(TEST_CLUSTER), fileBuf);

    fclose(f);

    return 0;
}