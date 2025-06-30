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
//     exit(0);
// }

int main(void) {
    //signal(SIGINT, cleanup);
    
    s = sem_open("/mtsem1", O_CREAT | O_RDWR, 0666, 10);
    m = sem_open("/mtsem2", O_CREAT | O_RDWR, 0666, 0);
    
    // if(s == SEM_FAILED || m == SEM_FAILED) {
    //     perror("sem_open");
    //     exit(1);
    // }

    while(1) {
        if(sem_wait(m)) perror("sem_wait(m)");
        printf("消费者：消费产品\n");
        fflush(stdout);
        sleep(1);
        if(sem_post(s)) perror("sem_post(s)");
    }
}