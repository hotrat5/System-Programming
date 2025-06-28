#include <signal.h>
#include <unistd.h>
#include <stdio.h>
void handler(int sig)
{

}

int safe_wait_with_timeout(int seconds){
    sigset_t mask, orig_mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, &orig_mask);
    alarm(seconds);
    int unslept = alarm(0);
    sigsuspend(&orig_mask);
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);
    
    return unslept;

}

int main(){
    signal(SIGALRM, handler);
    printf("Waiting with 5s timeout...\n");
    safe_wait_with_timeout(5);
    printf("Wait completed\n");
}