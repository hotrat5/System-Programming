#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// 设计一个任务节点
struct Task
{
    struct Task *next;
    void (*fun)(void);
};

struct Task *create_task(void (*fun)(void))
{
    struct Task *t = malloc(sizeof(struct Task));
    t->next = NULL;
    t->fun = fun;
    return t;
}

// 添加任务
void add_task(struct Task *t, void (*fun)(void))
{
    struct Task *node = create_task(fun);
    node->next = t->next;
    t->next = node;
}
// 提取任务
struct Task *get_task(struct Task *t)
{
    if (t->next == NULL)
        return NULL; // 没有任务
    struct Task *p = t->next;
    t->next = p->next;
    return p;
}

// 条件量
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // 条件量
// 互斥锁
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 线程池属性：
int thread_number = 0; // 总线程数
int run_number = 0;    // 运行线程数
int sleep_number = 0;  // 睡眠线程数

// 线程任务函数
void *run(void *arg)
{
    thread_number++;
    struct Task *head = (struct Task *)arg;
    while (1)
    {

        // 上锁
        pthread_mutex_lock(&mutex);
        sleep_number++; // 睡眠线程数据++；
        pthread_cond_wait(&cond, &mutex);
        sleep_number--; // 睡眠线程数据--；
        run_number++;
        pthread_mutex_unlock(&mutex);

        while (1)
        {
            pthread_mutex_lock(&mutex);
            // 提取任务
            struct Task *t = get_task(head);
            pthread_mutex_unlock(&mutex);

            // 执行任务
            if (t != NULL)
            {
                t->fun();
            }
            else
            {
                // 判断是否还有任务
                break;
            }
        }
        run_number--;
        // 空闲线程数
    }
    thread_number--;
}

void mytask(void)
{
    printf("线程ID:%ld\n", pthread_self());
    sleep(5);
    printf("总线程数:%d, 睡眠线程数:%d, 运行线程数:%d\n", thread_number, sleep_number, run_number);
}

int main(void)
{
    // 创建链表头
    struct Task *head = create_task(NULL);

    // 创建线程
    pthread_t id = 0;
    for (int i = 0; i < 10; i++)
    {

        int ret = pthread_create(&id, NULL, run, (void *)head);
        if (ret < 0)
        {
            perror("pthread_create");
            return -1;
        }

        // 分离属性
        pthread_detach(id);
    }

    while (1)
    {
        getchar(); // 按回车模拟任务添加
        printf("总线程数:%d, 睡眠线程数:%d, 运行线程数:%d\n", thread_number, sleep_number, run_number);
        // 上锁
        pthread_mutex_lock(&mutex);
        // 提供任务
        add_task(head, mytask);
        // 唤醒线程
        pthread_cond_signal(&cond);
        // 解锁
        pthread_mutex_unlock(&mutex);
    }
}