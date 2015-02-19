#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<string.h>//memset strcmp
#include<stdlib.h>//sizeof exit
#include<netinet/in.h>//INADDR_ANY
#include<arpa/inet.h>
#include<ctype.h> //isspace
#include<dirent.h> // for parsing directory
#include<unistd.h> //for access R_OK
#include<errno.h>
#include<sys/stat.h> //stat
#include<fcntl.h> //O_RDONLY
#include<sys/sendfile.h>
#include<pwd.h> //user authentication
#include<shadow.h>
#include<crypt.h>

#define PORT 8000
#define MAXSZ 1024

char out[MAXSZ];

int sys_auth_user(const char *username, const char *password)
{
  struct passwd *pw;
  struct spwd *sp;
  char *encrypted, *correct;

  pw = getpwnam(username);
  endpwent();

  if (!pw) 
	return 1; //user doesn't really exist

  sp = getspnam (pw->pw_name);
  endspent();
  if(sp)
     correct = sp->sp_pwdp;
  else
     correct = pw->pw_passwd;

  encrypted = crypt(password, correct);
  return strcmp (encrypted, correct) ? 2 : 0;  // bad password=2, success=0
}


int search2(char fil[100])
{ 
	bzero(out,sizeof(out));
	
	if(access(fil,F_OK)==0)
    {
		if(access(fil,R_OK)==0)
		{
			struct stat buf;
			stat(fil,&buf);
			if(S_ISDIR(buf.st_mode))
			{
				return 1;
			}
			else
			{
				strcpy(out,"Not a directory");
				return -1;
			}
		}
		else
		{ 
			strcpy(out,strerror(errno));
			return 0;
		}
    }
	else
	{ 
		strcpy(out,strerror(errno));
		return -1;
	}
}



int search1(char fil[100])
{ 
	bzero(out,sizeof(out));

	if(access(fil,F_OK)==0)
    {
		if(access(fil,R_OK)==0)
		{
			struct stat buf;
			stat(fil,&buf);
	
			if(S_ISREG(buf.st_mode))
			{
				return 1;
			}
			else
			{
				strcpy(out,"Not a regular file");
				return -1;	
			}
		}
		else
		{ 
			strcpy(out,strerror(errno));
			return 0;
		}
    }
	else
	{ 
		strcpy(out,strerror(errno));
		return -1;
	}
}


int main()
{
	int sockfd;//to create socket
	int newsockfd;//to accept connection

	struct sockaddr_in serverAddress;//server receive on this address
	struct sockaddr_in clientAddress;//server sends to client on this address

	int n;
	char msg[MAXSZ];
	socklen_t clientAddressLength;
	int pid;
	int yes = 1;
	int i = 0;
	int count;
	int res;
	int auth;
	int filehandle;
	int total;
	int c = 0;
	int size;
	
	long int p;
	long long int num;
	long int size;
	
	char cmd[MAXSZ];
	char flg[MAXSZ];
	char user[MAXSZ];
	char pass[MAXSZ];
	char curr_dir[MAXSZ];
	char content[MAXSZ];
	char found[MAXSZ];
	char size_buff[MAXSZ];
						
	char fail[]="Authentication failure";
	char success[]="true";
	char def[]="Directory changed";
	
	char *f;

	struct stat obj;
	struct stat tem;
	
	//create socket
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("Error");
		exit(1);
	}
	
	//initialize the socket addresses
	bzero(&serverAddress,sizeof(serverAddress));
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddress.sin_port=htons(PORT);

	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));

	//bind the socket with the server address and port
	bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));

	//listen for connection from client
	listen(sockfd,5);

	while(1)
	{
		//parent process waiting to accept a new connection
		printf("\nWaiting for new client connection:\n");
		clientAddressLength = sizeof(clientAddress);
		newsockfd = accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);
		printf("Connected to client: %s\n",inet_ntoa(clientAddress.sin_addr));

		//child process is created for serving each new clients
		pid = fork();
		if(pid == 0)//child process rec and send
		{
			close(sockfd);

			n = recv(newsockfd,user,sizeof(user),0);
			n = recv(newsockfd,pass,sizeof(pass),0);

			auth = sys_auth_user(user,pass);

			if(auth == 0)
			{
				
				n = strlen(success)+1;
				send(newsockfd,success,n,0);

				//receive from client
				while(1)
				{
					n = recv(newsockfd,msg,MAXSZ,0);
					msg[n] = '\0';

					if(msg[strlen(msg)-1] == '\r')
						msg[strlen(msg)-1] = '\0';
						
					if(msg[strlen(msg)-1] == '\n')
						msg[strlen(msg)-1] = '\0';

					sscanf(msg,"%s %s",cmd,flg);

					if(strcmp(cmd,"pwd") == 0)
					{
						getcwd(curr_dir,sizeof(curr_dir));
						send(newsockfd,curr_dir,strlen(curr_dir),0);
					}
					else if(strcmp(cmd,"ls") == 0)
					{
						system("ls > /tmp/new.txt");
						filehandle = open("/tmp/new.txt",O_RDONLY);
						
						fstat(filehandle,&tem);
						num = tem.st_size;	
						send(newsockfd, &num, sizeof(long long int), 0);
						
						if(num)
							sendfile(newsockfd,filehandle, NULL,num);
						
						close(filehandle);
					}
					else if(strcmp(cmd,"cd") == 0)
					{
						res = search2(flg);
						if(res != 1)
						{
							send(newsockfd,out,strlen(out),0);
						}
						else
						{
							chdir(flg);
							send(newsockfd,def,strlen(def),0);
						}
					}
					else if(strcmp(cmd,"get") == 0)
					{
						bzero(content,sizeof(content));
						strcpy(found,"found");
						res = search1(flg);

						if(res == 1)
						{
							p = 0;
							send(newsockfd,found,strlen(found)+1,0); 
							
							filehandle = open(flg,O_RDONLY);
							fstat(filehandle,&obj);
							size = (long)obj.st_size;
							
							if(filehandle == -1)
								size = 0;

							sprintf(size_buff,"%ld",size);
	
							send(newsockfd, size_buff, strlen(size_buff), 0);
 	
							lseek(filehandle,0,0);
							while(size > 0)
							{
								n=read(filehandle,content,1024);
								total = 0;
								while(total < n)
								{
									p = send(newsockfd,content + total,n - total,0);
									total += p;
								}
								
								bzero(content,sizeof(content));
								size -= n;	
		
							}

						close(filehandle);
						}
						else
						{
							send(newsockfd,out,strlen(out),0); 
						}
	
					}
					else if(strcmp(cmd,"put") == 0)
					{
						c = 0;
						recv(newsockfd, &size, sizeof(int), 0);
						i = 1;
						
						while(1)
						{
							filehandle = open(flg, O_CREAT | O_EXCL | O_WRONLY, 0666);
			
							if(filehandle == -1)
							{
								sprintf(flg + strlen(flg), "%d", i);
							}
							else
								break;
						}
						
						f = malloc(size);
						recv(newsockfd, f, size, 0);
						c = write(filehandle, f, size);
						
						close(filehandle);
						
						send(newsockfd, &c, sizeof(int), 0);
					}
					else
					{
						msg[n]='\0';
						send(newsockfd,msg,n,0);
					}

				}//close interior while
				exit(0);
			}
			else
			{
				
				n = strlen(fail);
				send(newsockfd,fail,n,0);
				close(newsockfd);
			}
		}
		else
		{
			close(newsockfd);//sock is closed BY PARENT
		}
	}//close exterior while
return 0;
}
