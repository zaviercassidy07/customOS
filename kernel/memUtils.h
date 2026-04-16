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

#define TMP0 0x0000ULL
#define TMP1 0x1000ULL
#define TMP2 0x2000ULL
#define TMP3 0x3000ULL

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;
typedef unsigned long long uintptr_t;

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

extern pml4_t pml4Phys[1024];

extern char _kernel_start;
extern char _kernel_end;

extern void printHex(uintptr_t str, int pfx);

uint8_t pmmBitmap[262144];
size_t totalPages = 262144; //1GB of pages (I think)
size_t usedPages = 0;
uint64_t nextFree = 0;

uintptr_t heapStart;
uintptr_t heapEnd;
uintptr_t heapPtr;
blockHeader_t* heapHead;
blockHeader_t* heapTail;

void initPMM();
void initHeap();

void* pmmAlloc();
void pmmFreePage(void* addr);

void reloadCr3();
uintptr_t* tmpMap(uintptr_t virt, uint64_t page);

void vMap(uintptr_t virt, uintptr_t phys);
void vUMap(uintptr_t virt);
int isMapped(uintptr_t virt);

void* malloc(size_t size);
void free(uintptr_t addr);

void mergeBlocks();

void dumpPmmBitmap(int start, int end);

extern void print(char* string);
extern void printLineHex(uintptr_t str, int pfx, uint8_t line);
#endif