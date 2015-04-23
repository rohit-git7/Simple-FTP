#include"header.h"
#include"list_client.h"
#include"passive_connect.h"
#include"list_content.h"
#include"get_content.h"
#include"put_content.h"
#include"put_unique.h"

/*Get home directory of user executing program */
char *find_home_dir(char *file)
{
	struct passwd *pw;
	char *sudo_uid = getenv("SUDO_UID");
	pw = getpwuid(atoi(sudo_uid));
	
	return pw->pw_dir;

}

/*Validating IP Address*/
int validate_ip(char *ip)
{
	int value_1 = MIN_VALUE;
	int value_2 = MIN_VALUE;
	int value_3 = MIN_VALUE;
	int value_4 = MIN_VALUE;
	int count = INITIALISE;
	int i = INITIALISE;

	while(ip[i] != '\0')
	{
		if(ip[i] == '.')
			count++;
		i++;
	}
	
	if(count != 3 )
		return -1;
	else
	{
		sscanf(ip,"%d.%d.%d.%d",&value_1,&value_2,&value_3,&value_4);
		
		if(value_1 < MIN_IP || value_2 < MIN_IP || value_3 < MIN_IP || value_4 < MIN_IP || value_1 > MAX_IP || value_2 > MAX_IP || value_3 > MAX_IP || value_4 > MAX_IP)/* IP Addresses from 0.0.0.0 to 255.255.255.255*/
			return -1;
		else
			return 1;

	}

}

int main(int argc, char *argv[])
{
	int sockfd;/* to create socket */
	int no_of_bytes;/* number of bytes sent or received from server */
	
	/*Temporary Variables*/
	int connect_value;
	int ip_valid;
	int temp = MIN_VALUE;
	int count;
	int dir_check;

	clock_t start,end;
	double cpu_time;

	struct sockaddr_in serverAddress;/* client will connect on this */
	
	char ch[MAXSZ];
	char message_from_server[MAXSZ];/* message from server*/
	char user_input[MAXSZ];/* input from user */
	char message_to_server[MAXSZ];/* message to server */
	char user[MAXSZ];/* user details sent to server */
	char pass[MAXSZ];/* password details sent to server */
	char dir[MAXSZ];/* directory name */
	char username[MAXSZ];/* username entered by the user */
	char working_dir[MAXSZ];
	char old_name[MAXSZ];
	char new_name[MAXSZ];

	char *home_dir;
	char *password = malloc(MAXSZ);/* password enterd by user */

	if(argc != 2) /* `./executable ip-adddress` */
	{
		printf("Error: argument should be ip-address of server\n");
		exit(1);
	}
	
	ip_valid = validate_ip(argv[1]);/* Validate ip-address entered by user */
	
	if(ip_valid == MIN_VALUE)/* Invalid ipaddress */
	{
		printf("Error: Invalid ip-address\n");
		exit(1);
	}
	
	home_dir = find_home_dir(argv[0]);/* Home directory of user executing the program */

	sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);/* Create socket */

	if(sockfd == -1)/* Error in socket creation */
	{
        	perror("Error"); 
		exit(1);
	}

	bzero(&serverAddress,sizeof(serverAddress));/* Initialise structure */

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(PORT);

	/* Connect to server */
	connect_value = connect(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
	if(connect_value == -1)/* Connection Error */
	{
       		perror("Error");
        	exit(1);
	}

	printf("Connected to %s.\n",argv[1]);

	/* Receive message from server "Server will send 220" */
	while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
	{
		message_from_server[no_of_bytes] = '\0';
		printf("%s\n",message_from_server);
		fflush(stdout);
	
		if(strstr(message_from_server,"220 ") > 0 || strstr(message_from_server,"421 ") > 0)	
			break;
	}

	if(strstr(message_from_server,"421 ") > 0)	
		exit(1);
	
	printf("Name (%s): ",argv[1]);
	scanf("%s",username);/* Enter name of user on server */
	
	sprintf(user,"USER %s\r\n",username);
		
	send(sockfd,user,strlen(user),0);/* Send username to server */
	

	/*
		Receive message from server after sending user name.
		Message with code 331 asks you to enter password corresponding to user.
		Message with code 230 means no password is required for the entered username(LOGIN successful).
	*/
	while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
	{	
		message_from_server[no_of_bytes] = '\0';
		if(strncmp(message_from_server,"331",3) == 0)
		{
			temp = 1;
		}
	
		if(strncmp(message_from_server,"230",3) == 0)
		{
			temp = 2;
		}
		
		if(strncmp(message_from_server,"530",3) == 0)
		{
			temp = 0;	
		}
		printf("%s\n",message_from_server);
		if(strstr(message_from_server,"230 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"332 ") > 0 || strstr(message_from_server,"530 ")|| strstr(message_from_server,"331 ") > 0)
			break;
		fflush(stdout);
	}

	if(temp == 1)
	{
		password = getpass("Password: ");/* Enter password */
		sprintf(pass,"PASS %s\r\n",password);
		
		send(sockfd,pass,strlen(pass),0);/* Send password to server */

		/* Receive message from server */
		while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
		{
			message_from_server[no_of_bytes] = '\0';

			if(strncmp(message_from_server,"230",3) == 0)
			{
				temp = 2;
			}
			
			if(strncmp(message_from_server,"530",3) == 0)
			{
				temp = 0;	
			}
			printf("%s\n",message_from_server);

			if(strstr(message_from_server,"230 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"332 ") > 0 || strstr(message_from_server,"530 ")|| strstr(message_from_server,"503 ") > 0 || strstr(message_from_server,"202 ") > 0)
				break;

			fflush(stdout);
			
		}
	}	

	if(temp == 0)
	{
		exit(1);
	}
	

	/* Systen type(Server) */
	sprintf(message_to_server,"SYST\r\n");
	send(sockfd,message_to_server,strlen(message_to_server),0);

	while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
	{
		message_from_server[no_of_bytes] = '\0';
		printf("%s\n",message_from_server);
		if(strstr(message_from_server,"215 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0) 	
			break;

	}

	
	/* Infinite Loop for user operation */
	while(1)
	{
		temp = 0;
		
		printf("ftp> ");
		fflush(stdout);
	
		/* Initialise strings */
		bzero(user_input,MAXSZ);
		bzero(message_to_server,MAXSZ);
		bzero(message_from_server,MAXSZ);
		bzero(working_dir,MAXSZ);
		bzero(old_name,MAXSZ);
		bzero(new_name,MAXSZ);
	
		/* Read user input */
		no_of_bytes = read(STDIN_FILENO,user_input,MAXSZ);
		user_input[no_of_bytes] = '\0';		
		
		/* Remove trailing return and newline characters */
		if(user_input[no_of_bytes - 1] == '\n')
			user_input[no_of_bytes - 1] = '\0';
		if(user_input[no_of_bytes - 1] == '\r')
			user_input[no_of_bytes - 1] = '\0';

		/* User wants to exit */
		if(strcmp(user_input,"exit") == 0 || strcmp(user_input,"quit") == 0 || strcmp(user_input,"bye") == 0)
		{
			sprintf(message_to_server,"QUIT\r\n");
		
			/* Send message to server */
			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
			{
				message_from_server[no_of_bytes] = '\0';
				printf("%s\n",message_from_server);
				if(strstr(message_from_server,"221 ") > 0 || strstr(message_from_server,"500 ") > 0)	
					break;
			}
			break;
		}

		/* Change directory on client side */
		if(strncmp(user_input,"!cd ",4) == 0 || strcmp(user_input,"!cd") == 0)
		{
			if(chdir(user_input + 4) == 0)
			{
				printf("Directory successfully changed\n\n");
			}			
			else
			{
				perror("Error");
			}
		}

		/* Current working directory on client side */
		if(strncmp(user_input,"!pwd ",5) == 0 || strcmp(user_input,"!pwd") == 0)
		{
			getcwd(working_dir,MAXSZ);	
			printf("%s\n\n",working_dir);
		}	

		/* List files with details in current working directory on client side */
		if(strncmp(user_input,"!ls -l ",7) == 0 || strcmp(user_input,"!ls -l") == 0)
		{
			getcwd(working_dir,MAXSZ);	
			ls_l_dir(working_dir);
			continue;
		}
		
		/* List files in current working directory on client side */	
		if(strncmp(user_input,"!ls ",4) == 0 || strcmp(user_input,"!ls") == 0)
		{
			getcwd(working_dir,MAXSZ);	
			ls_dir(working_dir);
		}
		
		/* Create directory on client side */
		if(strncmp(user_input,"!mkdir ",7) == 0)
		{
			dir_check = mkdir(user_input + 7,0755);
			if(dir_check == -1)
				perror("Error");
			else
				printf("Directory successfully created\n");
			printf("\n");
		}

		/* Remove directory on client side */
		if(strncmp(user_input,"!rmdir ",7) == 0)
		{
			dir_check = rmdir(user_input + 7);
			if(dir_check == -1)
				perror("Error");
			else
				printf("Directory successfully removed\n");
			printf("\n");
		}		
	
	
		/* Change directory on server side */
		if(strncmp(user_input,"cd ",3) == 0)
		{
			sprintf(dir,"CWD %s\r\n",user_input + 3);
			send(sockfd,dir,strlen(dir),0);
		
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
			{
				message_from_server[no_of_bytes] = '\0';
				printf("%s\n",message_from_server);
				fflush(stdout);
				
				if(strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"250 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;
			}
		}
	
		//* List files on server side */
		if(strncmp(user_input,"ls ",3)== 0 || strcmp(user_input,"ls")== 0)
		{
			list_content(argv[1],user_input,sockfd);	
		}
	
		/* Current working directory on server side */
		if(strcmp(user_input,"pwd")== 0)
		{
			sprintf(message_to_server,"PWD\r\n");
			send(sockfd,message_to_server,strlen(message_to_server),0);
		
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0)
			{
				message_from_server[no_of_bytes] = '\0';
				printf("%s\n",message_from_server);
				fflush(stdout);
				
				if(strstr(message_from_server,"257 ") > 0 || strstr(message_from_server,"500 ") > 0 || strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;
			}
		}

		/* Download file from server */
		if(strncmp(user_input,"get ",4) == 0)
		{
			start = clock();
			get_content(argv[1],user_input,sockfd,home_dir);
			end = clock();
			cpu_time = ((double)(end - start))/CLOCKS_PER_SEC;
			printf("Time taken %lf\n\n",cpu_time);
			
		}
		
		/* Upload file to server */
		if(strncmp(user_input,"put ",4)== 0)
		{
			put_content(argv[1],user_input,sockfd);
		}

		/* Upload file uniquely to server */
		if(strncmp(user_input,"uniqput ",8)== 0)
		{
			put_unique(argv[1],user_input,sockfd);
		}
		
		/* Rename file on server */	
		if(strncmp(user_input,"rename ",7) == 0)
		{
			
			/* parse user input to get old file name and new file name */
			count = sscanf(user_input,"%s %s %s",message_to_server,old_name,new_name);
	
			if(count != 3)
			{		
				printf("Error: rename expects two arguments\n\n");
				continue;
			}			

			sprintf(message_to_server,"RNFR %s\r\n",old_name);
		
			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
			{
				message_from_server[no_of_bytes] = '\0';
		
				if(strncmp(message_from_server,"550",3) == 0)/* RNFR fails*/
				{
					temp = 1;	
				}
					printf("%s\n",message_from_server);
				
				fflush(stdout);
				
				if(strstr(message_from_server,"350 ") > 0 || strstr(message_from_server,"450 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"500 ") > 0|| strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;	
			}
			
			if(temp  == 1)
				continue;

			sprintf(message_to_server,"RNTO %s\r\n",new_name);

			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
			{
				message_from_server[no_of_bytes] = '\0';
				if(strncmp(message_from_server,"550",3) == 0)/* RNTO fails*/
				{
					printf("Error: Renaming file failed.\n\n");
				}
				else
					printf("%s\n",message_from_server);
				
				fflush(stdout);
				
				if(strstr(message_from_server,"553 ") > 0 ||strstr(message_from_server,"250 ") > 0 || strstr(message_from_server,"532 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"500 ") > 0|| strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"503 ") > 0)
					break;

			}
	

		}

		/* Creating diectory on server */
		if(strncmp(user_input,"mkdir ",6) == 0)
		{	
			sprintf(message_to_server,"MKD %s\r\n",user_input + 6);
			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
			{
				message_from_server[no_of_bytes] = '\0';
				if(strncmp(message_from_server,"550",3) == 0)/* MKD fails*/
				{
					printf("Error: Creating directory failed.\n\n");
				}
				else
					printf("%s\n",message_from_server);
				
				fflush(stdout);

				if(strstr(message_from_server,"257 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"500 ") > 0|| strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;
			
			}
		}

		/* Removing directory on server */
		if(strncmp(user_input,"rmdir ",6) == 0)
		{	
			sprintf(message_to_server,"RMD %s\r\n",user_input + 6);
			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
			{
				message_from_server[no_of_bytes] = '\0';
				if(strncmp(message_from_server,"550",3) == 0)/* RMD fails*/
				{
					printf("Error: Removing directory failed.\n\n");
				}
				else
					printf("%s\n",message_from_server);
				
				fflush(stdout);

				if(strstr(message_from_server,"250 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"500 ") > 0|| strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;	
			}
		}
		
		/* Delete file on server */
		if(strncmp(user_input,"rm ",3) == 0)
		{	
			printf("Do you really want to remove this file? yes/no \n");
			
			scanf("%s",ch);
			
			if(strcasecmp(ch,"yes") != 0)
				continue;

			sprintf(message_to_server,"DELE %s\r\n",user_input + 3);

			send(sockfd,message_to_server,strlen(message_to_server),0);
			while((no_of_bytes = recv(sockfd,message_from_server,MAXSZ,0)) > 0 )
			{
				message_from_server[no_of_bytes] = '\0';
				if(strncmp(message_from_server,"550",3) == 0)/* DEL fails*/
				{
					printf("Error: Removing file failed.\n\n");
				}
				else
					printf("%s\n",message_from_server);
				
				fflush(stdout);

				if(strstr(message_from_server,"250 ") > 0 || strstr(message_from_server,"450 ") > 0 || strstr(message_from_server,"530 ") > 0 || strstr(message_from_server,"500 ") > 0|| strstr(message_from_server,"501 ") > 0 || strstr(message_from_server,"421 ") > 0 || strstr(message_from_server,"502 ") > 0 || strstr(message_from_server,"550 ") > 0)
					break;
			
			}
		}


	}
        close(sockfd);
	return 0;
}
