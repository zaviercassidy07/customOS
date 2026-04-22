#include "memUtils.h"

extern char _kernel_start;
extern char _kernel_end;

uint8_t pmmBitmap[262144];
size_t totalPages = 262144; //1GB of pages (I think)
size_t usedPages = 0;
uint64_t nextFree = 0;

uintptr_t heapStart;
size_t heapSize;
blockHeader_t* heapHead;
blockHeader_t* heapTail;

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
    heapStart = 0x400000; //Heap can be right at bottom with virtual high kernel
    if(heapStart < ((uintptr_t)&_kernel_end + 0x1000) & ~(PAGE_SIZE - 1)) //if its in the kernel
    {
        heapStart = ((uintptr_t)&_kernel_end + 0x1000) & ~(PAGE_SIZE - 1); //putit 4KB above kernel and align it
    }
    

    vMap(heapStart, (uintptr_t)pmmAlloc());

    heapSize = PAGE_SIZE;

    blockHeader_t* block = (blockHeader_t*)heapStart;
    block->size = 0; //placeholder as heapTail never gets size checked
    block->free = 1;
    block->next = NULL;

    heapHead = block;
    heapTail = block;

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
    for(size_t i = 0; i < nextFree; i++)
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

    if(page < nextFree)
    {
        nextFree = page;
    }

    BIT_CLEAR(pmmBitmap, page);
    usedPages--;
    return;
}

//invlpg is "invalidate page", clears cache about that page
void invlpg(uintptr_t addr)
{
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}

void vMap(uintptr_t virt, uintptr_t phys)
{
    uintptr_t allignedPhys = phys & ~(PAGE_SIZE - 1);
    uintptr_t allignedVirt = virt & ~(PAGE_SIZE - 1);

    // first 36 bits show address, 9 for each PML4, PDPT, PD and PT, but we have no PT so those bits are offset too
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    pml4_t* pml4 = (pml4_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | RECUR_INDEX << 12);
    if(!(pml4[pml4Index] & PAGE_PRESENT))
    {
        pdpt_t* newPdpt = (pdpt_t*)pmmAlloc();
        if(newPdpt == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }
        pml4[pml4Index] = (uintptr_t)newPdpt | PAGE_PRESENT | PAGE_WRITABLE;

        uintptr_t* tmp = (uintptr_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | pml4Index << 12);
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }
    }
    pdpt_t* pdpt = (pdpt_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | pml4Index << 12);

    if(!(pdpt[pdptIndex] & PAGE_PRESENT))
    {
        pd_t* newPd = (pd_t*)pmmAlloc();
        if(newPd == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }
        pdpt[pdptIndex] = (uintptr_t)newPd | PAGE_PRESENT | PAGE_WRITABLE;

        uintptr_t* tmp = (uintptr_t*)(RECUR | RECUR_INDEX << 30 | pml4Index << 21 | pdptIndex << 12);
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }

    }
    pd_t* pd = (pd_t*)(RECUR | RECUR_INDEX << 30 | pml4Index << 21 | pdptIndex << 12);

    if(!(pd[pdIndex] & PAGE_PRESENT))
    {
        pt_t* newPt = (pt_t*)pmmAlloc();
        if(newPt == NULL)
        {
            print("\nError: PMM ALLOC FAILED\n");
        }
        pd[pdIndex] = (uintptr_t)newPt | PAGE_PRESENT | PAGE_WRITABLE;

        uintptr_t* tmp = (uintptr_t*)(RECUR | pml4Index << 30 | pdptIndex << 21 | pdIndex << 12 );
        for(int i = 0; i < 512; i++)
        {
            tmp[i] = 0;
        }

    }

    pt_t* pt = (pt_t*)(RECUR | pml4Index << 30 | pdptIndex << 21 | pdIndex << 12);
    pt[ptIndex] = allignedPhys | PAGE_PRESENT | PAGE_WRITABLE;

    invlpg(allignedVirt);
}
void vUMap(uintptr_t virt)
{
    uint64_t pml4Index = (virt >> 39) & 0x1FF; //0x1FF is only first 9 bits
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    pml4_t* pml4 = (pml4_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | RECUR_INDEX << 12);
    pdpt_t* pdpt = (pdpt_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | pml4Index << 12);
    pd_t* pd = (pd_t*)(RECUR | RECUR_INDEX << 30 | pml4Index << 21 | pdptIndex << 12);
    pt_t* pt = (pt_t*)(RECUR | pml4Index << 30 | pdptIndex << 21 | pdIndex << 12);

    uintptr_t phys = pt[ptIndex] & ADDR_MASK;
    pmmFreePage((void*)phys);

    pt[ptIndex] = 0;

    uintptr_t allignedVirt = virt & ~(PAGE_SIZE - 1);
    invlpg(allignedVirt);

    return;
}

int isMapped(uintptr_t virt)
{
    uint64_t pml4Index = (virt >> 39) & 0x1FF;
    uint64_t pdptIndex = (virt >> 30) & 0x1FF;
    uint64_t pdIndex = (virt >> 21) & 0x1FF;
    uint64_t ptIndex = (virt >> 12) & 0x1FF;

    pml4_t* pml4 = (pml4_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | RECUR_INDEX << 12);
    if(!(pml4[pml4Index] & PAGE_PRESENT))
    {
        return 0;
    }

    pdpt_t* pdpt = (pdpt_t*)(RECUR | RECUR_INDEX << 30 | RECUR_INDEX << 21 | pml4Index << 12);
    if(!(pdpt[pdptIndex] & PAGE_PRESENT))
    {
        return 0;
    }

    pd_t* pd = (pd_t*)(RECUR | RECUR_INDEX << 30 | pml4Index << 21 | pdptIndex << 12);
    if(!(pd[pdIndex] & PAGE_PRESENT))
    {
        return 0;
    }

    pt_t* pt = (pt_t*)(RECUR | pml4Index << 30 | pdptIndex << 21 | pdIndex << 12);
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

    blockHeader_t* cur = heapHead;
    blockHeader_t* block = NULL;

    //find the block that our data will be assigned to
    while(cur)
    {
        if(cur == heapTail)
        {
            block = cur;
            break;
        }
        if(cur->free == 1 && cur->size >= size)
        {
            //split logic
            if(cur->size >= size + sizeof(blockHeader_t) + 8)
            {
                blockHeader_t* newBlock = (blockHeader_t*)((uintptr_t)cur + sizeof(blockHeader_t) + size);
                newBlock->free = 1;

                newBlock->next = cur->next;
                cur->next = newBlock;

                newBlock->size = cur->size - size - sizeof(blockHeader_t);
                cur->size = size;
            }


            block = cur;
            break;
        }
        cur = cur->next;
    }

    size_t totalSize = sizeof(blockHeader_t) + size;
    if(block == heapTail)
    {
        totalSize += sizeof(blockHeader_t); //if we're creating a new block on end of chain
    }
    uintptr_t blockEnd = (uintptr_t)block + totalSize;
    while(blockEnd >= heapStart + heapSize)
    {
        uintptr_t phys = (uintptr_t)pmmAlloc();
        if(!phys)
        {
            print("\nPMM ALLOC FAIL\n");
            return NULL;
        }
        vMap(heapStart + heapSize, phys);
        heapSize += PAGE_SIZE;
    }

    block->free = 0;

    if(block == heapTail)
    {
        if((uintptr_t)heapTail & ~0xFFF == 0x4FF000ULL)
        {
            print("About to page fault");
        }
        blockHeader_t* newBlock = (blockHeader_t*)((uintptr_t)block + sizeof(blockHeader_t) + size);
		
        newBlock->free = 1;
        newBlock->size = 0; //zero is used as placeholder, size is never checked for heapTail anyway
        newBlock->next = NULL;

        block->next = newBlock;
        block->size = size;

        heapTail = newBlock;
    }

    return(void*)(block + 1);
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

    if(heapHead == heapTail)
    {
        return;
    }

    while(cur && cur->next)
    {
        if(cur->free == 1 && cur->next->free == 1)
        {
            cur->size += sizeof(blockHeader_t) + cur->next->size;
            cur->next = cur->next->next;

            if(cur->next == heapTail)
            {
                heapTail = cur;
            }
        }
        else
        {
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