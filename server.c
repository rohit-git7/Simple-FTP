#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<string.h>//memset strcmp
#include<stdlib.h>//sizeof exit
#include<netinet/in.h>//INADDR_ANY
#include<ctype.h> //isspace
#include<dirent.h> // for parsing directory
#include<unistd.h> //for access R_OK
#include<errno.h>
#include<sys/stat.h> //stat
#include<fcntl.h> //O_RDONLY

#include<pwd.h> //user authentication
#include<shadow.h>

char out[100];
#define PORT 8000
#define MAXSZ 1024


int sys_auth_user(const char *username, const char *password)
{
  	struct passwd *pw;
  	struct spwd *sp;
	char *encrypted, *correct;

  	pw=getpwnam(username);
  	endpwent();

  	if(!pw) 
  	  return 1; //user doesn't really exist

	 sp=getspnam (pw->pw_name);
  	endspent();
  	
  	if(sp)
     	   correct = sp->sp_pwdp;
  	else
     	   correct = pw->pw_passwd;

  	encrypted = crypt(password, correct);
  	return strcmp (encrypted, correct) ? 2 : 0;  // bad pw=2, success=0
}


int search2(char fil[100])//To check entered name is a directory
{ 
	memset(out,0,sizeof(out));
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



int search1(char fil[100])//to check whether file is regular or not
{ 
	memset(out,0,sizeof(out));
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
	int clientAddressLength;
	int pid,status;

//create socket
	sockfd=socket(AF_INET,SOCK_STREAM,0);
//initialize the socket addresses
	memset(&serverAddress,0,sizeof(serverAddress));
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddress.sin_port=htons(PORT);

//bind the socket with the server address and port
	bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));

//listen for connection from client
	listen(sockfd,5);

	while(1)
	{
		//parent process waiting to accept a new connection
		printf("\nWaiting for new client connection:\n");
		clientAddressLength=sizeof(clientAddress);
		newsockfd=accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);

		//child process is created for serving each new clients
		pid=fork();
		if(pid==0)//child process rec and send
		{
			close(sockfd);
			int i=0,count,res;
			char cmd[100],flg[100],arg[100];
			char user[200],pass[200];
			n=recv(newsockfd,user,200,0); //user authentication
			n=recv(newsockfd,pass,200,0);

			int auth=sys_auth_user(user,pass);
			if(auth==0)
			{
				char success[]="true";
				n=strlen(success)+1;
				send(newsockfd,success,n,0);
				
				//receive from client
				while(1)
				{
					count=0;

					n=recv(newsockfd,msg,MAXSZ,0);
					count=sscanf(msg,"%s %s %s",cmd,flg,arg);

					if(strcmp(cmd,"pwd")==0)
					{
						char curr_dir[1024];
						getcwd(curr_dir,sizeof(curr_dir));
						send(newsockfd,curr_dir,strlen(curr_dir),0);
					}
					else if(strcmp(cmd,"ls")==0)
					{
						char def1[1024];
						int num;
						system("ls > /tmp/new.txt");
						memset(def1,0,sizeof(def1));
						int fd=open("/tmp/new.txt",O_RDONLY);
	
						if((num=read(fd,def1,1024)) >0)
	  					{
	  						write(newsockfd,def1,num);
	 						memset(def1,0,sizeof(def1));

	   					}
						close(fd);
					}
					else if(strcmp(cmd,"cd")==0)
					{
						res=search2(flg);
						if(res!=1)
						{
							send(newsockfd,out,strlen(out),0);
						}
						else
						{
							char def[]="Done";
	 						chdir(flg);
	 						send(newsockfd,def,strlen(def),0);
						}
					}
					else if(strcmp(cmd,"get")==0)
					{
						char content[1024],size[1024];
						bzero(content,sizeof(content));
						
						char found[]="found";
						res=search1(flg);
  
						if(res==1)
     						{
     							//to tell the client that file is present and accessible
     							send(newsockfd,found,strlen(found),0); 
	
							long long int size;
							struct stat obj;
							stat(flg,&obj);
	  						int filehandle = open(flg, O_RDONLY);
 							size = obj.st_size;
 	
							if(filehandle == -1)
 	   							size = 0;
 	
							send(newsockfd, &size, sizeof(long long int), 0);
 							
 							if(size)
							{	 sendfile(newsockfd, filehandle, NULL, size);
	
							}
							close(filehandle);
	      					}
    						else
     						{
     							send(newsockfd,out,strlen(out),0); 
      						}
	
					}
					else if(strcmp(cmd,"put")==0)
					{
	 					int c = 0, len,filehandle,size;
	 					char *f;
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
						send(newsockfd,msg,n,0);//no valid command.. reply back same command
					}

				}
				exit(0);
			}
			else	
			{
				char fail[]="Authentication failure";
				n=strlen(fail);
				send(newsockfd,fail,n,0);
				close(newsockfd);
			}
		}
		else
		{
			//wait(status);
			close(newsockfd);//sock is closed BY PARENT
		}
	}
return 0;
}
