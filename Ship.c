#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<stdarg.h>
#include<errno.h>

#define SEM_SHIP_IN 1
#define SEM_LADDER 2
#define SEM_SHIP_WAIT_IN 3
#define SEM_TREV_START 4
#define SEM_TREV_FINISH 5
#define SEM_SHIP_OUT 6
#define SEM_BEACH 7

#define SIN(SEMNAME, N) 				\
{							\
	if(increase_sem_n(semid, SEMNAME, N) == -1)	\
		return -1;				\
}

#define SDN(SEMNAME, N)					\
{							\
	if(increase_sem_n(semid, SEMNAME, -N) == -1)	\
		return -1;				\
}

int Npass;
int Nboat;
int K;
int Z;

int increase_sem_n(int semid,int sem_name, int n)
{
	struct sembuf buf = {sem_name, n, 0}; 
	if (semop(semid, &buf, 1) == -1) 
	{
		perror("SEM_ERROR\n");    
		return -1;
	}
	return 0;	
}

int ship(int semid)
{
	SIN(SEM_BEACH, Npass);
	printf("Ship come to beach\n");
	for(int i = 0; i < Z; i++)
	{
		printf("Ladder is lowered\n");
		SIN(SEM_SHIP_IN, Nboat);
		SIN(SEM_LADDER, K);
		SDN(SEM_SHIP_WAIT_IN, Nboat);
		SDN(SEM_LADDER, K);
		printf("Ladder is raised.Ship depart.\n");
		SIN(SEM_TREV_START, Nboat);
		SDN(SEM_TREV_FINISH, Nboat);
		printf("Ship came back.\n");
		SIN(SEM_SHIP_OUT, Nboat)
	}
	SIN(SEM_LADDER, K);
	printf("Ladder is lowered.That was last travelling.\n");
	SDN(SEM_BEACH, Npass);	
	return 0;
}

int pass(int semid, int Mynumber)
{
	printf("%d'th is hear.\n", Mynumber);
	while(1)
	{
		SDN(SEM_SHIP_IN, 1);
		SDN(SEM_BEACH, 1);
		SDN(SEM_LADDER, 1);
		printf("%d'th go on ship\n", Mynumber);
		SIN(SEM_LADDER, 1);
		SIN(SEM_SHIP_WAIT_IN, 1)
		SDN(SEM_TREV_START, 1);
		printf("%d'th are ready to back\n", Mynumber);
		SIN(SEM_TREV_FINISH, 1);
		SDN(SEM_LADDER, 1);
		printf("%d'th go out ship\n", Mynumber);
		SDN(SEM_SHIP_OUT, 1);
		SIN(SEM_LADDER, 1);
		SIN(SEM_BEACH, 1);
	}
	return 0;
}

int main(int argc, char * argv[])
{
	Npass = atoi(argv[1]);
	Nboat = atoi(argv[2]);
	K = atoi(argv[3]);
	Z = atoi(argv[4]);
	Nboat = (Npass < Nboat) ? Npass : Nboat;
	int semid = semget(IPC_PRIVATE, 8, IPC_CREAT|IPC_EXCL|0666);
	if (semid == -1) 
	{
		perror(strerror(errno));
		return -1;
	}
	pid_t names[Npass + 1];
	for(int i = 0; i < Npass + 1; i++)
	{
		int f = fork();
		if(f == -1)
		{
			perror(strerror(errno));
			return -1;
		}
		if(f == 0)
		{
			if(i == 0)
			{
				if(ship(semid) == -1)
					printf("SHIP_ERORR\n");
			}
			else
				if(pass(semid, i) == -1)
				{
					printf("PASS_ERROR\n");
					for(int j = 0; j < Npass + 1; j++)
						if(j != i)
							kill(names[j],SIGKILL);
					kill(getppid(), SIGKILL);
				}
			return 0;
		}
		else
			names[i] = f;
	}

	wait(NULL);
	for(int i = 1; i < Npass + 1; i++)
		kill(names[i],SIGKILL);
	semctl(semid, IPC_RMID, 0);
	return 0;	
}
