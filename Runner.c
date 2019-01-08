#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/wait.h>
#include<sys/time.h>

struct later
{
	int long type;
};

int judge(int N, int id)
{
	struct later buf;
	struct timeval t_start, t_finish;
	printf("\nJudge is here;\n");
	buf.type = 5;
	for(int i = 0; i < N; i++)
		msgsnd(id, &buf, 0, 0);
	for(int i = 0; i < N; i++)
		msgrcv(id, &buf, 0, 6, 0);
	buf.type = 14;
	printf("\n !!! START !!! \n\n");
	gettimeofday(&t_start, NULL);
	msgsnd(id, &buf, 0, 0);
	msgrcv(id, &buf, 0, (N + 1) * 14, 0);
	gettimeofday(&t_finish, NULL);
	printf("\n !!! FINISH !!! \n\n");
	printf("Obsen time of the race is %ld microseconds.\nIt was fast!\n",
			(t_finish.tv_sec - t_start.tv_sec) * 1000000 + (t_finish.tv_usec - t_start.tv_usec));
	return 0;
}

int runer(int MyNumber, int id)
{	
	struct later buf;
	msgrcv(id, &buf, 0, 5, 0);
	printf("%d'th runner is here;\n", MyNumber + 1);
	buf.type = 6;
	msgsnd(id, &buf, 0, 0);
	msgrcv(id, &buf, 0, (MyNumber + 1) * 14, 0);
	printf("%d'th start;\n", MyNumber + 1);
	buf.type = (MyNumber + 2) * 14;
	printf("%d'th finish;\n", MyNumber + 1);
	msgsnd(id, &buf, 0, 0);
	return 0;
}

int main(int argc, char * argv[])
{
	int N;
	if(argc == 1 || !(N = atoi(argv[1])))
	{
		printf("Bad data :( \n");
		return -1;
	}
	int id = msgget(IPC_PRIVATE, 0777|IPC_CREAT);
	int f = fork();
	if(f == -1)
	{
		perror(strerror(errno));
		return -1;
	}
	if(f == 0)
	{
		judge(N, id);
		return 0;
	}
	for(int i = 0; i < N; i++)
	{
		f = fork();
		if(f == -1)
		{
			perror(strerror(errno));
			return -1;
		}
		if(f == 0)
		{
			runer(i, id);
			return 0;
		}
	}
	for(int i = 0; i < N; i++)
		wait(NULL);
	return 0;
}
