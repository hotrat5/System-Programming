#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

void *handle(void *);

// 仓库容量
int num = 10;
// 信号量
sem_t s;
sem_t m;
int main(void)
{

    // 初始化信号量
    sem_init(&s, 0, num);
    sem_init(&m, 0, 0);

    // 创建线程
    pthread_t id = 0;
    int ret = pthread_create(&id, NULL, handle, NULL);
    // 分离线程属性
    pthread_detach(id);

    // 生产者
    while (1)
    {
        sem_wait(&s);
        printf("生产者：生成产品\n");
        sem_post(&m);
    }
}
void *handle(void *arg)
{

    // 消费者
    while (1)
    {
        sem_wait(&m);
        printf("消费者：消费产品\n");
        sleep(1);
        sem_post(&s);
    }
}