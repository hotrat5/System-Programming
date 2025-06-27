#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


static int buffer[32];
static char w_pos = 0;
static char r_pos = 0;
static int isempty = 1;
static int isfull = 0;

void* producer1(void* arg){
    while(1){
        int num = rand()%10;
        buffer[w_pos++] = num;
        while(w_pos >= 32){
            
        }
    }  
}

void* producer2(void* arg){
    while(1){
        int num = rand()%10;
        buffer[w_pos++] = num;
        while(w_pos >= 32){
            
        }
    }
}

void* consumer1(void* arg){
    while(1){
        int num = buffer[r_pos++];
        printf("生产者1：%d\n", num);
        while(r_pos>=32){

        }
    }
}

void* consumer2(void* arg){
    while(1){
        int num = buffer[r_pos++];
        printf("生产者1：%d\n", num);
        while(r_pos>=32){
            
        }
    }
}

int main(){
    pthread_t tid[4];

    
    pthread_create(&tid[0], NULL, producer1, NULL);
    pthread_create(&tid[1], NULL, producer2, NULL);
    pthread_create(&tid[2], NULL, consumer1, NULL);
    pthread_create(&tid[3], NULL, consumer2, NULL);
    
    

    return 0;
}