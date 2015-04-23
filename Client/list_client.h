/*
List files in current working directory on client side.
*/

/* Return result of character comparison(Ignoring LETTER CASE)*/
int mycompare(const void *a, const void *b)
{
	return strcasecmp(*(const char **)a,*(const char **)b);
}


/* `ls` comand in linux. List files in current working directory without details.*/
void ls_dir(char *dir_name)
{
	DIR *fd;

	char *arg[MAXSZ];

	int i = INITIALISE;
	int n;
	
	struct dirent *entry;
	fd = opendir(dir_name);/* Open directory */
	if(fd == NULL)
	{
		perror("Error");
		return;
	}

	while((entry = readdir(fd))!=NULL)/* Ignore hidden files */
	{
	    if(entry->d_name[0] != '.' && strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0)
		{
			*(arg + i) = entry->d_name;
			i++;
		}
	}
	
	n = i;
	
	qsort(arg,n,sizeof(const char *),mycompare);/* Sort file names in alphabetical order ignoring LETTER CASE*/

	i = 0;
	while(i < n)
	{
		printf("%s\n",*(arg + i));
		i++;
	}
	
	printf("\n");
	closedir(fd);
}

void ls_l_dir(char *dir_name)
{
	char time_buff[MAXSZ];
	char *arg[MAXSZ];
	
	int i = INITIALISE;
	int n;
	int val = INITIALISE;
	int temp;

	struct group *gp;/* structure contating group details */
	struct passwd *pw;/* structure containing passwd file */
	struct tm *info;
	struct stat buff;
	struct dirent *entry;
	DIR *fd;
	fd  = opendir(dir_name);/* Open directory */
	if(fd == NULL)
	{
		perror("Error");
		return;
	}
	
	while((entry = readdir(fd))!= NULL)/* Read directory */
	{
		
	/* Call kiya tha */
	   if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0 && entry->d_name[0]!= '.')
 		{
			*(arg + i) = entry->d_name;
			i++;
		}

	}

	n = i;
	qsort(arg,n,sizeof(const char *),mycompare);

	i = 0;
	while(i < n)
	{
		lstat(*(arg + i),&buff);
	   	gp = getgrgid(buff.st_gid);		
		pw = getpwuid(buff.st_uid);
		info = localtime(&(buff.st_mtime));
		strftime(time_buff,sizeof(time_buff),"%b %d %H:%M",info);
		
			if(buff.st_size % 1024 == 0)
			{
				temp = (int)buff.st_size / 1024;
				val += temp ;
			}
			else
			{
				temp = ((int)buff.st_size / 1024);
				val += (temp + (4 - (temp % 4)));
			}

			/* Selecting file type */		
			switch(buff.st_mode & S_IFMT)
			{
				case S_IFCHR:
					printf("c");
					break;
				case S_IFBLK:
					printf("b");
					break;
				case S_IFDIR:
                        		printf("d");
					break;
				case S_IFLNK:
					printf("l");
					break;
				case S_IFIFO:
					printf("p");
					break;
				case S_IFSOCK:
					printf("s");
					break;
				default:
					printf("-");
					break;
			}


			/* File permissions */
			if(buff.st_mode & S_IRUSR)
				printf("r");
			else
				printf("-");

			if(buff.st_mode & S_IWUSR)
				printf("w");
			else
				printf("-");
		
			if(buff.st_mode & S_IXUSR)
				printf("x");
			else
				printf("-");

			if(buff.st_mode & S_IRGRP)
				printf("r");
			else
				printf("-");

			if(buff.st_mode & S_IWGRP)
				printf("w");
			else
				printf("-");

			if(buff.st_mode & S_IXGRP)
				printf("x");
			else
				printf("-");

			if(buff.st_mode & S_IROTH)
				printf("r");
			else
				printf("-");

			if(buff.st_mode & S_IWOTH)
				printf("w");
			else
				printf("-");
			
			if(buff.st_mode & S_IXOTH)
				printf("x");
			else
				printf("-");


			if(((buff.st_mode & S_IFMT)^S_IFCHR) == 0 || ((buff.st_mode & S_IFMT)^S_IFBLK) ==0)
				printf(" %6d %8s %8s %5d, %5d %13s %s\n",(int)buff.st_nlink,pw->pw_name,gp->gr_name,major(buff.st_rdev),minor(buff.st_rdev),time_buff,*(arg + i));
			else
				printf(" %6d %8s %8s %12u %13s %s\n",(int)buff.st_nlink,pw->pw_name,gp->gr_name,(unsigned int)buff.st_size,time_buff,*(arg + i));
		i++;
	}

	printf("total %d\n\n",val);
	closedir(fd);

}

