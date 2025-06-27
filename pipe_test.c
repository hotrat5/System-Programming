#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int main(){
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if(pid == 0){
        close(fd[0]);
        char* buff = "子进程";
        write(fd[1], buff, strlen(buff) + 1);
        exit(0);
    }
    if(pid > 0){
        close(fd[1]);
        char buff[32];
        read(fd[0], buff, sizeof(buff));
        printf("%s\n", buff);
        
    }
    else{
        perror("fork");
        return -1;
    }
    return 0;
}