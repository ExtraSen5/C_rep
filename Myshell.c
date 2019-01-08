#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<string.h>

char ** get_args(int * length)
{
	char ** args = calloc(100,sizeof(char *));
	char * bigword = calloc(1000, 1);
	int _low, EOFind;
	args[0] = bigword;
	*length = 1;
	for(_low = 0; ((EOFind = getchar()) != '\n') && (EOFind != EOF); _low++)
	{
		bigword[_low] = EOFind;
		if(bigword[_low] == ' ' && bigword[_low-1] == ' ')
			_low--;
		if(bigword[_low] == ' ' && bigword[_low-1] == '|')
			_low--;
		if(bigword[_low] == '|' && bigword[_low-1] == '|')
		{
			printf("You have | too times\n");
			return 0;
		}
	}
	for(int i = 0; i < _low+1; i++)
	{
		if (bigword[i] == '|')
		{
			bigword[i++] = '\0';
			args[*length] = bigword+i;
			*length = *length+1;
		}
		if (bigword[i] == '\n')
			bigword[i] = '\0';
	}
	if(strcmp(args[0], "exit") == 0 && *length == 1)
		exit(-1);
	if(EOFind == EOF)
	{
		putchar('\n');
		exit(-1);
	}
	return args;
}

void free_args(char ** args)
{
	free(args[0]);
	free(args);
}

int main()
{
	while(1)
	{	
		printf("@@@@####--$> ");
		int length;
		char ** args = get_args(&length);
		if(args == NULL)
			continue;
		int fld[2], oldfld = -1;
		for(int i = 0; i < length; i++)
		{
			pipe(fld);
			int fk = fork();
			if(fk == -1)
			{
				perror(strerror(errno));
				return 0;
			}
			if(fk == 0)
			{
				close(fld[0]);
				if (i != 0)
				{
					close(0);
					dup(oldfld);
					close(oldfld);	
				}		
				if (i != length-1)
				{
					close(1);
					dup(fld[1]);
					close(fld[1]);
				}
				int l = 1;
				char ** new_args = calloc(100,sizeof(char *));
				new_args[0] = args[i];
				for(int j = 0; args[i][j] != '\0'; j++)
					if (args[i][j] == ' ')
					{
						new_args[l++] = args[i]+j+1;
						args[i][j++] = '\0';
					}
				execvp(new_args[0], new_args);
				perror(strerror(errno));
				return 0;
			}
			else
			{
				if(oldfld != -1)
					close(oldfld);
				oldfld = fld[0];
				close(fld[1]);
			}
		}
		for(int i = 0; i < length; i++)
			wait(0);
		free_args(args);
	}
	return 0;	
}

