#ifndef SHELL_H
#define SHELL_H
#include <filesystem/Ext2.h>

void initShell(Filesystem *fsp);
void shell();
static void command_eval(char *cmd, char *args);

#endif
