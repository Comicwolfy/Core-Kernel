#include "base_kernel.h"

void initialize_all_extensions(void) {
    terminal_writestring("Initializing extensions...\n");

    for (extension_auto_register_func_t* func_ptr = __ext_register_start;
         func_ptr < __ext_register_end;
         func_ptr++) {
        if (*func_ptr) {
            (*func_ptr)();
        }
    }
    terminal_writestring("All linked extensions initialized.\n\n");
}
