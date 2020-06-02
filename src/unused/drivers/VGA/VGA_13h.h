int VGA_WIDTH = 320;
int VGA_HEIGHT = 200;
char VGA_MODE = 0x13;
void putPixel(int x, int y, char color);
void setPalette(char id, char r, char g, char b);
#include "VGA_13h.c"