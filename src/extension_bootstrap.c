// File: src/extension_bootstrap.c

#include "base_kernel.h"

// This function iterates through all registered extension functions
// and calls them, allowing each extension to self-register with the kernel.
void initialize_all_extensions(void) {
    terminal_writestring("Initializing extensions...\n"); // Uses terminal_writestring from kernel.c via base_kernel.h

    // Iterate through the array of function pointers.
    // The linker places pointers to all functions marked with
    // __attribute__((section(".ext_register_fns")))
    // between __ext_register_start and __ext_register_end.
    for (extension_auto_register_func_t* func_ptr = __ext_register_start;
         func_ptr < __ext_register_end;
         func_ptr++) {
        if (*func_ptr) { // Safety check, though typically not needed for this pattern
            (*func_ptr)(); // Call the extension's self-registration function
        }
    }
    terminal_writestring("All linked extensions initialized.\n\n"); // Uses terminal_writestring from kernel.c via base_kernel.h
}
