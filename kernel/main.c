void kernel_main() {
    const char* msg = "Welcome to my kernel x64!\n";
    char* video = (char*) 0xb8000;

    for (int i = 0; msg[i] != '\0'; i++) {
        video[i * 2] = msg[i];
        video[i * 2 + 1] = 0x07;
    }

    while (1);
}
