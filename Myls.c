#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/errno.h>
#include<string.h>
#include<errno.h>
#include<dirent.h>
#include<getopt.h>
#include<time.h>
#include<grp.h>
#include<pwd.h>

#define l_key ikey[0]
#define n_key ikey[1]
#define i_key ikey[2]
#define a_key ikey[3]
#define R_key ikey[4]
#define d_key ikey[5]
int argind;
int ikey[6] = {0,0,0,0,0,0};
// 0 - l, 1 - n, 2 - i, 3 - a, 4 - R, 5 - d;
char *st[8] = {"---","--x","-w-","-wx","r--","r-x","rw-","rwx"};

int myls(char * name)
{
	DIR * dir;
	char * dirname;
	struct dirent * dirbuf;
	struct stat mystat;
	struct group * mygroup;
	struct passwd * myuser;
	int isnotdir = 0;
	int newname = 0;

	if((dir = opendir(name)) == NULL)
	{
		for(int i = strlen(name); i > 0; i--)
		{
			if(name[i] == '/')
			{
				name[i] = '\0';
				dir = opendir(name);
				name[i] = '/';
				newname = i + 1;
				break;
			}
			if(i == 1)
			{
				dir = opendir(".");
				break;
			}
		}
		isnotdir = 1;
	}
	else
	{
		if((argind > 1 || R_key) && !d_key)
			printf("%s:\n", name);
	}
	while((dirbuf = readdir(dir)) != NULL)
	{
		dirname = calloc(1024, 1);
		strcat(dirname,name);
		if(!isnotdir)
		{
			if(dirname[strlen(dirname)-1] != '/')
				strcat(dirname,"/");
			if(!d_key)
				strcat(dirname,dirbuf -> d_name);
		}
		if(lstat(dirname, &mystat) == -1)
		{
			perror(strerror(errno));
			exit(-1);
		}
		free(dirname);
		if(isnotdir && strcmp(name + newname, dirbuf -> d_name) != 0)
			continue;

		if(dirbuf -> d_name[0] == '.' && !a_key && !d_key)
			continue;
		if(i_key)
			printf("%ld ", mystat.st_ino);
		if(l_key || n_key)
		{
			mygroup = getgrgid(mystat.st_gid);
			myuser = getpwuid(mystat.st_uid);

			char timebuf[256];
			ctime_r(& mystat.st_mtime, timebuf);
			timebuf[strlen(timebuf)-9] = '\0';
			
			putchar((dirbuf -> d_type == DT_DIR) ? 'd' : (S_ISLNK(mystat.st_mode)) ? 'l' : '-');
			printf("%s", st[(mystat.st_mode >> 6) & 7]);
			printf("%s", st[(mystat.st_mode >> 3) & 7]);
			printf("%s", st[mystat.st_mode & 7]);

			printf(" %ld",mystat.st_nlink);

			if(n_key)
			{
				if(myuser == 0)
					printf(" %d", mystat.st_uid);
				else
					printf(" %s",myuser -> pw_name);
				if(mygroup == 0)
					printf(" %d", mystat.st_gid);	
				else
					printf(" %s",mygroup -> gr_name);
			}
			else
			{
				printf(" %d", mystat.st_uid);
				printf(" %d", mystat.st_gid);	
			}

			printf(" %ld",mystat.st_size);
			printf(" %s",&timebuf[4]);
		}
		printf(" %s  ", (d_key || newname != 0) ? name : dirbuf -> d_name);

		if(l_key || n_key)
		{
			char link[256];
			if(S_ISLNK(mystat.st_mode))
			{
				readlink(dirbuf -> d_name, link, 256);
				printf(" -> %s\n", link);
			}
			else
				putchar('\n');
		}
		if(d_key)
			return 0;
	}
	if(!d_key)
		putchar('\n');
	closedir(dir);
	if(!R_key || isnotdir)
		return 0;
	dir = opendir(name);
	while((dirbuf = readdir(dir)) != NULL)
		if(strcmp(dirbuf -> d_name,".") != 0 && strcmp(dirbuf -> d_name,"..") != 0 && dirbuf -> d_type == DT_DIR)
		{
			printf("\n");
			char * newname = calloc(512, 1);
			strcat(newname,name);
			if(newname[strlen(newname)-1] != '/')
				strcat(newname,"/");
			strcat(newname,dirbuf -> d_name);
			if(dirbuf -> d_name[0] == '.' && !a_key)
				continue;
			putchar('\n');
			myls(newname);
			free(newname);
		}
	closedir(dir);
	return 0;
}

int main(int argc, char ** argv)
{
	//myls -l -n -i -a -R -d
	//getopt( ... ); getopt-long( ... ); for key
	static struct option long_option[]={// use to identify key
		{"long", no_argument, 0, 'l'},
		{"all", no_argument, 0, 'a'},
		{"numeric-uid-gid", no_argument, 0, 'n'},
		{"recursive", no_argument, 0, 'R'},
		{"inode", no_argument, 0, 'i'},
		{"directory", no_argument, 0, 'd'},
		{0,		0 , 0,  0 }
	};
	char key[] = "lniaRd";
	int opt;
	int longind;
	while((opt = getopt_long(argc, argv, key, long_option, &longind))!=EOF)
		switch(opt)
		{
			case 'l':
				{
					l_key = 1;
					break;
				}
			case 'n':
				{
					n_key = 1;
					break;
				}
			case 'i':
				{
					i_key = 1;
					break;
				}
			case 'a':
				{
					a_key = 1;
					break;
				}
			case 'R':
				{
					R_key = 1;
					break;
				}
			case 'd':
				{
					d_key = 1;
					break;
				}
		}

 	argind = argc - optind;
	if(argind == 0)
	{
		myls(".");
		return 0;
	}
	for(int i = optind; i < argc; i++)
		myls(argv[i]);
	putchar('\n');
	return 0;
}
