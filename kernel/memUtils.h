#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#define NULL ((void*)0)

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

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;
typedef unsigned long long uintptr_t;

typedef uint64_t pml4_t;
typedef uint64_t pdpt_t;
typedef uint64_t pd_t;
typedef uint64_t pt_t;

extern pml4_t pml4[1024];

extern char _kernel_start;
extern char _kernel_end;

extern void printHex(uintptr_t str, int pfx);

static uint8_t pmmBitmap[262144];
static size_t totalPages = 262144; //1GB of pages (I think)
static size_t usedPages = 0;

static uintptr_t heapStart;
static uintptr_t heapEnd;
static uintptr_t heapPtr;

void initPMM();
void initHeap();

void* pmmAlloc();
void pmmFreePage(void* addr);

void vMap(uintptr_t virt, uintptr_t phys);
void vUMap(uintptr_t virt);

void* malloc(size_t size);

void dumpPmmBitmap(int start, int end);

#endif