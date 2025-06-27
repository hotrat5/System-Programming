#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
    
    printf("PID: %d\n", getpid());
    pause(); // 保持运行，等待信号
}