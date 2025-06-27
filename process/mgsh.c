#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glob.h>

#define DELIMS " \t\n"
struct cmd_st{
    glob_t globres;
}

static void prompt(void){
    printf("mgsh$ ");
}

static void parse(char* line, struct cmd_st* res){
    char* tok;
    int i = 0;
    while(1){
        
    }
}

int main(){
    char* linebuf = NULL;
    size_t linebuf_size = 0;
    struct cmd_st cmd;
    pid_t pid;
    while(1){
        prompt();
        getline();
        parse(linebuf, &cmd);
        if(strcmp(cmd.globres.gl_pathv[0], "exit") == 0){
            exit(0);
        }
        if(0){

        }
        else{
            
        }
    }
    exit(0);
}
