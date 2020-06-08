#ifndef SHELL_H
#define SHELL_H
#include <kernel/filesystem/Ext2.h>
#include <kernel/filesystem/InodeRef.h>

class Shell {
public:
	Shell();
	void shell();
	void command_eval(char *cmd, char *args);
private:
	char cmdbuf[256] = {0};
	char argbuf[256] = {0};
	char dir_buf[256] = {0};
	bool exitShell = false;
	DC::shared_ptr<InodeRef> current_dir;
};

#endif
