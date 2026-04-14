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
    for(size_t i = 0; i < 0x200000 / PAGE_SIZE; i++) //reserve first 2MB as a lot of system stuff is there: HACK SOLUTION, NEED TO MAKE DYNAMIC
    {
        BIT_SET(pmmBitmap, i);
    }
    printHex(kernelEnd, 1);
    print("\n");
    return;
}

void initHeap()
{
    heapStart = 0x100000; //1MB, should be safe for now
    if(heapStart < ((uintptr_t)&_kernel_end + 0x1000) & ~0xFFF) //if its in the kernel
    {
        heapStart = ((uintptr_t)&_kernel_end + 0x1000) & ~0xFFF; //putit 4KB above kernel and align it
    }
    heapPtr = heapStart;
    heapEnd = heapStart + 4096;

    vMap(heapStart, (uintptr_t)pmmAlloc());

    heapHead = NULL;

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

void vMap(uintptr_t virt, uintptr_t phys)
{
    print("\nMapping V=");
    printHex(virt, 1);
    print("\nMapping P=");
    printHex(phys, 1);
    print("\n");

    // first 36 bits show address, 9 for each PML4, PDPT, PD and PT, but we have no PT so those bits are offset too
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    if(!(pml4[pml4Index] & PAGE_PRESENT))
    {
        pdpt_t* newPdpt = (pdpt_t*)pmmAlloc();
        for(int i = 0; i < 512; i++)
        {
            newPdpt[i] = 0;
        }
        pml4[pml4Index] = (uintptr_t)newPdpt | PAGE_PRESENT | PAGE_WRITABLE;
    }
    pdpt_t* pdpt = (pdpt_t*)(pml4[pml4Index] & ADDR_MASK);

    if(!(pdpt[pdptIndex] & PAGE_PRESENT))
    {
        pd_t* newPd = (pd_t*)pmmAlloc();
        for(int i = 0; i < 512; i++)
        {
            newPd[i] = 0;
        }
        pdpt[pdptIndex] = (uintptr_t)newPd | PAGE_PRESENT | PAGE_WRITABLE;
    }
    pd_t* pd = (pd_t*)(pdpt[pdptIndex] & ADDR_MASK);

    if(!(pd[pdIndex] & PAGE_PRESENT))
    {
        pt_t* newPt = (pt_t*)pmmAlloc();
        for(int i = 0; i < 512; i++)
        {
            newPt[i] = 0;
        }
        pd[pdIndex] = (uintptr_t)newPt | PAGE_PRESENT | PAGE_WRITABLE;
    }
    pt_t* pt = (pt_t*)(pd[pdIndex] & ADDR_MASK); // the & part keeps only the physical address

    uintptr_t allignedPhys = phys & ~0xFFF;
    uintptr_t allignedVirt = virt & ~0xFFF;
    pt[ptIndex] = allignedPhys | PAGE_PRESENT | PAGE_WRITABLE;

    //invlpg is "invalidate page", clears cache about that page
    __asm__ volatile("invlpg (%0)" ::"r"(allignedVirt) : "memory");
}
void vUMap(uintptr_t virt)
{
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    pdpt_t* pdpt = (pdpt_t*)(pml4[pml4Index] & ADDR_MASK);
    pd_t* pd = (pd_t*)(pdpt[pdptIndex] & ADDR_MASK);
    pt_t* pt = (pt_t*)(pd[pdIndex] & ADDR_MASK);

    uintptr_t phys = pt[ptIndex] & ADDR_MASK;
    pmmFreePage((void*)phys);

    pt[ptIndex] = 0;

    uintptr_t allignedVirt = virt & ~0xFFF;
    __asm__ volatile("invlpg (%0)" ::"r"(allignedVirt) : "memory");

    return;
}

void* malloc(size_t size)
{
    size = (size + 7) & ~7; //align to 8 bytes

    blockHeader_t* cur = heapHead; //create pointer to start of chain

    while (cur) // while its not null
    {
        if(cur->free == 1 && cur->size >= size) //if its free and bit enough
        {
            if(cur->size >= 32 + size) //if at least 8 bits after header, create new header so memory is usable
            {
                blockHeader_t* split = (blockHeader_t*)((uintptr_t)cur + sizeof(blockHeader_t) + size);
                split->free = 1;
                split->size = cur->size - sizeof(blockHeader_t) - size;
                
                split->next = cur->next;
                cur->next = split;
                
                if(heapTail == cur)
                {
                    heapTail = split;
                }

                cur->size = size;
            }
            cur->free = 0; //now its not free

            return (void*)(cur + 1); //return this one
        }
        cur = cur->next; //if its not free or big enough, move to next in chain
    }

    size_t totalSize = sizeof(blockHeader_t) + size; //total size is header plus requested size
    while(heapPtr + totalSize > heapEnd) //if too big
    {
        void* page = pmmAlloc(); //get more
        if(!page)
        {
            return NULL;
        }

        vMap(heapEnd, (uintptr_t)page);
        heapEnd += 4096;
    }

    //if no existing block is good, make new one at heapPtr
    blockHeader_t* block = (blockHeader_t*)heapPtr; 
    block->size = size;
    block->free = 0;
    block->next = NULL;

    if(!heapHead) //if there is no beginning of chain, this is now beginning of chain
    {
        heapHead = block;
        heapTail = block;
    }
    else
    {
        heapTail->next = block; //if chain does exist, add this to end
        heapTail = block;
    }

    heapPtr += totalSize; //move heapPtr

    void* addr = (void*)(block + 1); //return the address just past the header

    return addr;
}

void free(uintptr_t addr)
{
    blockHeader_t* cur = (blockHeader_t*)(addr - sizeof(blockHeader_t));
    cur->free = 1;
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