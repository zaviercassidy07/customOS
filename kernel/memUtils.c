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

    return;
}

void initHeap()
{
    heapStart = 0x400000; //4MB, should be safe for now, just outside mapped region
    if(heapStart < ((uintptr_t)&_kernel_end + 0x1000) & ~0xFFF) //if its in the kernel
    {
        heapStart = ((uintptr_t)&_kernel_end + 0x1000) & ~0xFFF; //putit 4KB above kernel and align it
    }
    heapPtr = heapStart;
    heapEnd = heapStart;

    vMap(heapStart, (uintptr_t)pmmAlloc());

    heapEnd += PAGE_SIZE;

    heapHead = NULL;
    heapTail = NULL;

    return;
}

void* pmmAlloc()
{
    for(size_t i = nextFree; i < totalPages; i++)
    {
        if(!BIT_TEST(pmmBitmap, i))
        {
            nextFree = i + 1;
            BIT_SET(pmmBitmap, i);
            usedPages++;
            return (void*)(i * PAGE_SIZE);
        }
    }
    for(size_t i = 0; i < totalPages; i++)
    {
        if(!BIT_TEST(pmmBitmap, i))
        {
            nextFree = i + 1;
            BIT_SET(pmmBitmap, i);
            usedPages++;
            return (void*)(i * PAGE_SIZE);
        }
    }

    print("\nPMM FULL\n");
    printHex(usedPages, 0);
    print(" \\ ");
    printHex(totalPages, 0);
    print("\n");

    return NULL;
}

void pmmFreePage(void* addr)
{
    size_t page = (uintptr_t)addr / PAGE_SIZE;

    if(nextFree < page)
    {
        nextFree = page;
    }

    BIT_CLEAR(pmmBitmap, page);
    usedPages--;
    return;
}

void reloadCr3()
{
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pml4Phys) : "memory");
}

//invlpg is "invalidate page", clears cache about that page
void invlpg(uintptr_t addr)
{
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}

uintptr_t* tmpMap(uintptr_t virt, uint64_t page)
{
    if(page == 0)
    {
        page = TMP0;
    }
    else if(page == 1)
    {
        page = TMP1;
    }
    else if(page == 2)
    {
        page = TMP2;
    }
    else
    {
        page = TMP3;
    }

    uint64_t pml4Index = (page >> 39) & 0x1FF;
    uint64_t pdptIndex = (page >> 30) & 0x1FF;
    uint64_t pdIndex = (page >> 21) & 0x1FF;
    uint64_t ptIndex = (page >> 12) & 0x1FF;

    pdpt_t* pdpt = (pdpt_t*)(pml4Phys[pml4Index] & ADDR_MASK);
    pd_t* pd = (pd_t*)(pdpt[pdptIndex] & ADDR_MASK);
    pt_t* pt = (pt_t*)(pd[pdIndex] & ADDR_MASK);

    pt[ptIndex] = (virt & ~0xFFF) | PAGE_PRESENT | PAGE_WRITABLE;

    invlpg((uintptr_t)page);

    return (uintptr_t*)page;
}

void vMap(uintptr_t virt, uintptr_t phys)
{
    if(virt < 0x400000ULL)
    {
        print("\nERROR: Low address\n");
        return;
    }

    uintptr_t allignedPhys = phys & ~0xFFF;
    uintptr_t allignedVirt = virt & ~0xFFF;

    // first 36 bits show address, 9 for each PML4, PDPT, PD and PT, but we have no PT so those bits are offset too
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    if(!(pml4Phys[pml4Index] & PAGE_PRESENT))
    {
        pdpt_t* newPdpt = (pdpt_t*)pmmAlloc();
        if(newPdpt == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }

        uintptr_t* tmp = tmpMap((uintptr_t)newPdpt, 0);
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }
        pml4Phys[pml4Index] = (uintptr_t)newPdpt | PAGE_PRESENT | PAGE_WRITABLE;
        reloadCr3();
    }
    pdpt_t* pdpt = (pdpt_t*)tmpMap((pml4Phys[pml4Index] & ADDR_MASK), 1);

    if(!(pdpt[pdptIndex] & PAGE_PRESENT))
    {
        pd_t* newPd = (pd_t*)pmmAlloc();
        if(newPd == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }

        uintptr_t* tmp = tmpMap((uintptr_t)newPd, 0);
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }

        pdpt[pdptIndex] = (uintptr_t)newPd | PAGE_PRESENT | PAGE_WRITABLE;
        reloadCr3();
    }
    pd_t* pd = (pd_t*)tmpMap((pdpt[pdptIndex] & ADDR_MASK), 2);

    if(!(pd[pdIndex] & PAGE_PRESENT))
    {
        pt_t* newPt = (pt_t*)pmmAlloc();
        if(newPt == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }

        uintptr_t* tmp = tmpMap((uintptr_t)newPt, 0);
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }

        pd[pdIndex] = (uintptr_t)newPt | PAGE_PRESENT | PAGE_WRITABLE;
        reloadCr3();
    }

    pt_t* pt = (pt_t*)tmpMap((pd[pdIndex] & ADDR_MASK), 3); // the & part keeps only the physical address

    pt[ptIndex] = allignedPhys | PAGE_PRESENT | PAGE_WRITABLE;

    invlpg(allignedVirt);
}
void vUMap(uintptr_t virt)
{
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    pdpt_t* pdpt = (pdpt_t*)(pml4Phys[pml4Index] & ADDR_MASK);
    pd_t* pd = (pd_t*)(pdpt[pdptIndex] & ADDR_MASK);
    pt_t* pt = (pt_t*)(pd[pdIndex] & ADDR_MASK);

    uintptr_t phys = pt[ptIndex] & ADDR_MASK;
    pmmFreePage((void*)phys);

    pt[ptIndex] = 0;

    uintptr_t allignedVirt = virt & ~0xFFF;
    invlpg(allignedVirt);

    return;
}

int isMapped(uintptr_t virt)
{
    if(virt < 0x400000ULL)
    {
        return 1; //this region is identity mapped
    }

    uint64_t pml4Index = (virt >> 39) & 0x1FF;
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    if(!(pml4Phys[pml4Index] & PAGE_PRESENT))
    {
        return 0;
    }

    pdpt_t* pdpt = (pdpt_t*)tmpMap(pml4Phys[pml4Index] & ADDR_MASK, 1);
    if(!(pdpt[pdptIndex] & PAGE_PRESENT))
    {
        return 0;
    }

    pd_t* pd = (pd_t*)tmpMap(pdpt[pdptIndex] & ADDR_MASK, 2);
    if(!(pd[pdIndex] & PAGE_PRESENT))
    {
        return 0;
    }

    pt_t* pt = (pt_t*)tmpMap(pd[pdIndex] & ADDR_MASK, 3);
    if(!(pt[ptIndex] & PAGE_PRESENT))
    {
        return 0;
    }

    return 1;
}

void* malloc(size_t size)
{
    mergeBlocks();

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

    size_t totalSize = sizeof(blockHeader_t) + size;

    uintptr_t blockStartPage = heapPtr & ~0xFFFULL;
    uintptr_t blockEndAddr   = heapPtr + totalSize - 1;
    uintptr_t blockLastPage  = blockEndAddr & ~0xFFFULL;

    //Make sure page with header is mapped
    for(uintptr_t page = blockStartPage; page <= blockLastPage; page += PAGE_SIZE)
    {
        if(isMapped(page) == 0 || page == 0x3300000 || page == 0x1FCFF000) //no idea why but these pages here seems to always dodge being allocated
        {
            void* phys = pmmAlloc();
            if(!phys)
            {
                print("\nPMM ALLOC FAILED\n");
                return NULL;
            }
            vMap(page, (uintptr_t)phys);

            if(page + PAGE_SIZE > heapEnd)
            {
                heapEnd = page + PAGE_SIZE;
            }
        }
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

void mergeBlocks()
{
    blockHeader_t* cur = heapHead;
    blockHeader_t* oldCur = heapHead;

    if(heapHead == heapTail)
    {
        return;
    }

    cur = cur->next;

    while(cur)
    {
        if(cur->free == 1 && oldCur->free == 1)
        {
            oldCur->size = oldCur->size + cur->size + sizeof(blockHeader_t);
            oldCur->next = cur->next;

            cur = cur->next;
        }
        else
        {
            oldCur = cur;
            cur = cur->next;
        }
    }
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