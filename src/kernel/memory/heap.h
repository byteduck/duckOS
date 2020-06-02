#ifndef HEAP_H
#define HEAP_H

typedef struct Frame{
	uint32_t num; //The number of the frame.
	uint32_t set; //The set the frame is in (There are 8 frames in a set)
	uint32_t pos; //Which number in the set it is.
} Frame;

typedef struct FrameSet{
	Frame start;
	uint32_t len;
} FrameSet;

void init_heap();
Frame first_available_frame();
Frame falloc();
void ffree(Frame f);
FrameSet first_available_frameset(uint32_t len);
Frame getFrame(uint32_t i);
FrameSet fsalloc();
void *kmalloc(uint32_t len);
void kfree(void *ptr, uint32_t len);
char *String(char *str);
void strfree(char *str);

#endif
