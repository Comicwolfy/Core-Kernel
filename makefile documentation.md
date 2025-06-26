To incorporate new extensions, place the C source files (e.g., my_new_feature.c) into the src/extensions/ directory. Next, modify the Makefile by adding the new files path to the C_SOURCES variable. For example, if adding my_new_feature.c, locate the C_SOURCES definition and append the new file:
Makefile

C_SOURCES += src/extensions/extention.c

After this modification, a standard make command will automatically compile and link the new extension into core kernel.
