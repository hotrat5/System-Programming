#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
int data;
int main(){
    data = 0;
    while(1){
    pid_t pid = fork();
    if(pid > 0){
            data = data+1;
            sleep(1);
            wait(NULL);
        }
    
    else{    
            sleep(1);
            printf("%d\n", data);
            exit(1);
        }

}

    return 0;
}