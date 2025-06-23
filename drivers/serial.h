#ifndef SERIAL_H
#define SERIAL_H

void serial_init();
void serial_write_char(char c);
void serial_write_string(const char* str);

#endif
