#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_ARGS 20      // 命令最大参数数量
#define MAX_CMD_LEN 100  // 单条命令最大长度
#define MAX_PIPES 10     // 最大管道数量

// 解析输入命令
int parse_command(char *cmd, char **args) {
    int i = 0;
    args[i] = strtok(cmd, " \t\n");
    
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        args[++i] = strtok(NULL, " \t\n");
    }
    return i; // 返回参数个数
}

// 执行单条命令
void exec_single_command(char **args, int input_fd, int output_fd) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // 子进程
        // 输入重定向
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        // 输出重定向
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
        
        // 执行命令
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}

// 处理管道命令
void handle_pipes(char ***commands, int cmd_count) {
    int pipes[2];
    int input_fd = STDIN_FILENO;
    
    for (int i = 0; i < cmd_count; i++) {
        // 创建管道（最后一个命令除外）
        if (i < cmd_count - 1) {
            if (pipe(pipes) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        
        // 执行当前命令
        exec_single_command(commands[i], input_fd, 
                           (i == cmd_count - 1) ? STDOUT_FILENO : pipes[1]);
        
        // 关闭不再需要的文件描述符
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (i < cmd_count - 1) {
            close(pipes[1]);
            input_fd = pipes[0]; // 下一个命令的输入来自当前管道
        }
    }
    
    // 等待所有子进程完成
    while (wait(NULL) > 0);
}

// 主Shell循环
int main() {
    char input_line[256];
    char *commands[MAX_PIPES][MAX_ARGS];
    int cmd_count;
    
    while (1) {
        printf("mysh> ");
        fflush(stdout);
        
        // 读取输入
        if (!fgets(input_line, sizeof(input_line), stdin)) {
            break; // 处理EOF (Ctrl+D)
        }
        
        // 处理空行
        if (strcmp(input_line, "\n") == 0) continue;
        
        // 分割管道命令
        char *cmd = strtok(input_line, "|\n");
        cmd_count = 0;
        
        while (cmd != NULL && cmd_count < MAX_PIPES) {
            parse_command(cmd, commands[cmd_count]);
            cmd_count++;
            cmd = strtok(NULL, "|\n");
        }
        
        // 处理内置命令
        if (strcmp(commands[0][0], "exit") == 0) {
            exit(EXIT_SUCCESS);
        } else if (strcmp(commands[0][0], "cd") == 0) {
            if (chdir(commands[0][1]) != 0) {
                perror("cd");
            }
            continue;
        }
        
        // 处理管道命令
        if (cmd_count > 1) {
            handle_pipes(commands, cmd_count);
        } 
        // 处理单命令
        else {
            exec_single_command(commands[0], STDIN_FILENO, STDOUT_FILENO);
            wait(NULL); // 等待子进程
        }
    }
    return 0;
}