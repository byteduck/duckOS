#include <kstddef.h>
#include <kstdio.h>
#include <keyboard.h>

bool shift = 0;
char kbdbuf[256];
volatile bool input_mode = false;
volatile bool input_done = false;
uint8_t input_i;
volatile bool shell_mode = false;

unsigned char kbdus[256] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'\"', '~', 0, '|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M',
	'<', '>', '?', 0, '*',
	0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0
	
};

void keyboard_handler(struct registers *r){
    uint8_t scancode;
	
    scancode = inb(0x60);
	
    if (scancode & 0x80){
		if(scancode == 0xAA)
			shift = false;
    }else{
		if(scancode == 0x2a){shift = 1;return;}
		if(scancode == 0x1c){
			putch('\n');
			if(input_mode)
				input_done = true;
				return;
			}
		if(scancode == 0x0e){
			if(shell_mode){
				if(input_i > 0){
					backspace();
				}
			}else{
				backspace();
			}
			
			if(input_mode && input_i > 0)
				input_i--;
			
			return;
		}
		if(isACharacter(kbdus[scancode])){
			if(shift){
				putch(kbdus[scancode+90]);
				if(input_mode){
					kbdbuf[input_i] = kbdus[scancode+90];
					input_i++;
				}
			}else{
				putch(kbdus[scancode]);
				if(input_mode){
					kbdbuf[input_i] = kbdus[scancode];
					input_i++;
				}
			}
		}
    }
}

void getInput(){ //Puts characters into kbdbuf until enter is pressed or you reach 255 characters
	input_mode = true;
	input_done = false;
	input_i = 0;
	while(!input_done && input_i < 255){}
	kbdbuf[input_i] = '\0';
	input_mode = false;
}