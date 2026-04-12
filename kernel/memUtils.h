#ifndef MEM_UTILS_H
#define MEM_UTILS_H

#define NULL ((void*)0)

//we'll use 4kb pages for this
#define PAGE_SIZE 4096 

//use macros as they're way faster than function calls, b is bitmap and i is pageNum
#define BIT_SET(b, i) (b[(i)/8] |= (1 << ((i)%8)))
#define BIT_CLEAR(b, i) (b[(i)/8] &= ~(1 << ((i)%8)))
#define BIT_TEST(b, i) (b[(i)/8] & (1 << ((i)%8)))

typedef unsigned char uint8_t;
typedef unsigned long long size_t;
typedef unsigned long long uintptr_t;

extern char _kernel_start;
extern char _kernel_end;

extern void printHex(uintptr_t str, int pfx);

static uint8_t pmmBitmap[262144]; //placeholder, we want it above kernel, but as close to as possible
static size_t totalPages = 262144; //1GB of pages (I think)
static size_t usedPages = 0;

static uintptr_t heapStart;
static uintptr_t heapEnd;
static uintptr_t heapPtr;

void initPMM();
void initHeap();

void* pmmAlloc();
void pmmFreePage(void* addr);

void* malloc(size_t size);

void dumpPmmBitmap(int start, int end);

#endif