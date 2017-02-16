#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFF 1024
//参数: pid dump file
int main(int argc, char const *argv[])
{
	int fd;
	int fd_dest;
	unsigned long startaddr = 0,endaddr = 0;
	unsigned long map_len;
	size_t pid = 0;
	char buffer[BUFF] = {0};
	int buff;
    char* tmp= NULL;
	char path[100] = "";
	char* delim = NULL;
	char* str =NULL;
	char* ptr = NULL;
	size_t result = 0;
	if(argc < 4) {
		printf("[+] Error: less args [+]\n");
		printf("[+] ./memdump pid target save [+]\n");
		goto Failed;
	}
	tmp = (char*)malloc(BUFF);
	pid = atoi(argv[1]);
	sprintf(path,"/proc/%d/maps",pid);
	printf("[+] path:%s [+]\n",path);
	fd = open(path,O_RDONLY|O_CREAT);

	if(fd < 0) {
		printf("[+] Error: Open file %s failed [+]\n",path);
		goto Failed;
	}

	//读/proc/%d/maps文件
	while (result != -1) {
		result = readline(fd,buffer,BUFF);  //读取一行
		//00008000-00030000 r-xp 00000000 00:01 1222       /init		
		if(strstr(buffer,argv[2]) != NULL) {// && strstr(buffer,"r-xp") !=NULL)
			printf("[+] %s [+]\n", buffer);
			strcpy(tmp,buffer);
    		str = strtok(tmp,"-");//第一次调用strtok
    		if(str != NULL) {//当返回值不为NULL时，继续循环
    			sscanf(str, "%x", &startaddr);
				str = strtok(NULL," ");//继续调用strtok，分解剩下的字符串
				if(str != NULL)
					sscanf(str, "%x", &endaddr);
    		}
    		close(fd);
    		break;
		}
    }
    printf("[+] startaddr:%lu [+]\n",startaddr);
    map_len = endaddr - startaddr;
    printf("[+] map_len:%lu [+]\n",map_len);
    memset(path,0,sizeof(path));
    sprintf(path,"/proc/%d/mem",pid);

	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
		fprintf(stderr, "[+] ptrace attach failed. [+]\n");
		perror("ptrace");
		goto Failed;
	}
	if (waitpid(pid, NULL, 0) == -1) {
		fprintf(stderr, "[+] waitpid failed. [+]\n");
		perror("waitpid");
		ptrace(PTRACE_DETACH, pid, NULL, NULL);
		goto DETACH;
	}

    fd = open(path,O_RDONLY);
    if(fd <= 0) {
    	printf("[+] Error: Open file %s failed [+]\n",path);
		goto DETACH;
    }

    lseek(fd,startaddr,SEEK_SET);
    result = 0;
    fd_dest = open(argv[3], O_CREAT | O_TRUNC | O_RDWR, 0666);
    if(fd_dest == -1) {
    	fprintf(stderr, "[+] fopen failed. [+]\n");
		perror("fopen");
		ptrace(PTRACE_DETACH, pid, NULL, NULL);
		goto DETACH;
    }

    for(;startaddr < endaddr;startaddr += sizeof(int)) {
        errno = 0;
        if(((buff = ptrace(PTRACE_PEEKDATA,pid,(void*)startaddr,NULL)) == -1) && errno) {
            perror("PTRACE_KEEPDATA");
            if(ptrace(PTRACE_DETACH,pid,NULL,NULL))
                perror("PTRACE_DETACH");
            goto DETACH;
        }
        if(write(fd_dest,&buff,sizeof(buff)) != 4) {
            perror("write");
            if(ptrace(PTRACE_DETACH,pid,NULL,NULL))
                perror("PTRACE_DETACH\n");
            goto DETACH;
        }
    }
DETACH:
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
Failed:
	if(fd > 0)
		close(fd);
	if(fd_dest > 0)
		close(fd_dest);
	if(tmp != NULL) {
		free(tmp);
		tmp = NULL;
	}
	return 0;
}
