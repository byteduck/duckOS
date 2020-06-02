unsigned char* vidmem = (char*)0xA0000;
void putch_color(char c, char color){
	if(c == '\r'){
		xpos = 0;
	}else if(c == '\n'){
		xpos = 0;
		ypos++;
	}else{
		for(int x = 0; x < 8; x++){
			for(int y = 0; y < 8; y++){
				if(((font_8x8[c][y] >> x)  & 0x01))
					vidmem[xpos*8+x+(ypos*320*8)+y*320] = color;
			}
		}
		xpos++;
		if(xpos >= 40){
			ypos++;
			xpos = 0;
			if(ypos >= 25){
				ypos = 0;
			}
		}
	}
}

void clearScreen(){
	for(int y=0; y<200; y++){
		for(int x=0; x<320; x++){
			vidmem[(x+(y*320))] = 0;
		}
	}
	xpos = 0;
	ypos = 0;
}

void center_print(char* c, char color){
	center_print_base(c,color,40);
}