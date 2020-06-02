void drawLine(int x1, int y1, int x2, int y2, char color){
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;
	
	dx=x2-x1;
	dy=y2-y1;
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	x=dyabs>>1;
	y=dxabs>>1;
	px=x1;
	py=y1;

	if(dxabs>=dyabs){
		for(i=0;i<dxabs;i++){
			y+=dyabs;
			if(y>=dxabs){
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			putPixel(px,py,color);
		}
	  }else{
		for(i=0;i<dyabs;i++){
			x+=dxabs;
			if(x>=dyabs){
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			putPixel(px,py,color);
		}
	}
}

void drawMonoBitmap(const char* bmp, int width, int height, int px, int py, char color){
	int apos = 0;
	int spos = 7;
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			if(!((bmp[apos] >> spos) & 0x01))
				putPixel(x+px,y+py,color);
			spos--;
			if(spos < 0){
				spos = 7;
				apos++;
			}
		}
	}
}