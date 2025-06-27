#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(){
    key_t key = ftok(".", 1);
    int shmid;
    shmid = shmget(key, 4096, IPC_CREAT|0666);
    char* mptr = shmat(shmid, NULL, 0);

    char buffer[64] = {0};
    int i = 0;
    while(1){
        fgets(buffer, sizeof(buffer), stdin);
        strncpy(mptr, buffer, strlen(buffer));
        sleep(1);
    }
    shmdt(mptr);
}

