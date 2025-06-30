#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// 任务结构体
typedef struct Task {
    void (*function)(void*);  // 任务函数指针
    void* arg;                // 任务参数
    struct Task* next;        // 下一个任务指针
} Task;

// 线程池结构体
typedef struct ThreadPool {
    pthread_mutex_t lock;     // 互斥锁
    pthread_cond_t cond;      // 条件变量
    pthread_t* threads;       // 线程数组
    Task* task_queue;         // 任务队列头
    Task* task_queue_tail;    // 任务队列尾
    int thread_count;         // 线程数量
    int task_count;           // 任务数量
    bool shutdown;            // 关闭标志
} ThreadPool;

// 初始化线程池
ThreadPool* thread_pool_create(int thread_count) {
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (!pool) {
        perror("Failed to allocate thread pool");
        return NULL;
    }

    // 初始化互斥锁和条件变量
    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        perror("Mutex init failed");
        free(pool);
        return NULL;
    }
    
    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        perror("Cond init failed");
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;
    }

    // 创建线程数组
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        perror("Failed to allocate threads");
        pthread_mutex_destroy(&pool->lock);
        pthread_cond_destroy(&pool->cond);
        free(pool);
        return NULL;
    }

    // 初始化队列
    pool->task_queue = NULL;
    pool->task_queue_tail = NULL;
    pool->thread_count = thread_count;
    pool->task_count = 0;
    pool->shutdown = false;

    // 创建工作线程
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            perror("Thread creation failed");
            // 销毁已创建的线程
            for (int j = 0; j < i; j++) {
                pthread_cancel(pool->threads[j]);
            }
            free(pool->threads);
            pthread_mutex_destroy(&pool->lock);
            pthread_cond_destroy(&pool->cond);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

// 工作线程函数
void* worker_thread(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;
    
    while (true) {
        pthread_mutex_lock(&pool->lock);
        
        // 等待任务或关闭信号
        while (pool->task_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }
        
        // 如果关闭且没有任务，则退出
        if (pool->shutdown && pool->task_count == 0) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }
        
        // 从队列取出任务
        Task* task = pool->task_queue;
        if (task) {
            pool->task_queue = task->next;
            if (!pool->task_queue) {
                pool->task_queue_tail = NULL;
            }
            pool->task_count--;
        }
        
        pthread_mutex_unlock(&pool->lock);
        
        // 执行任务
        if (task) {
            task->function(task->arg);
            free(task);
        }
    }
    
    return NULL;
}

// 添加任务到线程池
int thread_pool_add_task(ThreadPool* pool, void (*function)(void*), void* arg) {
    if (!pool || pool->shutdown) {
        return -1;
    }
    
    // 创建新任务
    Task* new_task = (Task*)malloc(sizeof(Task));
    if (!new_task) {
        perror("Failed to allocate task");
        return -1;
    }
    
    new_task->function = function;
    new_task->arg = arg;
    new_task->next = NULL;
    
    pthread_mutex_lock(&pool->lock);
    
    // 将任务加入队列
    if (pool->task_queue_tail) {
        pool->task_queue_tail->next = new_task;
    } else {
        pool->task_queue = new_task;
    }
    pool->task_queue_tail = new_task;
    pool->task_count++;
    
    // 通知等待的线程
    pthread_cond_signal(&pool->cond);
    
    pthread_mutex_unlock(&pool->lock);
    
    return 0;
}

// 销毁线程池
void thread_pool_destroy(ThreadPool* pool) {
    if (!pool) return;
    
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = true;
    pthread_mutex_unlock(&pool->lock);
    
    // 唤醒所有线程
    pthread_cond_broadcast(&pool->cond);
    
    // 等待所有线程退出
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // 释放剩余任务
    Task* task = pool->task_queue;
    while (task) {
        Task* next = task->next;
        free(task);
        task = next;
    }
    
    // 释放资源
    free(pool->threads);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
    free(pool);
}

// 示例任务函数
void sample_task(void* arg) {
    int task_id = *(int*)arg;
    printf("Task %d started by thread %lu\n", task_id, (unsigned long)pthread_self());
    // 模拟任务执行时间
    usleep(100000); // 100ms
    printf("Task %d completed\n", task_id);
}

int main() {
    // 创建线程池（4个工作线程）
    ThreadPool* pool = thread_pool_create(4);
    if (!pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        return 1;
    }
    
    // 添加20个任务
    int task_ids[20];
    for (int i = 0; i < 20; i++) {
        task_ids[i] = i;
        if (thread_pool_add_task(pool, sample_task, &task_ids[i]) != 0) {
            fprintf(stderr, "Failed to add task %d\n", i);
        }
    }
    
    // 等待所有任务完成
    printf("All tasks added. Waiting for completion...\n");
    sleep(3); // 给足够时间执行
    
    // 销毁线程池
    thread_pool_destroy(pool);
    printf("Thread pool destroyed\n");
    
    return 0;
}