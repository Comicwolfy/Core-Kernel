#include "paging.h"

#define PAGE_SIZE 4096
#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2

static uint64_t page_directory[512] __attribute__((aligned(4096)));

void paging_init() {
    for (int i = 0; i < 512; i++) {
        page_directory[i] = 0;
    }

    for (int i = 0; i < 512; i++) {
        page_directory[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITE;
    }

    uint64_t cr3 = (uint64_t)page_directory;
    __asm__ volatile ("mov %0, %%cr3" :: "r"(cr3));
    __asm__ volatile (
        "mov %%cr0, %%rax\n"
        "or $0x80000000, %%rax\n"
        "mov %%rax, %%cr0\n"
        :::"rax"
    );
}
