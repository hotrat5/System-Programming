#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(void)
{

    char *fifopath[2] = {"/tmp/Afifo", "/tmp/Bfifo"};
    int fifofd[2] = {0};
    for (int i = 0; i < 2; i++)
    {
        // 创建两个管道（Afifo， Bfifo）
        int ret = mkfifo(fifopath[i], 0777);
        if (ret < 0 && errno != EEXIST) // 错误，但是不是文件存在导致的错误
        {
            perror("Afifo");
            return -1;
        }

        fifofd[i] = open(fifopath[i], O_RDWR);
        if (fifofd[i] < 0)
        {
            perror("open");
            return -1;
        }
    }

    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0)
    {
        char buffer[1024] = {0};
        while (1)
        {
            int ret = read(fifofd[1], buffer, 1024);
            if (ret <= 0)
                break; // 读失败
            printf("read:%s\n", buffer);
        }
    }
    if (pid > 0)
    {
        char buffer[1024] = {0};
        while (1)
        {
            scanf("%[^\n]", buffer);
            write(fifofd[0], buffer, strlen(buffer));
            while (getchar() != '\n') // 读取缓冲区中的所有字符
                ;
        }
    }
}