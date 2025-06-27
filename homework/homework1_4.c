//危害：子进程退出后，父进程未调用 wait()，子进程成为僵尸进程，直到父进程退出。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // 子进程
        printf("子进程(PID=%d)\n", getpid());
        exit(0);
    } else {
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