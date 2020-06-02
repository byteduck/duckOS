#ifndef PAGING_H
#define PAGING_H

extern "C" long kstart;
extern "C" long kend;
extern "C" void load_page_dir(unsigned int*);
void setupPaging();
void pageFaultHandler(struct registers *r);
void exec(uint8_t* prog);

#endif