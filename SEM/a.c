#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

sem_t *s, *m;

// void cleanup(int sig) {
//     sem_close(s);
//     sem_close(m);
//     sem_unlink("/mtsem1");
//     sem_unlink("/mtsem2");
//     exit(0);
// }

int main(void) {
    // signal(SIGINT, cleanup);
    
    // 清理可能存在的旧信号量
    sem_unlink("/mtsem1");
    sem_unlink("/mtsem2");
    
    s = sem_open("/mtsem1", O_CREAT | O_EXCL, 0666, 10);
    m = sem_open("/mtsem2", O_CREAT | O_EXCL, 0666, 0);
    
    // if(s == SEM_FAILED || m == SEM_FAILED) {
    //     perror("sem_open");
    //     exit(1);
    // }

    while(1) {
        if(sem_wait(s)) perror("sem_wait(s)");
        printf("生产者：生成产品\n");
        fflush(stdout);  // 确保及时输出
        if(sem_post(m)) perror("sem_post(m)");
        usleep(100000);  // 100ms延迟
    }
}