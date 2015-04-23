/*
Download files from server.
*/
void get_content(char *arg,char *user_input,int sockfd,char *home_dir)
{
	/* Temporary variables*/
	int no_of_bytes;
	int port;	
	int newsockfd;
	int total;
	int p;
	int fd;	
	int size;
	int file_size;
	int down = 2;
	int temp;
	int temp1;	

	char message_from_server[MAXSZ];
	char message_to_server[MAXSZ];
	char file[MAXSZ];// File name
	char file_name[MAXSZ];// File name with instruction to server
	char file_home_dir[MAXSZ];// Location of file i.e. User's home directory(Complete path to file).
	char data[MAXSZ];// Data transfer
	char size_file[MAXSZ];// Get file size

	/* Initialise all the character arrays */
	bzero(message_from_server,MAXSZ);
	bzero(message_to_server,MAXSZ);
	bzero(file_name,MAXSZ);
	bzero(file,MAXSZ);
	bzero(file_home_dir,MAXSZ);
	bzero(data,MAXSZ);
	bzero(size_file,MAXSZ);
	
	/* Tell server to change to BINARY mode */
	send(sockfd,"TYPE I\r\n",8,0);

	while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
	{
		message_from_server[no_of_bytes] = '\0';
		printf("%s",message_from_server);
		fflush(stdout);
		if(strstr(message_from_server,"200 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"504 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
			break;
	}
	printf("\n");	

	if(strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"504 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
		return;	
	
	/* Send request for PASSIVE connection */	
	send(sockfd,passive,strlen(passive),0);
	while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
	{
		message_from_server[no_of_bytes] = '\0';
		printf("%s",message_from_server);
		fflush(stdout);
		if(strstr(message_from_server,"227 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
			break;
	}
	
	printf("\n");
	if(strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
		return;
		
	/* Server accepts request and sends PORT variables */
	if(strncmp(message_from_server,"227",3) == 0)
	{
		/* Generate a PORT number using PORT variables */
		port = passive_port_number(message_from_server); 
	
		/* Connect to server using another PORT for file transfers */
		newsockfd = func_to_connect_passive(arg,port);
	//	fcntl(newsockfd,F_SETFL,FNDELAY);
	
		sprintf(file,"%s",user_input + 4);
		
		/* Getting file size from server*/
		sprintf(size_file,"SIZE %s\r\n",user_input + 4);
		send(sockfd,size_file,strlen(size_file),0);

		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			if(strstr(message_from_server,"213 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"550 ") > 0)
				break;
		}
		
		size = atoi(message_from_server + 4);/* Convertin string to integer */

/*		sprintf(message_to_server,"REST 100\r\n");
		send(sockfd,message_to_server,strlen(message_to_server),0);
		
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			printf("%s\n",message_from_server);
			fflush(stdout);
			if(message_from_server[no_of_bytes-2] == '\r' && message_from_server[no_of_bytes-1] == '\n')
				break;
		}
*/

		/* Send file name */
		sprintf(file_name,"RETR %s\r\n",user_input + 4);	
		send(sockfd,file_name,strlen(file_name),0);
				
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			printf("%s",message_from_server);
			fflush(stdout);
			if(strstr(message_from_server,"425 ") > 0|| strstr(message_from_server,"125 ") > 0 ||strstr(message_from_server,"150 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"550 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
				break;
		}
		printf("\n");
		
		/* Permission Denied */
		if(strncmp(message_from_server,"550",3) == 0 || strncmp(message_from_server,"425",3) == 0)
		{
			close(newsockfd);
				return;
		}
					
		sprintf(file_home_dir,"%s/%s",home_dir,file);
		
		/* Create file on client system */	
		fd = open(file_home_dir,O_CREAT|O_WRONLY|O_TRUNC,0644);			
		file_size = 0;		
		if(size % 100 == 0)
			temp = (size / 100);
		else
			temp = (size / 100) + 1;
		
		temp1 = temp;
		printf("Downloading [");
		fflush(stdout);
		while((no_of_bytes = recv(newsockfd,data,MAXSZ,0))>0)
		{
			total = 0;
			file_size += no_of_bytes;
			temp1 = temp * down;
			
			while(temp1 <= file_size)
			{
				printf("#");
				fflush(stdout);
				down += 2;
				temp1 = temp * down;
			}
			/* For partial write operations */
			while(total < no_of_bytes)
                	{
                	        p = write(fd,data + total,no_of_bytes - total);
                       		total += p;
			}
			
		}
		printf("] 100%%\n");
		fflush(stdout);
			
		/* Close PASSIVE socket and file */
		close(newsockfd);	
		close(fd);
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			printf("%s",message_from_server);
			fflush(stdout);
			if(strstr(message_from_server,"226 ") > 0)
				break;
		}
			printf("\n");
		
	}
}
		

