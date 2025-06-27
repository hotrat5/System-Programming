#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(){
    signal(SIGINT,SIG_IGN);
    while(1){
        printf("信号\n");
        sleep(1);
    }
    sigset_t set;
    sigaddset(&set, SIGRTMIN + 4);
    setprocmask(SIG_SETMASK, &set, NULL);
    
    
    
    

}