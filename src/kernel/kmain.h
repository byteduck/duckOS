extern "C" void syscall_handler();
extern void load_gdt();
extern "C" uint8_t boot_disk;
void interrupts_init();
void parse_mboot(uint32_t addr);

extern "C" int kmain(uint32_t mbootptr);
void kmain_late();
