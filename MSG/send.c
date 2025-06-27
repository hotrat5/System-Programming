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

	struct message msg;
	bzero(&msg, sizeof(msg));

	msg.mtype = 1;
	// 消息内容
	fgets(msg.mtext, 80, stdin);

    // 发送消息
	msgsnd(msgid, &msg, strlen(msg.mtext), 0);
}