// kernel/paging.h - Fixed for 64-bit
#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40

// Page table entry structure
typedef struct {
    uint64_t present    : 1;
    uint64_t write      : 1;
    uint64_t user       : 1;
    uint64_t pwt        : 1;
    uint64_t pcd        : 1;
    uint64_t accessed   : 1;
    uint64_t dirty      : 1;
    uint64_t pat        : 1;
    uint64_t global     : 1;
    uint64_t available  : 3;
    uint64_t address    : 40;
    uint64_t available2 : 11;
    uint64_t nx         : 1;
} __attribute__((packed)) page_entry_t;

// 64-bit paging structures
typedef struct {
    page_entry_t entries[512];
} __attribute__((aligned(4096))) page_table_t;

void paging_init();
void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
void unmap_page(uint64_t virtual_addr);
uint64_t get_physical_addr(uint64_t virtual_addr);
void* allocate_pages(size_t num_pages);
void free_pages(void* virtual_addr, size_t num_pages);

#endif

// kernel/paging.c - Complete 64-bit paging implementation
#include "paging.c"
#include "drivers/vga.h"
#include "drivers/serial.h"

// Page structure levels for 64-bit (4-level paging)
static page_table_t* pml4 __attribute__((aligned(4096)));
static page_table_t* pdpt __attribute__((aligned(4096)));
static page_table_t* page_dir __attribute__((aligned(4096)));
static page_table_t* page_tables[512] __attribute__((aligned(4096)));

// Physical memory management
#define PHYS_MEM_START 0x100000  // 1MB
#define PHYS_MEM_SIZE  0x7F00000 // 127MB (total 128MB)
static uint8_t* phys_bitmap;
static uint64_t phys_mem_base = PHYS_MEM_START;
static uint64_t total_pages;

// Initialize physical memory bitmap
static void init_physical_memory() {
    total_pages = PHYS_MEM_SIZE / PAGE_SIZE;
    phys_bitmap = (uint8_t*)0x500000; // Store bitmap at 5MB
    
    // Clear bitmap
    for (uint64_t i = 0; i < total_pages / 8; i++) {
        phys_bitmap[i] = 0;
    }
    
    serial_write_string("Physical memory initialized: ");
    serial_write_string("pages available\n");
}

// Allocate a physical page
static uint64_t alloc_physical_page() {
    for (uint64_t i = 0; i < total_pages; i++) {
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        
        if (!(phys_bitmap[byte_index] & (1 << bit_index))) {
            phys_bitmap[byte_index] |= (1 << bit_index);
            return phys_mem_base + (i * PAGE_SIZE);
        }
    }
    return 0; // Out of memory
}

// Free a physical page
static void free_physical_page(uint64_t phys_addr) {
    if (phys_addr < phys_mem_base) return;
    
    uint64_t page_index = (phys_addr - phys_mem_base) / PAGE_SIZE;
    if (page_index >= total_pages) return;
    
    uint64_t byte_index = page_index / 8;
    uint64_t bit_index = page_index % 8;
    
    phys_bitmap[byte_index] &= ~(1 << bit_index);
}

// Get page table entry indices from virtual address
static void get_page_indices(uint64_t vaddr, uint64_t* pml4_idx, uint64_t* pdpt_idx, 
                           uint64_t* pd_idx, uint64_t* pt_idx) {
    *pml4_idx = (vaddr >> 39) & 0x1FF;
    *pdpt_idx = (vaddr >> 30) & 0x1FF;
    *pd_idx = (vaddr >> 21) & 0x1FF;
    *pt_idx = (vaddr >> 12) & 0x1FF;
}

// Initialize paging
void paging_init() {
    serial_write_string("Initializing 64-bit paging...\n");
    
    // Initialize physical memory management
    init_physical_memory();
    
    // Allocate page structures
    pml4 = (page_table_t*)alloc_physical_page();
    pdpt = (page_table_t*)alloc_physical_page();
    page_dir = (page_table_t*)alloc_physical_page();
    
    // Clear page structures
    for (int i = 0; i < 512; i++) {
        pml4->entries[i].present = 0;
        pdpt->entries[i].present = 0;
        page_dir->entries[i].present = 0;
        page_tables[i] = 0;
    }
    
    // Set up PML4 -> PDPT
    pml4->entries[0].present = 1;
    pml4->entries[0].write = 1;
    pml4->entries[0].address = ((uint64_t)pdpt) >> 12;
    
    // Set up PDPT -> Page Directory
    pdpt->entries[0].present = 1;
    pdpt->entries[0].write = 1;
    pdpt->entries[0].address = ((uint64_t)page_dir) >> 12;
    
    // Identity map first 2MB (for kernel)
    for (uint64_t i = 0; i < 512; i++) {
        page_tables[0] = (page_table_t*)alloc_physical_page();
        
        // Clear page table
        for (int j = 0; j < 512; j++) {
            page_tables[0]->entries[j].present = 0;
        }
        
        // Set up page directory entry
        page_dir->entries[0].present = 1;
        page_dir->entries[0].write = 1;
        page_dir->entries[0].address = ((uint64_t)page_tables[0]) >> 12;
        
        // Map first 2MB
        for (uint64_t j = 0; j < 512; j++) {
            uint64_t phys_addr = j * PAGE_SIZE;
            page_tables[0]->entries[j].present = 1;
            page_tables[0]->entries[j].write = 1;
            page_tables[0]->entries[j].address = phys_addr >> 12;
        }
        break; // Only need first page table for now
    }
    
    // Load new page directory
    asm volatile("mov %0, %%cr3" : : "r"((uint64_t)pml4));
    
    // Enable paging (should already be enabled in 64-bit mode)
    uint64_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
    
    serial_write_string("64-bit paging initialized successfully\n");
    vga_puts("[OK] 64-bit paging initialized\n");
}

// Map a virtual page to a physical page
void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags) {
    uint64_t pml4_idx, pdpt_idx, pd_idx, pt_idx;
    get_page_indices(virtual_addr, &pml4_idx, &pdpt_idx, &pd_idx, &pt_idx);
    
    // Ensure all intermediate tables exist
    if (!pml4->entries[pml4_idx].present) {
        uint64_t new_pdpt = alloc_physical_page();
        if (!new_pdpt) return; // Out of memory
        
        pml4->entries[pml4_idx].present = 1;
        pml4->entries[pml4_idx].write = 1;
        pml4->entries[pml4_idx].address = new_pdpt >> 12;
        
        // Clear new PDPT
        page_table_t* pdpt_ptr = (page_table_t*)new_pdpt;
        for (int i = 0; i < 512; i++) {
            pdpt_ptr->entries[i].present = 0;
        }
    }
    
    page_table_t* current_pdpt = (page_table_t*)(pml4->entries[pml4_idx].address << 12);
    
    if (!current_pdpt->entries[pdpt_idx].present) {
        uint64_t new_pd = alloc_physical_page();
        if (!new_pd) return;
        
        current_pdpt->entries[pdpt_idx].present = 1;
        current_pdpt->entries[pdpt_idx].write = 1;
        current_pdpt->entries[pdpt_idx].address = new_pd >> 12;
        
        // Clear new PD
        page_table_t* pd_ptr = (page_table_t*)new_pd;
        for (int i = 0; i < 512; i++) {
            pd_ptr->entries[i].present = 0;
        }
    }
    
    page_table_t* current_pd = (page_table_t*)(current_pdpt->entries[pdpt_idx].address << 12);
    
    if (!current_pd->entries[pd_idx].present) {
        uint64_t new_pt = alloc_physical_page();
        if (!new_pt) return;
        
        current_pd->entries[pd_idx].present = 1;
        current_pd->entries[pd_idx].write = 1;
        current_pd->entries[pd_idx].address = new_pt >> 12;
        
        // Clear new PT
        page_table_t* pt_ptr = (page_table_t*)new_pt;
        for (int i = 0; i < 512; i++) {
            pt_ptr->entries[i].present = 0;
        }
    }
    
    page_table_t* current_pt = (page_table_t*)(current_pd->entries[pd_idx].address << 12);
    
    // Set the final page mapping
    current_pt->entries[pt_idx].present = (flags & PAGE_PRESENT) ? 1 : 0;
    current_pt->entries[pt_idx].write = (flags & PAGE_WRITE) ? 1 : 0;
    current_pt->entries[pt_idx].user = (flags & PAGE_USER) ? 1 : 0;
    current_pt->entries[pt_idx].address = physical_addr >> 12;
    
    // Invalidate TLB entry
    asm volatile("invlpg (%0)" : : "r"(virtual_addr));
}

// Unmap a virtual page
void unmap_page(uint64_t virtual_addr) {
    uint64_t pml4_idx, pdpt_idx, pd_idx, pt_idx;
    get_page_indices(virtual_addr, &pml4_idx, &pdpt_idx, &pd_idx, &pt_idx);
    
    if (!pml4->entries[pml4_idx].present) return;
    
    page_table_t* current_pdpt = (page_table_t*)(pml4->entries[pml4_idx].address << 12);
    if (!current_pdpt->entries[pdpt_idx].present) return;
    
    page_table_t* current_pd = (page_table_t*)(current_pdpt->entries[pdpt_idx].address << 12);
    if (!current_pd->entries[pd_idx].present) return;
    
    page_table_t* current_pt = (page_table_t*)(current_pd->entries[pd_idx].address << 12);
    if (!current_pt->entries[pt_idx].present) return;
    
    // Free the physical page
    uint64_t phys_addr = current_pt->entries[pt_idx].address << 12;
    free_physical_page(phys_addr);
    
    // Clear the page table entry
    current_pt->entries[pt_idx].present = 0;
    current_pt->entries[pt_idx].write = 0;
    current_pt->entries[pt_idx].user = 0;
    current_pt->entries[pt_idx].address = 0;
    
    // Invalidate TLB entry
    asm volatile("invlpg (%0)" : : "r"(virtual_addr));
}

// Get physical address from virtual address
uint64_t get_physical_addr(uint64_t virtual_addr) {
    uint64_t pml4_idx, pdpt_idx, pd_idx, pt_idx;
    get_page_indices(virtual_addr, &pml4_idx, &pdpt_idx, &pd_idx, &pt_idx);
    
    if (!pml4->entries[pml4_idx].present) return 0;
    
    page_table_t* current_pdpt = (page_table_t*)(pml4->entries[pml4_idx].address << 12);
    if (!current_pdpt->entries[pdpt_idx].present) return 0;
    
    page_table_t* current_pd = (page_table_t*)(current_pdpt->entries[pdpt_idx].address << 12);
    if (!current_pd->entries[pd_idx].present) return 0;
    
    page_table_t* current_pt = (page_table_t*)(current_pd->entries[pd_idx].address << 12);
    if (!current_pt->entries[pt_idx].present) return 0;
    
    return (current_pt->entries[pt_idx].address << 12) + (virtual_addr & 0xFFF);
}

// Allocate virtual pages
void* allocate_pages(size_t num_pages) {
    static uint64_t next_virtual_addr = 0x10000000; // Start at 256MB
    
    uint64_t start_addr = next_virtual_addr;
    
    for (size_t i = 0; i < num_pages; i++) {
        uint64_t phys_addr = alloc_physical_page();
        if (!phys_addr) {
            // Cleanup already allocated pages
            for (size_t j = 0; j < i; j++) {
                unmap_page(start_addr + (j * PAGE_SIZE));
            }
            return NULL;
        }
        
        map_page(next_virtual_addr, phys_addr, PAGE_PRESENT | PAGE_WRITE);
        next_virtual_addr += PAGE_SIZE;
    }
    
    return (void*)start_addr;
}

// Free virtual pages
void free_pages(void* virtual_addr, size_t num_pages) {
    uint64_t addr = (uint64_t)virtual_addr;
    
    for (size_t i = 0; i < num_pages; i++) {
        unmap_page(addr + (i * PAGE_SIZE));
    }
}