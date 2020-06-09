#ifndef STDIO_H
#define STDIO_H

#include <kernel/kstddef.h>
#include <common/string.h>

#define ASSERT(cond) \
if(!(cond)) { \
  PANIC("Assertion failed:", __FILE__ " at line " STR(__LINE__), true); \
}

void print_color(char* c, char color);
void println_color(char* c, char color);
void putch(char c);
void print(char* c);
void println(char* c);
void setColor(char color);
void center_print_base(char* c, char color, int width);
void printHex(uint8_t num);
void printHexw(uint16_t num);
void printHexl(uint32_t num);
void printNum(int num);
void printf(char *fmt, ...);
void backspace();
void PANIC(char *error, char *msg, bool hang);
void putch_color(char c, char color);
void clearScreen();
void setAllColor(char color);
void center_print(char* c, char color);
void update_cursor();
void scroll();
void set_vidmem(uint8_t* memloc);
#define SCREEN_CHAR_WIDTH 80
#define SCREEN_CHAR_HEIGHT 25

#endif
