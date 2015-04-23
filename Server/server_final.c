#include"login_user.h"

int passive_connection(int newsockfd)
{
	int n;
    	int err, i , sock;
	int var = 0;
	int port;	
	int p1,p2;
	int optval = 1;

        struct ifreq ifr;
	struct sockaddr_in sa;
     
        char array[] = "eth0";
	char address[20];
	char message_to_client[MAXSZ];

	bzero(address,sizeof(address));
	bzero(message_to_client,sizeof(message_to_client));
    	bzero(&sa,sizeof(sa));

        n = socket(AF_INET, SOCK_DGRAM, 0);
	
       	//Type of address to retrieve - IPv4 IP address
       	ifr.ifr_addr.sa_family = AF_INET;

     	//Copy the interface name in the ifreq structure
    	strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    	ioctl(n, SIOCGIFADDR, &ifr);
     	sprintf(address,"%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
     	close(n);
    
	struct sockaddr_in saa;
    	bzero(&saa,sizeof(saa));
	saa.sin_family = AF_INET;
	saa.sin_addr.s_addr = inet_addr("127.0.0.1");
    
	for( i = 1025 ; i <= 64000 ; i++) 
	{
		if( i == PORT)
        	{
			continue;
	        }
 
	        saa.sin_port = htons(i);
	        sock = socket(AF_INET, SOCK_STREAM , 0);
		setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
	
       		if(sock < 0)
	        {    
	        	continue;
		}

	        err = connect(sock , (struct sockaddr*)&saa , sizeof(saa));

        	if(err >= 0)
		{
        	    	var = 1;
			port = i;
        		close(sock);
			break;
        	}
        	close(sock);
    	}	
	
	i = 0;     

	while(address[i] != '\0')
	{
		if(address[i] == '.')
			address[i] = ',';
		i++;
	}

	if(var == 1)
	{
		p2 = port % 256;
		p1 = port/256;
		sprintf(message_to_client,"227 Entering Passive Mode (%s,%d,%d).\r\n",address,p1,p2);
	}
	else
	{
		sprintf(message_to_client,"421 Could not create passive connection.\r\n");
	
	}
	
	send(newsockfd,message_to_client,strlen(message_to_client),0);

	if(var == 1)
		return port;
	else
		return -1;
}

int file_present(char file[])
{
	DIR *fd=opendir(".");
	struct dirent *entry=readdir(fd);
	struct stat file_buff;
	
	while(entry != NULL)
	{
		if(strlen(entry->d_name) == strlen(file) && strcmp(entry->d_name,file) == 0 )
		{
			stat(file,&file_buff);
        
			if(S_ISDIR(file_buff.st_mode))
				return 1;
			else
				return 0;
		}
		entry=readdir(fd);
	}
	return 0;
}


int directory_present(char file[])
{
	struct stat buf;

	bzero(out,sizeof(out));

	if(access(file,F_OK) == 0)
	{
		if(access(file,R_OK) == 0)
		{
			lstat(file,&buf);
		
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

int main(int argc, char *argv[])
{

	int sockfd;
	int newsockfd;
	int ret;
	int no_of_bytes;

	pid_t pid;
	socklen_t length;	

	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	
	struct stat file_buff;

	char message_from_client[MAXSZ];
	char message_to_client[MAXSZ];
	char instruction[MAXSZ];
	char curr_dir[MAXSZ];
	char def[]="250 Directory successfully changed.\n";
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1)
	{
		perror("Error");
		exit(1);
	}

	bzero(&serverAddress,sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);
	
	bind(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
	
	listen(sockfd,20);
	
	while(1)
	{
		length = sizeof(clientAddress);

		newsockfd = accept(sockfd,(struct sockaddr *)&clientAddress,&length);
		if(newsockfd == -1)
			continue;

		pid = fork();
		
		if(pid == 0)
		{
			struct sockaddr_in serv_Address;
			struct sockaddr_in cli_Address;
	
			int passive_fd;
			int passive_new_fd;
			int port;
			int file_handle;
			int p;	
			int total;
			int optval = 1;	

			char data[MAXSZ];
		
			close(sockfd);
			
			ret = login(newsockfd);
			
			if(ret == 1)
			{
				while(1)
				{
					bzero(message_from_client,MAXSZ);
					bzero(message_to_client,MAXSZ);
					bzero(instruction,MAXSZ);
					bzero(curr_dir,MAXSZ);	
					bzero(data,MAXSZ);	
	
					while((no_of_bytes = recv(newsockfd,message_from_client,MAXSZ,0)) > 0)
					{
						
						message_from_client[no_of_bytes] = '\0';
						strcat(instruction,message_from_client);
						if(message_from_client[no_of_bytes - 2] == '\r' && message_from_client[no_of_bytes - 1] == '\n')
							break;
					}

					if(instruction[strlen(instruction) - 1] == '\n')
						instruction[strlen(instruction) - 1] = '\0';
					if(instruction[strlen(instruction) - 1] == '\r')
						instruction[strlen(instruction) - 1] = '\0';

					if(strcmp(instruction,"PWD") == 0)
					{
						getcwd(curr_dir,sizeof(curr_dir));
					
						sprintf(message_to_client,"%s\r\n",curr_dir);
						send(newsockfd,message_to_client,strlen(message_to_client),0);
					}
					
					if(strncmp(instruction,"CWD ",3) == 0)
					{
						ret = directory_present(instruction+3);
						if(ret != 1)
						{
							sprintf(message_to_client,"%s\r\n",out);
							send(newsockfd,message_to_client,strlen(message_to_client),0);
						}
						else
						{
							chdir(instruction + 3);
							sprintf(message_to_client,"%s\r\n",def);
							send(newsockfd,message_to_client,strlen(message_to_client),0);
						}
						
					}
					
					if(strcmp(instruction,"SYST") == 0)
					{
						struct utsname sys;
						uname(&sys);	
						sprintf(message_to_client,"215 %s\r\n",sys.sysname);
						send(newsockfd,message_to_client,strlen(message_to_client),0);
					}

					if(strcmp(instruction,"QUIT") == 0)
					{
						sprintf(message_to_client,"221 Goodbye.\r\n");
						send(newsockfd,message_to_client,strlen(message_to_client),0);
						close(newsockfd);
						exit(0);
					}

					if(strcmp(instruction,"TYPE I") == 0)
					{
						sprintf(message_to_client,"200 Switching to Binary mode.\r\n");
						send(newsockfd,message_to_client,strlen(message_to_client),0);
					}

					if(strcmp(instruction,"PASV") == 0)
					{
						port = passive_connection(newsockfd);
						if(port == -1)
							continue;
	
						passive_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
						setsockopt(passive_fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));

						bzero(&serv_Address,sizeof(serv_Address));
						serv_Address.sin_family = AF_INET;
						serv_Address.sin_port = htons(port);
						serv_Address.sin_addr.s_addr = htonl(INADDR_ANY);
						bind(passive_fd,(struct sockaddr *)&serv_Address,sizeof(serv_Address));
						listen(passive_fd,20);
						length = sizeof(cli_Address);
						
						passive_new_fd = accept(passive_fd,(struct sockaddr *)&cli_Address,&length);
					}
					
					if(strncmp(instruction,"SIZE ",5) == 0)
					{
						
						if(lstat(instruction + 5,&file_buff) == 0)
							sprintf(message_to_client,"213 %llu\r\n",(unsigned long long)file_buff.st_size);
						else
							sprintf(message_to_client,"550 Permission denied.\r\n");
							
						send(newsockfd,message_to_client,strlen(message_to_client),0);
					}
					
					if(strncmp(instruction,"RETR ",5) == 0)
					{
						
						if(port == -1)
						{
							sprintf(message_to_client,"421 Passive connection not established.\r\n");
							send(newsockfd,message_to_client,strlen(message_to_client),0);
							continue;

						}

						if(access(instruction + 5,F_OK) == 0)
						{
		
							if(access(instruction + 5,R_OK) == 0)
							{
								
								lstat(instruction + 5,&file_buff);
								sprintf(message_to_client,"150 Opening BINARY mode data connection for %s (%d bytes).\r\n",instruction + 5,(int)file_buff.st_size);
							}
							else
								sprintf(message_to_client,"550 Permission denied.\r\n");
						}
						else	
							sprintf(message_to_client,"550 Failed to open file.\r\n");
						send(newsockfd,message_to_client,strlen(message_to_client),0);
						
						file_handle = open(instruction + 5,O_RDONLY);
						if(file_handle == -1)
						{
							close(file_handle);
							close(passive_new_fd);
							close(passive_fd);
							continue;
						}
						while((no_of_bytes = read(file_handle,data,MAXSZ)) > 0)
						{
							total = 0;
							while(total < no_of_bytes)
							{
								p = send(passive_new_fd,data + total,no_of_bytes - total, 0);
								total += p;
							}
						}

						close(file_handle);
						close(passive_new_fd);
						close(passive_fd);
					
						bzero(message_to_client,MAXSZ);
						sprintf(message_to_client,"226 Transfer Complete.\r\n");
							
						send(newsockfd,message_to_client,strlen(message_to_client),0);
							
					}
					
					if(strncmp(instruction,"STOR ",5) == 0)
					{
						
						if(port == -1)
						{
							sprintf(message_to_client,"421 Passive connection not established.\r\n");
							send(newsockfd,message_to_client,strlen(message_to_client),0);
							continue;

						}

						if(access(instruction + 5,F_OK) != 0)
						{
		
							if((file_handle = open(instruction + 5,O_WRONLY|O_CREAT,0644)) > 0)
							{
								
								sprintf(message_to_client,"150 FILE: %s.\r\n",instruction + 5);
							}
							else
								sprintf(message_to_client,"550 Permission denied.\r\n");
						}
						else	
						{
							file_handle = -1;
							sprintf(message_to_client,"553 Cannot create file.\r\n");
						}

						send(newsockfd,message_to_client,strlen(message_to_client),0);
						
						if(file_handle == -1)
						{
							close(file_handle);
							close(passive_new_fd);
							close(passive_fd);
							continue;
						}

						while((no_of_bytes = recv(passive_new_fd,data,MAXSZ,0)) > 0)
						{
							total = 0;
							while(total < no_of_bytes)
							{
								p = write(file_handle,data + total,no_of_bytes - total);
								total += p;
							}
						}

						close(file_handle);
						close(passive_new_fd);
						close(passive_fd);
					
						bzero(message_to_client,MAXSZ);
						sprintf(message_to_client,"226 Transfer Complete.\r\n");
							
						send(newsockfd,message_to_client,strlen(message_to_client),0);
				}
		
				}				
			}
			else
			{
				close(newsockfd);

			}	
		
		}
		else
		{
			wait(NULL);
			close(newsockfd);
	
		}
	}
	

	return 0;
}
