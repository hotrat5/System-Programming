#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    
    pid_t pid = atoi(argv[1]);
    
    kill(pid, SIGKILL); // 发送SIGUSR1信号
    printf("已发送信号到进程 %d\n", pid);
}