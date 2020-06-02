void putPixel(int x, int y, char color){
	if(x <= 320 && y <= 200){
		char* vidmem = (char*)0xA0000;
		vidmem[x+y*320] = color;
	}
}

void setPalette(char id, char r, char g, char b){
	outb(0x3c8,id);
	outb(0x3c9,r);
	outb(0x3c9,g);
	outb(0x3c9,b);
}