#include "memUtils.h"

void initPMM()
{
    uintptr_t kernelStart = (uintptr_t)&_kernel_start; //use & as we need address of this symbol
    uintptr_t kernelEnd = (uintptr_t)&_kernel_end;

    kernelStart &= ~(PAGE_SIZE - 1); //4096 = 0x1000, 4096 - 1 = 0x0FFF, & ~(0x0FFF) means offset bits are zeroed out, and we round down to 4096
    kernelEnd = (kernelEnd + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); //rounds up by pushing it into the next page then rounding down

    size_t startPage = kernelStart / PAGE_SIZE;
    size_t endPage = kernelEnd / PAGE_SIZE;

    for(size_t i = startPage; i < endPage; i++) //reserve kernel space, probably overlap between this and first MB
    {
        BIT_SET(pmmBitmap, i);
    }
    for(size_t i = endPage; i < totalPages; i++) //mark the rest available
    {
        BIT_CLEAR(pmmBitmap, i);
    }
    for(size_t i = 0; i < 0x100000 / PAGE_SIZE; i++) //reserve first MB as a lot of system stuff is there
    {
        BIT_SET(pmmBitmap, i);
    }
    return;
}

void initHeap()
{
    heapStart = (uintptr_t)pmmAlloc();
    heapPtr = heapStart;
    heapEnd = heapStart + 4096;

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

void* malloc(size_t size)
{
    size = (size + 7) & ~7; //align to 8 bytes

    if(heapPtr + size > heapEnd)
    {
        void* page = pmmAlloc();

        if(!page)
        {
            return 0;
        }

        heapEnd += 4096; //extend heap now that more is allocated
    }

    void* addr = (void*)heapPtr;
    heapPtr += size;

    return addr;
}

void dumpPmmBitmap(int start, int end)
{
    for(size_t i = start; i < end; i++) //prints only selected section
    {
        printHex(pmmBitmap[i], 0);
    }
    return;
}