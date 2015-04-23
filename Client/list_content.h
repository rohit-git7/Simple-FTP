/*
List the contents of current working directory on server (`ls` and `ls -l` linux commands).
*/
void list_content(char *arg, char *user_input, int sockfd)
{
	/* Temporary variables */
	int no_of_bytes;
	int port;	
	int newsockfd;
	
	struct timeval tm;/* time structure to set time wait for receive buffer */
	tm.tv_sec = 4;
	tm.tv_usec = 750000;

	char message_from_server[MAXSZ];
	char message_to_server[MAXSZ];
	
	/* Initialise character arrays */
	bzero(message_from_server,MAXSZ);
	bzero(message_to_server,MAXSZ);

	/* Request server to start BINARY mode */
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

	/* Request server to connect to PASSIVE port for file transfers */
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

	/* Request acepted. Connect to PASSIVE port */
	if(strncmp(message_from_server,"227",3)== 0)
	{
		
		/* Generate PORT address */		
		port = passive_port_number(message_from_server); 

		/* Create socket for PASSIVE connection */
		newsockfd =  func_to_connect_passive(arg,port);
					
		if(strcmp(user_input,"ls -l") == 0)
			sprintf(message_to_server,"LIST -l\r\n");/* ls -l */
		else
			sprintf(message_to_server,"NLST\r\n");/* ls */
					
		send(sockfd,message_to_server,strlen(message_to_server),0);
		
		
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';
			printf("%s",message_from_server);
			fflush(stdout);
			
			if(strstr(message_from_server,"425 ") > 0|| strstr(message_from_server,"125 ") > 0||strstr(message_from_server,"150 ") > 0 || strstr(message_from_server,"501 ") > 0 ||strstr(message_from_server,"500 ") > 0 ||strstr(message_from_server,"502 ") > 0 ||strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"530 ") > 0)
				break;
		}
		printf("\n");
	
		if(strncmp(message_from_server,"125",3) != 0 && strncmp(message_from_server,"150",3) != 0)
			return;

		/* Read data on new PASSIVE socket */		
		int new_fd = open("temp.txt",O_WRONLY|O_CREAT|O_TRUNC);
				
		while((no_of_bytes = recv(newsockfd,message_from_server,MAXSZ,0)) > 0)
		{
			write(new_fd,message_from_server,no_of_bytes);
			message_from_server[no_of_bytes] = '\0';
			printf("%s",message_from_server);
			fflush(stdout);
		
		}
		printf("\n");
		close(new_fd);
		close(newsockfd);/* Close PASSIVE connection */
	
		/* Set time boundation on receive buffer */
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)&tm,sizeof(tm));
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{	
			message_from_server[no_of_bytes] = '\0';
			printf("%s",message_from_server);
			fflush(stdout);	
			if(strstr(message_from_server,"226 ") > 0)
				break;
		}
		printf("\n");
	
	tm.tv_sec = 0;
	tm.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)&tm,sizeof(tm));
	}

}	

