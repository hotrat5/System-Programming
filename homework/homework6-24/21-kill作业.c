#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    pid_t pid = atoi(argv[1]);
    for (int i = 1; i <= 64; i++)
    {
        if (i != 9)
            kill(pid, i);
    }
}