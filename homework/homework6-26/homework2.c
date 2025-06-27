/*运用多线程，文件操作， 完成下面内容（提高题选做）
a.将一个大文件拆分为多个数据块，使用多线程并行写入另一个文件
b.链表管理任务块（每个块包含起始 / 结束偏移和数据缓冲区）
c.互斥锁保护文件指针和任务队列
提示：1.先把文件分块（每一个块作为一个任务用链表存储）
     2.每个线程从链表头部获取任务，从链表中提取一个任务（删除一个节点）主要互斥锁
     3.提取一个任务（就是数据块结构体，里面有每个块包含起始 / 结束偏移和数据缓冲区）
    根据这些信息把数据写入到新的文件中*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// 任务块结构体
typedef struct Chunk {
    long start_offset;      // 块起始偏移
    long end_offset;        // 块结束偏移
    char* buffer;           // 数据缓冲区
    size_t size;            // 实际数据大小
    struct Chunk* next;     // 下一个任务块指针
} Chunk;

// 全局变量
Chunk* task_head = NULL;            // 任务链表头指针
pthread_mutex_t task_mutex;         // 任务队列互斥锁
pthread_mutex_t file_mutex;         // 文件操作互斥锁
int in_fd, out_fd;                  // 输入/输出文件描述符
int active_threads = 0;             // 活跃线程计数器
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // 条件变量

// 工作线程函数
void* worker(void* arg) {
    while (1) {
        pthread_mutex_lock(&task_mutex);
        
        // 等待任务或所有任务完成
        while (task_head == NULL) {
            if (active_threads <= 1) {
                active_threads--;
                pthread_cond_broadcast(&cond);
                pthread_mutex_unlock(&task_mutex);
                return NULL;
            }
            pthread_cond_wait(&cond, &task_mutex);
        }
        
        // 获取任务块
        Chunk* task = task_head;
        task_head = task_head->next;
        pthread_mutex_unlock(&task_mutex);
        
        // 写入文件（需要文件操作锁）
        pthread_mutex_lock(&file_mutex);
        ssize_t written = pwrite(out_fd, task->buffer, task->size, task->start_offset);
        pthread_mutex_unlock(&file_mutex);
        
        if (written != task->size) {
            perror("写入文件失败");
        }
        
        // 释放任务资源
        free(task->buffer);
        free(task);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "用法: %s <输入文件> <输出文件> <线程数>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* in_filename = argv[1];
    const char* out_filename = argv[2];
    int num_threads = atoi(argv[3]);
    
    // 打开输入文件
    if ((in_fd = open(in_filename, O_RDONLY)) < 0) {
        perror("打开输入文件失败");
        exit(EXIT_FAILURE);
    }
    
    // 创建输出文件
    if ((out_fd = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
        perror("创建输出文件失败");
        close(in_fd);
        exit(EXIT_FAILURE);
    }

    // 获取文件大小
    struct stat st;
    if (fstat(in_fd, &st) < 0) {
        perror("获取文件大小失败");
        close(in_fd);
        close(out_fd);
        exit(EXIT_FAILURE);
    }
    long file_size = st.st_size;
    
    // 初始化互斥锁
    pthread_mutex_init(&task_mutex, NULL);
    pthread_mutex_init(&file_mutex, NULL);

    // 计算分块参数
    const long CHUNK_SIZE = 1024 * 1024; // 1MB 块大小
    long num_chunks = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    
    // 创建任务链表
    for (long i = 0; i < num_chunks; i++) {
        long start = i * CHUNK_SIZE;
        long end = (i == num_chunks - 1) ? file_size - 1 : (i + 1) * CHUNK_SIZE - 1;
        size_t size = end - start + 1;
        
        // 分配任务块内存
        Chunk* chunk = (Chunk*)malloc(sizeof(Chunk));
        if (!chunk) {
            perror("分配内存失败");
            break;
        }
        
        // 分配数据缓冲区
        chunk->buffer = (char*)malloc(size);
        if (!chunk->buffer) {
            perror("分配缓冲区失败");
            free(chunk);
            break;
        }
        
        // 读取文件数据
        if (pread(in_fd, chunk->buffer, size, start) != (ssize_t)size) {
            perror("读取文件失败");
            free(chunk->buffer);
            free(chunk);
            break;
        }
        
        // 设置任务块属性
        chunk->start_offset = start;
        chunk->end_offset = end;
        chunk->size = size;
        
        // 添加到链表头部（线程安全）
        pthread_mutex_lock(&task_mutex);
        chunk->next = task_head;
        task_head = chunk;
        pthread_mutex_unlock(&task_mutex);
    }
    
    // 创建工作线程
    pthread_t threads[num_threads];
    active_threads = num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            perror("创建线程失败");
            continue;
        }
    }
    
    // 主线程监控任务完成情况
    pthread_mutex_lock(&task_mutex);
    while (active_threads > 0) {
        pthread_cond_wait(&cond, &task_mutex);
    }
    pthread_mutex_unlock(&task_mutex);
    
    // 清理资源
    close(in_fd);
    close(out_fd);
    pthread_mutex_destroy(&task_mutex);
    pthread_mutex_destroy(&file_mutex);
    
    printf("文件处理完成! 总大小: %ld 字节, 使用线程: %d\n", file_size, num_threads);
    return 0;
}