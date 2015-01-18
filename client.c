#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<string.h>//memset
#include<stdlib.h>//sizeof
#include<netinet/in.h>//INADDR_ANY
#include<dirent.h> // DIR
#include<sys/stat.h>//stat
#include<fcntl.h>//open close
#include<unistd.h>


#define PORT 8000
#define MAXSZ 1024


int search(char file[100]) // to check if user have required permissions on file
{

	DIR *fd=opendir(".");
	struct dirent *entry=readdir(fd);
	while(entry != NULL)
	{
		if(strlen(entry->d_name)==strlen(file) && strcmp(entry->d_name,file)==0 )
		{
			struct stat buff;
			stat(file,&buff);
			if(S_ISREG(buff.st_mode))
				return 1;
        		else
				return 0;
		}
		entry=readdir(fd);
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int sockfd;//to create socket
	char *pass=(char *)malloc(1024);
	struct sockaddr_in serverAddress;//client will connect on this
	int n,k;
	char msg1[MAXSZ],msg2[MAXSZ],usr[MAXSZ];

//create socket
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
	{
		perror("Error:");
		exit(0);
	}

//initialize the socket addresses
	bzero(&serverAddress,sizeof(serverAddress));
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_addr.s_addr=inet_addr(argv[1]);
	serverAddress.sin_port=htons(PORT);

//client  connect to server on port
	k=connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));

	if(k==-1)
	{
		perror("Error");
		exit(0);
	}
//user authentication
//user must be added on the server side
	printf("Username:");
	scanf("%s",usr);
	pass=getpass("Please enter the password: ");//to hide password
	send(sockfd,usr,strlen(usr)+1,0);
	send(sockfd,pass,strlen(pass)+1,0);
	bzero(msg1,sizeof(msg1));

	n=recv(sockfd,msg1,MAXSZ,0);
	
	if(strcmp(msg1,"true")!=0)
	{
		printf("Authentication failure!!!\n");
		close(sockfd);
		exit(0);
	}
	else
	{
		char success[]="Authentication Succesful!!!";
		write(STDOUT_FILENO,success,strlen(success));

		while(1)
		{
			char cmd[100],flg[100],arg[100];
			int count=0;
			bzero(msg1,sizeof(msg1));
			bzero(msg2,sizeof(msg1));

			printf("\nftp> ");
			fflush(stdout);
			n=read(STDIN_FILENO,msg1,MAXSZ);

			n=strlen(msg1)+1;
			
			if(strcmp(msg1,"exit\n")==0)
			{
				printf("Closing FTP!!!\n");
				close(sockfd); 
				exit(0);
			}
			else
			{
				char filesize[1024];
				long long size=0,size_file=0,i=0;
				int filehandle;
				count=sscanf(msg1,"%s %s %s",cmd,flg,arg);
     
				if(strcmp(cmd,"get")==0)
				{
					send(sockfd,msg1,n,0);
					recv(sockfd,msg2,MAXSZ,0);
	
					if(strcmp(msg2,"found")==0)//file is present on server side and you have required permissions
					{
						recv(sockfd, &size, sizeof(long long int), 0);
						char *f = malloc(size);
						recv(sockfd, f, size, 0);
	
						while(1)
						{
							filehandle = open(flg, O_CREAT | O_EXCL | O_WRONLY, 0666);
	
							if(filehandle == -1)
							{
								sprintf(strlen(flg)+flg,"%d", i);
							}
							else 
								break;
						}
	
						write(filehandle, f, size);
						close(filehandle);
	
					}
					else//either file is not present ot you don't have required permissions
					{
						printf("%s\n",msg2);
						memset(msg2,0,sizeof(msg2));

					}
				}
				else if(strcmp(cmd,"put")==0)
				{
					int res=search(flg);//to check if user has required permissions on file
					if(res==0)
					{
						printf("Permission Denied");
						exit(0);
					}
					filehandle = open(flg, O_RDONLY);
					if(filehandle == -1)
					{
						perror("Error:");
					}
					else
					{
						int status;
						send(sockfd,msg1,MAXSZ,0);
						struct stat obj;
						stat(flg, &obj);
						size = obj.st_size;
						send(sockfd, &size, sizeof(int), 0);
						sendfile(sockfd, filehandle, NULL, size);
						recv(sockfd, &status, sizeof(int), 0);
	   
						if(status)
							printf("File stored successfully\n");
						else
							printf("File failed to be stored to remote machine\n");

					}
				}
				else if(strcmp(cmd,"ls")==0)
				{
					send(sockfd,msg1,n,0);
					recv(sockfd, &size, sizeof(long long int), 0);
		
					char *f1=malloc(size);
					recv(sockfd, f1, size, 0);
					write(1,f1,size);

				}
				else
				{
					send(sockfd,msg1,n,0);	
					n=recv(sockfd,msg2,MAXSZ,0);
					msg2[n]='\0';
					
					if(n>0)
						printf("%s\n",msg2);
					bzero(msg2,sizeof(msg2));
				}
			} 
		}
	}
	close(sockfd);
	return 0;
}
