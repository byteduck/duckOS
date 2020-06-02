#ifndef SHELL_H
#define SHELL_H
#include <filesystem/ext2.h>

void initShell(filesystem_t *fsp);
void shell();
static void command_eval(char *cmd, char *args);

#endif
