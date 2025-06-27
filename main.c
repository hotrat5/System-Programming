#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("开始\n");

    char *const envp[] = {"A=192.168.64.1", "B=9999", NULL};
    int ret = execle("", "main", NULL, envp); // 执行成功把ls程序加载到当前进程，并且执行，
    if (ret < 0)
    {
        perror("execl:");
    }


}