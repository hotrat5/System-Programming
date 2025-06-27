/*创建一个线程生成10个随机数（不相同）， 
另外一个线程输出这10个随机数， 
通过互斥锁和条件量来实现*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t v_consumer = PTHREAD_COND_INITIALIZER; // 消费者条件变量
pthread_cond_t v_producer = PTHREAD_COND_INITIALIZER; // 生产者条件变量
static int num = 0;
static int data_ready = 0; 

void* generate_num(void* arg) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&m);
        // 等待数据被消费（避免覆盖）
        while (data_ready == 1) {
            pthread_cond_wait(&v_producer, &m);
        }
        num = random() % 100;
        printf("生成随机数:%d\n", num);
        data_ready = 1;
        pthread_cond_signal(&v_consumer); // 通知消费者
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

void* print_num(void* arg) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&m);
        // 循环检查
        while (data_ready != 1) {
            pthread_cond_wait(&v_consumer, &m);
        }
        printf("打印随机数:%d\n", num);
        data_ready = 0; // 重置状态
        pthread_cond_signal(&v_producer); // 通知生产者
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main() {
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, generate_num, NULL);
    pthread_create(&tid2, NULL, print_num, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}