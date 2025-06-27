#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(){
    key_t key = ftok(".", 1);
    int shmid = shmget(key, 4096, IPC_CREAT|0666);
    char* mptr = shmat(shmid, NULL, 0);
    while(1){
        printf("%s\n", mptr);

        if(strstr(mptr, "quit")) break;
    }

    shmdt(mptr);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}