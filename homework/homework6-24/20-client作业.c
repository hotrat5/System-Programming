#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// 管道文件初始化函数
int fifo_init(const char *fifopath)
{
    // 创建个管道（Afifo， Bfifo）
    int ret = mkfifo(fifopath, 0777);
    if (ret < 0 && errno != EEXIST) // 错误，但是不是文件存在导致的错误
    {
        perror("Afifo");
        return -1;
    }

    int fd = open(fifopath, O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    return fd;
}

int main(void)
{
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0)
    {
        int fd = fifo_init("/tmp/Afifo");
        char buffer[1024] = {0};
        while (1)
        {
            int ret = read(fd, buffer, 1024);
            if (ret <= 0)
                break; // 读失败
            printf("read:%s\n", buffer);
        }
    }
    if (pid > 0)
    {
        int fd = fifo_init("/tmp/Bfifo");
        char buffer[1024] = {0};
        while (1)
        {
            scanf("%[^\n]", buffer);
            write(fd, buffer, strlen(buffer));
            while (getchar() != '\n') // 读取缓冲区中的所有字符
                ;
        }
    }
}