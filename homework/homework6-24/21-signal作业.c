#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handle(int sig)
{
    printf("第%d号信号触发\n", sig);
}

int main(void)
{
    printf("%d\n", getpid());
    for (int i = 1; i <= 64; i++)
    {
        signal(i, handle);
    }

    // 屏蔽信号
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, NULL);

    while (1)
    {
        int num = 0;
        printf("请输入要解除屏蔽信号编号:1-64(除32,33):");
        scanf("%d", &num);
        sigdelset(&set, num);
        sigprocmask(SIG_SETMASK, &set, NULL);
    }
}