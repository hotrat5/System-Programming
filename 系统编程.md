## 线程

#### 1.概念

* 线程是一个正在运行的函数，多核处理器下操作系统可以将多个线程调度到不同的CPU中实现并行加速

* 线程是一种轻量级进程，依附于进程存在，进程中至少有一个线程。


* 系统分配资源的最小单位:进程；系统调度的最小单位：线程。


* 进程就像一个容器，容器承载多个线程。进程为线程提供资源环境，线程是进程的运行时。
* 线程之间共享地址空间和进程资源，且其本身拥有独立的PCB，

注：

1.posix线程是一套标准，而不是具体实现。比如线程标识：`pthread_t`，这只是一个标识，具体是结构体还是整形数，由开发者自己实现。

编译时需要在makefile加上选项(gcc进行默认编译时不用)

```
CFLAGES+=-pthread
LDFLAGS+=-pthread
```

2.线程的ID用`pthread_t`类型表示

```
pthread_equal();//比较线程标识
pthread_self(); //返回当前线程标识
```

3.现代主流操作系统中，线程和进程实现已经明确分离，但在Linux中，`clone()`系统调用是更底层的实现，`fork()`和线程创建都基于`clone()`,但传递不同的标志参数。



#### 2.创建，终止，栈的清理，取消选项

所有以`pthread_xxx`开头的线程函数，成功一律返回0，失败一律返回错误码



##### 1）创建线程

* 创建函数

```c
#include <pthread.h>
int pthread_create(pthread_t *thread, /*线程标识*/
					const pthread_attr_t *attr,   /*线程属性*/
					void *(*start_routine)(void *) /*线程函数*/
					void *arg /* 线程函数的参数*/) 
```

* 线程的调度取决于调度器策略，主线程推出后其他线程也会一并退出。


##### 2)线程的终止

* 线程从启动例程返回，返回值就是线程的退出码

* 线程可以从被同一进程中的其他线程取消

* 线程调用`pthread_exit();`

  * 如果任意线程调用了`exit`，那整个进程将会终止。

  * ```c
    #include <pthread.h>
    void pthread_exit(void *rval_ptr);/*rval_ptr:指向任意类型数据作为返回值*/
    ```

* `pthread_join()`和`pthread_detach()`

  * ```c
    #include <pthread.h>
    int pthread_join(pthread_t thread, void **retval);/*阻塞某个线程直到本线程结束，返回值存储在rval_ptr*/
    int pthread_detach(pthread_t thread);/*分离指定线程，此线程推出后自动释放资源而不会造成其他线程的阻塞*/
    ```

##### 4）栈的清理

线程退出时可以指定析构函数，并且可以以栈的方式指定多个析构函数及其执行顺序

```c
#include <pthread.h>

void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
```

`push`:压入一个析构函数`routine`,参数时`arg`

`pop`:`execute`为1时使对应顺序的函数生效，为0时使之无效

两个函数实际是由宏来实现的，并且`push`展开后包含`{`，`pop`展开后包含`}`，故`push`和`pop`之间可以包含任意代码，但要求必须成对出现，否则会报错花括号未闭合

##### 4）取消选项

线程之间可以借助函数来取消某个线程

```c
#include <pthread.h>

int pthread_cancel(pthread_t thread);/*发送取消信号给指定线程*/ 
int pthread_setcancelstate(int state, int *oldstate);/*设置此线程是否允许取消*/
int pthread_setcanceltype(int type, int *oldtype);/*设置取消的方式，如异步，推迟等*/
void pthread_testcancel(void);/*在本位置设置一个取消点*/
```



```c
#include <stdio.h>
#include <pthread.h>

static void* func(void* p){
    puts("Thread is working!");
    pthread_exit(NULL);
    return NULL;
}

int main(){
	pthread_t tid;
    int err;
    puts("Begin!");
    err = pthread_create(&tid, NULL, func, NULL);
    if(err){
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }
    puts("End!");
    exit(0);
	
}
```





## 同步互斥

### 1）互斥锁与读写锁

* 互斥是控制两个进度使之互相排斥，不同时运行
* 同步是控制两个进度使之有先有后，次序可控

#### 互斥锁

* ##### 创建, 初始化与销毁

  ```c
  #include <pthread.h>
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER/*静态初始化*/
  int pthread_mutex_init(pthread_mutex_t *restrict mutex, 
                         const pthread_mutexattr_t *restrict attr);/*动态初始化*/
  int pthread_mutex_destroy(pthread_mutex_t *mutex);  /*销毁*/  
  ```

  * 静态初始化互斥锁不涉及动态内存，无需显式释放互斥锁资源（无需手动销毁），互斥锁将伴随程序一直存在，直到程序退出
  * 动态初始化给互斥锁分配动态内存并赋予初始值，互斥锁需要在用完之后显式的进行释放资源（销毁）

* ##### 加锁解锁

```c
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);/*阻塞式锁定*/
int pthread_mutex_trylock(pthread_mutex_t *mutex);/*非阻塞加锁*/
int pthread_mutex_unlock(pthread_mutex_t *mutex);

```

加锁解锁时机的判断：

* 操作变量时是否时共享的，是否会造成读写冲突
* 在临界区内找到所有可能跳出临界区的位置并在其中对锁进行处理

##### 基本阻塞流程

1. 尝试获取锁
   * 线程调用`pthread_mutex_lock()`尝试获取互斥锁
   * 如果锁当前可用（未被其他线程持有），线程立即获取锁并继续执行
   * 如果锁已被其他线程持有，线程进入阻塞状态（）
2. 阻塞状态
   * 线程被移出CPU调度队列
   * 线程状态从“运行”变为“阻塞”
   * 操作系统将线程添加到该互斥锁的等待队列
   * 阻塞的线程处于睡眠状态，不会占用CPU时间片，直到被唤醒
3. 唤醒过程
   * 当持有锁的线程调用`pthread_mutex_unlock()`释放锁时
   * 操作系统从等待队列中选择一个线程（选择策略因实现而异）
   * 被选中的线程从阻塞变为就绪
   * 当CPU调度器选择该线程时，它重新尝试获取锁

#### 读写锁

读操作可以多任务并发执行，只有写操作才能进行恰当的互斥

* ##### 初始化销毁解锁

  ```c
  #include <pthread.h>
  
  // 静态初始化：
  pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
  
  // 动态初始化与销毁：
  int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock,
                          const pthread_rwlockattr_t *restrict attr);
  
  int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
  int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
  ```

  * 多条线程可以对同一个读写锁加多重读锁
  * 多条线程只能有一个拥有写锁
  * 读锁与写锁也是互斥的

  ### 条件变量

  条件变量是一种线程同步机制，它给多个线程提供一个汇合的场所，允许线程以无竞争的方式等待某个特定条件的发生，并在条件可能满足时被唤醒，条件变量本身需要由互斥锁保护

  ##### 初始化，销毁

  ```c
  #include <pthread.h>
  
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
  int pthread_cond_destroy(pthread_cond_t *cond);
  ```

  ##### 阻塞

  ```c
  #include <pthread.h>
  
  //阻塞当前线程并将其放入等待队列中，然后等待条件变量cond
  int pthread_cond_wait(pthread_cond_t *restrict cond, 
                        pthread_mutex_t *restrict mutex);
  
  /*阻塞当前线程并将其放入等待队列中，然后等待条件变量cond，
  并设置一个实现abstime,超时后不再等待并返回ETIMEOUT*/
  int pthread_cond_timedwait(pthread_cond_t *restrict cond, 
                             pthread_mutex_t *restrict mutex, 
                             const struct timespec *restrict abstime);
  ```

  * 这两个函数在阻塞进程前会先解锁让出线程，

  * `*restrict`关键字表示在该指针的生命周期内，其指向的对象不会被别的指针所引用。有利于编译优化

    **`pthread_cond_wait()`的原子操作**（“释放锁+进入等待”是原子操作）

    1. 释放互斥锁：**立即解锁关联的互斥锁**
    2. 进入等待状态：线程被添加到条件变量的等待队列
    3. 阻塞线程：线程进入阻塞状态，不消耗CPU资源
    4. 等待信号：线程在条件变量上休眠，直到被唤醒

    

  ##### 唤醒

  ```c
  #include <pthread.h>
  
  /*唤醒所有正在等待条件变量cond的线程*/
  int pthread_cond_broadcast(pthread_cond_t *cond);
  
  /*唤醒正在等待条件变量cond所有线程中的一个
  int pthread_cond_signal(pthread_cond_t *cond);
  ```

  当其他线程调用`pthread_cond_signal(&cond)`时：

  1. 检查等待队列：如果有线程在等待
  2. 唤醒一个线程：选择队列中的一个线程唤醒（具体策略取决于实现）
  3. 标记为可运行：操作系统将线程状态改为就绪
  4. 线程尝试重新获取锁：当被唤醒的线程获得CPU时间后，它首先尝试重新获取关联的互斥锁



