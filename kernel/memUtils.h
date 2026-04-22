#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#include "types.h"

#include "printUtils.h"

//we'll use 4kb pages for this
#define PAGE_SIZE 4096 

//use macros as they're way faster than function calls, b is bitmap and i is pageNum
#define BIT_SET(b, i) (b[(i)/8] |= (1 << ((i)%8)))
#define BIT_CLEAR(b, i) (b[(i)/8] &= ~(1 << ((i)%8)))
#define BIT_TEST(b, i) (b[(i)/8] & (1 << ((i)%8)))

//define page attributes
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define ADDR_MASK 0xFFFFFFFFFF000

#define RECUR 0xFFFFFF0000000000ULL
#define RECUR_INDEX 510ULL

typedef uint64_t pml4_t;
typedef uint64_t pdpt_t;
typedef uint64_t pd_t;
typedef uint64_t pt_t;

typedef struct blockHeader_t
{
    size_t size; 
    int free;
    struct blockHeader_t* next;
} blockHeader_t;

void initPMM();
void initHeap();

void* pmmAlloc();
void pmmFreePage(void* addr);

void invlpg(uintptr_t addr);

void vMap(uintptr_t virt, uintptr_t phys);
void vUMap(uintptr_t virt);
int isMapped(uintptr_t virt);

void* malloc(size_t size);
void free(uintptr_t addr);

void mergeBlocks();

void dumpPmmBitmap(int start, int end);

#endif