#include "memUtils.h"

void initPMM()
{
    for(size_t i = 0; i < 256; i++) //this needs to change if kernel is bigger than 1MB
    {
        BIT_SET(pmmBitmap, i);
    }
    for(size_t i = 256; i < totalPages; i++)
    {
        BIT_CLEAR(pmmBitmap, i);
    }
    return;
}

void* pmmAlloc()
{
    for(size_t i = 0; i < totalPages; i++)
    {
        if(!BIT_TEST(pmmBitmap, i))
        {
            BIT_SET(pmmBitmap, i);
            usedPages++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmmFreePage(void* addr)
{
    size_t page = (uintptr_t)addr / PAGE_SIZE;
    BIT_CLEAR(pmmBitmap, page);
    usedPages--;
    return;
}

void dumpPmmBitmap(int start, int end)
{
    for(size_t i = start; i < end; i++) //prints only selected section
    {
        printHex(pmmBitmap[i], 0);
    }
    return;
}