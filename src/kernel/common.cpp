#include <common.h>
#include <stdio.h>

void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %1, %0" : : "d" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value){
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value){
    asm volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port){
   uint8_t ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

uint16_t inw(uint16_t port){
   uint16_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

uint32_t inl(uint16_t port){
   uint32_t ret;
   asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

int sgn(int x){
	if(x>0) return 1;
	else return -1;
	return 0;
}

int abs(float x){
	return (int)x;
}

void *memset(void *dest, char val, int count){
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count){
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void numToHexString(uint8_t num, char *str){
	str[1] = nibbleToHexString(num);
	str[0] = nibbleToHexString(num >> 4);
}

char nibbleToHexString(uint8_t num){
	uint8_t tmp = num & 0xF;
	if(tmp < 0xA){
		return tmp+0x30;
	}else{
		return tmp+0x57;
	}
}

char *itoa(int i, char *p, int base){
	char const digit[] = "0123456789";
	int nbcount = 0;
	bool flag = 0;
	int ind;
	switch(base){
		case 10: {
			if (i < 0) {
				*p++ = '-';
				i *= -1;
			}
			int shifter = i;
			do {
				++p;
				shifter = shifter / 10;
			} while (shifter);
			*p = '\0';
			do {
				*--p = digit[i % 10];
				i = i / 10;
			} while (i);
		}
		break;
		//I figured out how to roll base 2 and 16 into one thing... Not sure how efficient it is though
		case 2:
		case 16:
			if(i == 0){p[0] = '0'; p[1] = '\0';}else{
				uint8_t shift = base == 16 ? 4 : 1;
				for(uint32_t a = (base == 16 ? 0xF0000000 : 0x80000000); a > 0; a = a >> shift)
					if((i&a) != 0 || flag){ nbcount++; flag = true;}
				ind = nbcount;
				for(ind > 0; ind--;)
					p[-ind+nbcount-1] = base == 16 ? (nibbleToHexString((i >> (ind*4)) & 0xF)) : (((i >> ind) & 0x1) ? '1' : '0');
				p[nbcount] = '\0';
			}
		break;
	}
	return p;
}

bool isACharacter(uint8_t num){
	return num >= 0x20 && num <= 0x7E;
}

int strlen(const char *str){
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (s - str);
}

bool strcmp(string str1,string str2){
    int i = 0;
	bool flag = false;

    while(str1[i]!='\0' && str2[i]!='\0'){
         if(str1[i]!=str2[i]){
             flag=1;
             break;
         }
         i++;
    }

    return flag == 0 && str1[i] == '\0' && str2[i] == '\0';

}

int indexOf(char c, char *str){
	int i = 0;
	while(str[i] != '\0'){
		if(str[i] == c)
			return i;
		i++;
	}
	return strlen(str);
}

int indexOfn(char c, int n, char *str){ //like indexOf, except ignores n instances of the character
	int i = 0;
	int count = 0;
	while(str[i] != '\0'){
		if(str[i] == c)
			if(count == n)
				return i;
			else
				count++;
		i++;
	}
	return strlen(str);
}

void substr(int i, char *src, char *dest){ //substring exclusive
	memcpy(dest,src,i);
	dest[i] = '\0';
}

void substri(int i, char *src, char *dest){ //substring inclusive
	memcpy(dest,src,i+1);
	dest[i+1] = '\0';
}

void substrr(int s, int e, char *src, char *dest){ //substring exclusive range (end is exclusive, beginning is inclusive)
	memcpy(dest,&src[s],e-s);
	dest[e-s] = '\0';
}

void strcpy(char *src, char *dest){
	memcpy(dest, src, strlen(src));
	dest[strlen(src)] = '\0';
}

int countOf(char c, char *str){ //Returns number of instances of c in str
	int count = 0;
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == c)
			count++;
	}
	return count;
}

bool contains(char *str, char *cont){ //Returns true if str has cont in it.
	int i = 0;
	int contlen = strlen(cont);
	bool flaga = false;
	bool flagb = false;
	while(str[i+contlen-1] != '\0'){
		flagb = true;
		for(int j = 0; j < strlen(cont); j++){
			if(cont[j] != str[j+i])
				flagb = false;
		}
		if(flagb)
			flaga = true;
		i++;
	}
	return flaga;
}

int strToInt(char *str){
	int len = strlen(str);
	int ret = 0;
	for(int i = 0; i < len; i++){
		ret = ret * 10 + (str[i] - '0');
	}
	return ret;
}

void cli(){
	asm volatile("cli");
}

void sti(){
	asm volatile("sti");
}

void toUpper(char *str){
	while(*str != '\0'){
		if(*str >= 'a' && *str <= 'z') *str = *str - ('a' - 'A');
		*str++;
	}
}

char *strcat(char *dest, const char *src){
    uint32_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

extern "C" void __cxa_pure_virtual()
{
	// Do nothing or print an error message.
}
