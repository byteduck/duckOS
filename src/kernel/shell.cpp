#include <common.h>
#include <keyboard.h>
#include <kstdio.h>
#include <shell.h>
#include <filesystem/Ext2.h>
#include <memory/kliballoc.h>
#include <tasking/tasking.h>
#include <tasking/elf.h>
#include <pci/pci.h>
#include <memory/paging.h>
#include <device/ide.h>

char cmdbuf[256];
char argbuf[256];
char dirbuf[512];
char dirbuf2[512];
bool exitShell = false;
//file_t currentDir = {}, fileBuf = {};
extern bool shell_mode;
extern char kbdbuf[256];
Filesystem *shellfs;
extern bool tasking_enabled;

void initShell(Filesystem *fsp){
	shellfs = fsp;
	//shellfs->getFile("/",&currentDir,shellfs);
	dirbuf[0] = '/';
	dirbuf[1] = '\0';
}

void dummy(){
	while(1);
}

void shell(){
	while(!exitShell){
		printf("kernel:%s$ ", dirbuf);
		shell_mode = true;
		getInput();
		shell_mode = false;
		setColor(0x07);
		substr(indexOf(' ', kbdbuf), kbdbuf, cmdbuf);
		if(indexOf(' ', kbdbuf)+1 <= strlen(kbdbuf)){
			substrr(indexOf(' ', kbdbuf)+1, strlen(kbdbuf), kbdbuf, argbuf);
		}else{
			argbuf[0] = 0;
		}
		command_eval(cmdbuf, argbuf);
		setColor(0x0f);
	}
	__kill__();
}

/*bool findAndExecute(char *cmd, bool wait){
	file_t *file = (file_t *)kmalloc(sizeof(file_t));
	if(shellfs->getFile(cmd, file, shellfs) && !file->isDirectory){
		if(file->sectors*512 > 0x1000)
			printf("Executable too large.\n");
		else{
			shellfs->read(file, prog, shellfs);
			process_t *proc = createProcess(cmd, (uint32_t)progx);
			uint32_t pid = addProcess(proc);
			while(wait && proc->state == PROCESS_ALIVE);
		}
		kfree(file, sizeof(file_t));
		return true;
	}
	return false;
}*/

static void command_eval(char *cmd, char *args){
	/*if(strcmp(cmd,"help")){
		println("ls: List the files in the current directory. Use -h for help.");
		println("cd: Change the current directory.");
		println("pwd: Print the working directory.");
		println("about: Shows some information about the system.");
		println("help: Shows this message.");
		println("cat: Prints a file's contents.");
		println("about: Prints some information.");
		//println("partinfo: Prints information about the current partition.");
		println("pagefault: Triggers a page fault, in case you wanted to.");
		println("tasks: Prints all running tasks.");
		println("bg: Run a program in the background.");
		println("kill: Kill a program.");
		println("dummy: Create a dummy process.");
		println("elfinfo: Print info about an ELF executable.");
		println("lspci: Lists PCI devices.");
		println("exit: Pretty self explanatory.");
	}else if(strcmp(cmd,"ls")){
		if(strcmp(args,"")){
			shellfs->listDir(&currentDir, shellfs);
		}else{
			/*uint32_t inodeID = ext2_findFile(args, current_inode, inode_buf, ext2);
			if(!inodeID) printf("That directory does not exist.\n"); else{
				ext2_inode *inode = kmalloc(sizeof(ext2_inode));
				ext2_readInode(inodeID, inode, ext2);
				if((inode->type & 0xF000) != EXT2_DIRECTORY) printf("%s is not a directory.\n",args); else ext2_listDirectory(inodeID, ext2);
			}*//*
		}
	}else if(strcmp(cmd,"cd")){
		strcpy(dirbuf, dirbuf2);
		strcat(dirbuf2,args);
		strcat(dirbuf2,"/");
		if(!shellfs->getFile(dirbuf2, &fileBuf, shellfs)) printf("That directory does not exist.\n"); else{
			if(!fileBuf.isDirectory) printf("%s is not a directory.\n",args); else{
				currentDir = fileBuf;
				strcpy(dirbuf2, dirbuf);
			}
		}
	}else if(strcmp(cmd,"pwd")){
		printf("%s\n",dirbuf);
	}else if(strcmp(cmd,"about")){
		println("DuckOS v0.0");
	}else if(strcmp(cmd,"cat")){
		file_t file = {};
		strcpy(dirbuf, dirbuf2);
		strcat(dirbuf2,args);
		strcat(dirbuf2,"/");
		if(shellfs->getFile(dirbuf2, &file, shellfs)){
			uint8_t *buf = (uint8_t *)kmalloc(file.sectors*512);
			shellfs->read(&file, buf, shellfs);
			for(int i = 0; i < file.size; i++)
				putch(buf[i]);
			kfree(buf, file.sectors*512);
		}else{
			printf("Cannot find %s.\n",args);
		}
	}else if(strcmp(cmd,"pagefault")){
		if(strcmp(args,"-r")){
			char i = ((char*)0xDEADC0DE)[0];
		}else if(strcmp(args,"-w")){
			((char*)0xDEADC0DE)[0]='F';
		}else{
			println("Usage: pagefault [-r,-w]");
			println("-r: Triggers a page fault by reading.");
			println("-w: Triggers a page fault by writing.");
		}
	}else if(strcmp(cmd,"exit")){
		exitShell = true;
	}else if(strcmp(cmd,"tasks")){
		printTasks();
	}else if(strcmp(cmd,"bg")){
		if(strcmp(args,"") || !findAndExecute(args, false)) printf("Cannot find \"%s\".\n", args);
	}else if(strcmp(cmd,"kill")){
		uint32_t pid = strToInt(args);
		process_t *proc = getProcess(pid);
		if(proc != NULL && proc->pid != 1){
			kill(proc);
			printf("Sent SIGTERM (%d) to %s (PID %d).\n", SIGTERM, proc->name, proc->pid);
		}else if(proc->pid == 1)
			printf("Cannot kill kernel!\n");
		else
			printf("No process with PID %d.\n", pid);
	}else if(strcmp(cmd, "dummy")){
		addProcess(createProcess("dummy", (uint32_t)dummy));
	}else if(strcmp(cmd, "elfinfo")){
		file_t file = {};
		strcpy(dirbuf, dirbuf2);
		strcat(dirbuf2,args);
		strcat(dirbuf2,"/");
		if(shellfs->getFile(dirbuf2, &file, shellfs)){
			uint8_t *headerBuf = (uint8_t *)kmalloc(512);
			elf32_header *header = (elf32_header*)headerBuf;
			shellfs->read(&file, headerBuf, shellfs);
			if(header->magic != ELF_MAGIC) {
				printf("Not an ELF file. %x\n", header->magic);
				return;
			}
			printf("Bits: %s\n", header->bits == ELF32 ? "32" : "64");
			printf("Endianness: %s\n", header->endianness == ELF_LITTLE_ENDIAN ? "little" : "big");
			printf("Instruction set: %s\n", header->instruction_set == ELF_X86 ? "x86" : "other");
			printf("Program Header Entry Size: %d\n", header->program_header_table_entry_size);
			printf("Num Program Header Entries: %d\n", header->program_header_table_entries);
			kfree(headerBuf, 512);
		}else{
			printf("Cannot find %s.\n",args);
		}
	}else if(strcmp(cmd, "lspci")){
		PCI::enumerate_devices([](PCI::Address address, PCI::ID id, void* dataPtr) {
			uint8_t clss = PCI::read_byte(address, PCI_CLASS);
			uint8_t subclss = PCI::read_byte(address, PCI_SUBCLASS);
			printf("%x:%x.%x Vendor: 0x%x Device: 0x%x\n  Class: 0x%x Subclass: 0x%x\n", address.bus, address.slot, address.function, id.vendor, id.device, clss, subclss);
		}, nullptr);
	}else if(strcmp(cmd, "findpata")){
		IDE::PATAChannel channel = IDE::find_pata_channel(ATA_PRIMARY);
		printf("%x:%x.%x Int %d\n", channel.address.bus, channel.address.slot, channel.address.function, PCI::read_byte(channel.address, PCI_INTERRUPT_LINE));
	}else{
		if(!findAndExecute(cmd, true)) printf("\"%s\" is not a recognized command, file, or program.\n", cmd);
	}*/
}
