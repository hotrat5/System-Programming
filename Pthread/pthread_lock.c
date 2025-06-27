// concurrency.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t n = PTHREAD_MUTEX_INITIALIZER;

int global = 100;

void *isPrime(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&m);`    

        // 一段朴素的代码
        if(global%2 == 0)
            printf("%d是偶数\n", global);
            
        pthread_mutex_unlock(&n);
    }
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, isPrime, NULL);

    // 一条人畜无害的赋值语句
    while(1)
    {
        pthread_mutex_lock(&n);
        global = rand() % 5000;
        pthread_mutex_unlock(&m);
    }
}
