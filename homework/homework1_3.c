#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork(); 
    if (pid == 0) {
        execlp("ping", "ping", "192.168.64.1", NULL);
        exit(1);
    } 
    
    else {
        while (1) {
            if (waitpid(pid, NULL, WNOHANG) == pid) {
                printf("子进程已结束\n");
                break;
            } 
            sleep(1); 
        }
    }

    return 0;
}