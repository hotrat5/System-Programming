#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

static int i;
void* handle(void* arg){
    while(1){
        printf("%d\n", i);
        sleep(1);
        exit(0);
    }
}

int main(){

    pthread_t tid;
    i = 1;
    if(pthread_create(&tid, NULL, handle, NULL) < 0){
        return -1;
    }

    while(1){
        i = i + 1;
        sleep(1);
    }
}