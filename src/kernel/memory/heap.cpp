#include <common.h>
#include <stdio.h>
#include <memory/heap.h>

uint8_t heap_space[0x100000]; //1MiB of heap space
uint8_t frames[0x2000];       //16byte Frames, 1 bit each. 1 = used, 0 = free
Frame frbuf;
FrameSet frsbuf;

void init_heap(){
	for(int i = 0; i < 0x2000; i++){
		frames[i] = 0;
	}
}

Frame first_available_frame(){
	for(int i = 0; i < 0x2000; i++){
		for(int j = 0; j < 8; j++){
			if(!(frames[i] >> j & 1)){
				frbuf.num = i*8+j;
				frbuf.set = i;
				frbuf.pos = j;
				return frbuf;
			}
		}
	}
	PANIC("NO_HEAP_SPACE","The heap ran out of memory.",true);
	return frbuf;
}

FrameSet first_available_frameset(uint32_t len){ //len is the amount of 16byte frames, not bytes.
	uint32_t numFrames = 0;
	for(int i = 0; i < 0x2000; i++){
		for(int j = 0; j < 8; j++){
			if(!(frames[i] >> j & 1)){
				if(numFrames == 0){
					frbuf.num = i*8+j;
					frbuf.set = i;
					frbuf.pos = j;
				}
				numFrames++;
			}else{
				numFrames = 0;
			}
			if(numFrames == len){
				frsbuf.start = frbuf;
				frsbuf.len = numFrames;
				return frsbuf;
			}
		}
	}
	PANIC("NO_VAR_SPACE","The heap doesn't have enough consecutive frames to fit a variable.",false);
	print("Variable size: ~"); printHexl(len*16); println("bytes");
	while(true);
}

FrameSet fsalloc(uint32_t len){
	first_available_frameset(len);
	Frame buf;
	for(int i = 0; i < frsbuf.len; i++){
		buf = getFrame(i+frsbuf.start.num);
		frames[buf.set] |= 1 << buf.pos;
	}
	FrameSet fs;
	fs.start = getFrame(frsbuf.start.num);
	fs.len = len;
	return fs;
}

Frame getFrame(uint32_t i){
	Frame f;
	f.set = i/8;
	f.pos = i%8;
	f.num = i;
	return f;
}

Frame falloc(){
	first_available_frame();
	frames[frbuf.set] = frames[frbuf.set] | (1 << frbuf.pos);
	Frame f;
	f.set = frbuf.set;
	f.num = frbuf.num;
	f.pos = frbuf.pos;
	return frbuf;
}

void ffree(Frame f){
	frames[f.set] &= ~(1 << f.pos);
}

void *kmalloc(uint32_t len){
	frsbuf = fsalloc(len/0x10+1);
	return &heap_space[frsbuf.start.num*0x10];
}

uint32_t kfreebuf;

void kfree(void *ptr, uint32_t len){
	kfreebuf = (uint32_t)ptr-(uint32_t)&heap_space;
	kfreebuf/=0x10;
	for(int i = 0; i < len/0x10+1; i++){
		frbuf = getFrame(i+kfreebuf);
		ffree(frbuf);
	}
}

char *String(char *str){
	char* str1 = (char*) kmalloc(strlen(str)+1);
	strcpy(str,str1);
	return str1;
}

void strfree(char *str){
	kfree(str, strlen(str)+1);
}
