ENTRY(_start)

. = 0x100000;

SECTIONS {
    .text ALIGN (0x1000) : {
        *(.text)
    }

    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss ALIGN (0x1000) : {
        *(.bss)
    }

    .ext_register_fns ALIGN(4) : {
        _ext_register_start = .;
        *(.ext_register_fns)
        _ext_register_end = .;
    }
}
