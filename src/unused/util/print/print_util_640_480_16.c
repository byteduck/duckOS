void putch_color(char c, char color){
	unsigned char* vidmem = (char*)0xA0000;
	if(c == '\r'){
		xpos = 0;
	}else if(c == '\n'){
		xpos = 0;
		ypos++;
	}else{
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if(((font_8x8[c][y] >> x)  & 0x01))
					putPixel(xpos*8+x,ypos*8+y] = color;
			}
		}
		xpos++;
		if(xpos >= 80){
			ypos++;
			xpos = 0;
			if(ypos >= 60){
				ypos = 0;
			}
		}
	}
}

void clearScreen(){
	unsigned char* vidmem = (char*)0xA0000;
	for(int y=0; y<480; y++){
		for(int x=0; x<640; x++){
			vidmem[(x+(y*640))] = 0x00;
		}
	}
	xpos = 0;
	ypos = 0;
}

void center_print(char* c, char color){
	center_print_base(c,color,80);
}