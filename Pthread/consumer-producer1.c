#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5  // 缓冲区大小
#define TOTAL_ITEMS 10 // 总共要生产的数字

// 循环缓冲区结构
typedef struct {
    int buffer[BUFFER_SIZE];  // 缓冲区数组
    int in;                   // 生产位置
    int out;                  // 消费位置
    int count;                // 当前元素数量
    pthread_mutex_t mutex;    // 互斥锁
    pthread_cond_t not_full;  // 缓冲区不满条件变量
    pthread_cond_t not_empty; // 缓冲区不空条件变量
} BoundedBuffer;

// 初始化缓冲区
void init_bounded_buffer(BoundedBuffer *bb) {
    bb->in = 0;
    bb->out = 0;
    bb->count = 0;
    pthread_mutex_init(&bb->mutex, NULL);
    pthread_cond_init(&bb->not_full, NULL);
    pthread_cond_init(&bb->not_empty, NULL);
}

// 销毁缓冲区资源
void destroy_bounded_buffer(BoundedBuffer *bb) {
    pthread_mutex_destroy(&bb->mutex);
    pthread_cond_destroy(&bb->not_full);
    pthread_cond_destroy(&bb->not_empty);
}

// 生产者函数：向缓冲区添加数据
void put(BoundedBuffer *bb, int item) {
    pthread_mutex_lock(&bb->mutex);
    
    // 等待缓冲区有空位
    while (bb->count == BUFFER_SIZE) {
        pthread_cond_wait(&bb->not_full, &bb->mutex);
    }
    
    // 将数据放入缓冲区
    bb->buffer[bb->in] = item;
    bb->in = (bb->in + 1) % BUFFER_SIZE;
    bb->count++;
    
    printf("生产者 %lu 生产: %d\n", (unsigned long)pthread_self(), item);
    
    // 通知消费者有新数据
    pthread_cond_signal(&bb->not_empty);
    pthread_mutex_unlock(&bb->mutex);
}

// 消费者函数：从缓冲区取出数据
int get(BoundedBuffer *bb) {
    pthread_mutex_lock(&bb->mutex);
    
    // 等待缓冲区有数据
    while (bb->count == 0) {
        pthread_cond_wait(&bb->not_empty, &bb->mutex);
    }
    
    // 从缓冲区取出数据
    int item = bb->buffer[bb->out];
    bb->out = (bb->out + 1) % BUFFER_SIZE;
    bb->count--;
    
    printf("消费者 %lu 消费: %d\n", (unsigned long)pthread_self(), item);
    
    // 通知生产者有空位
    pthread_cond_signal(&bb->not_full);
    pthread_mutex_unlock(&bb->mutex);
    
    return item;
}

// 生产者线程函数
void *producer(void *arg) {
    BoundedBuffer *bb = (BoundedBuffer *)arg;
    static int next_item = 1;
    int produced = 0;
    
    while (produced < TOTAL_ITEMS / 2) {  // 每个生产者生产5个数字
        int item = __sync_fetch_and_add(&next_item, 1);
        put(bb, item);
        produced++;
        
        // 随机延迟(0-100ms)
        usleep(rand() % 100000);
    }
    return NULL;
}

// 消费者线程函数
void *consumer(void *arg) {
    BoundedBuffer *bb = (BoundedBuffer *)arg;
    int consumed = 0;
    
    while (consumed < TOTAL_ITEMS / 2) {  // 每个消费者消费5个数字
        get(bb);
        consumed++;
        
        // 随机延迟(0-100ms)
        usleep(rand() % 100000);
    }
    return NULL;
}

int main() {
    srand(time(NULL));  // 初始化随机数种子
    
    BoundedBuffer bb;
    init_bounded_buffer(&bb);
    
    pthread_t producers[2], consumers[2];
    
    // 创建两个生产者线程
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, producer, &bb);
    }
    
    // 创建两个消费者线程
    for (int i = 0; i < 2; i++) {
        pthread_create(&consumers[i], NULL, consumer, &bb);
    }
    
    // 等待生产者线程结束
    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // 等待消费者线程结束
    for (int i = 0; i < 2; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    destroy_bounded_buffer(&bb);
    return 0;
}