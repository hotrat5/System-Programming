#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

struct message
{
	long mtype;
	char mtext[80];
};

int main(void)
{
	int msgid;
	msgid = msgget(ftok(".", 1), IPC_CREAT | 0666);

	struct message msgbuf;
	bzero(&msgbuf, sizeof(msgbuf));

	printf("等待消息...\n");
    int m = msgrcv(msgid, &msgbuf, sizeof(msgbuf)-sizeof(long), 1, 0);

	if(m < 0)
		perror("msgrcv()");
    else
	    printf("%s\n", msgbuf.mtext);

	msgctl(msgid, IPC_RMID, NULL);
	return 0;
}