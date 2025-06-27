#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

void sender_process() {
    int fd;
    char buffer[64];


    fd = open("/tmp/fifo1", O_WRONLY);
    if (fd == -1) {
        perror("打开发送管道失败");
        exit(1);
    }

    printf("服务器发送进程已启动\n");

    while (1) {
        // 从标准输入读取消息
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // 移除换行符

        // 发送消息到客户端
        write(fd, buffer, strlen(buffer));
    }

    close(fd);
}

void receiver_process() {
    int fd;
    char buffer[64];

    // 创建命名管道（如果不存在）
    if (mkfifo("/tmp/fifo2", 0666) == -1) {
        if (errno != EEXIST) {
            perror("创建接收管道失败");
            exit(1);
        }
    }

    // 打开命名管道以读取数据
    fd = open("/tmp/fifo2", O_RDONLY);
    if (fd == -1) {
        perror("打开接收管道失败");
        exit(1);
    }

    printf("服务器接收进程已启动\n");

    while (1) {
        // 读取客户端消息
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            printf("客户端: %s\n", buffer);
        }
    }

    close(fd);
    unlink("/tmp/fifo2");  // 删除命名管道
}

int main() {
    // 创建命名管道（如果不存在）
    if (mkfifo("/tmp/fifo1", 0666) == -1) {
        if (errno != EEXIST) {
            perror("创建发送管道失败");
            return 1;
        }
    }

    // 创建子进程
    pid_t sender_pid = fork();
    if (sender_pid < 0) {
        perror("创建发送子进程失败");
        return 1;
    }

    if (sender_pid == 0) {
        // 子进程：负责发送消息
        sender_process();
        return 0;
    } else {
        // 父进程继续执行，创建接收子进程
        pid_t receiver_pid = fork();
        if (receiver_pid < 0) {
            perror("创建接收子进程失败");
            return 1;
        }

        if (receiver_pid == 0) {
            // 子进程：负责接收消息
            receiver_process();
            return 0;
        }
    }

    // 父进程等待子进程结束
    wait(NULL);
    wait(NULL);

    // 清理资源
    unlink("/tmp/fifo1");

    return 0;
}  