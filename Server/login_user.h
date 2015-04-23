#include"header.h"
#include"authenticate.h"
	
int login(int newsockfd)
{
	char message_to_client[MAXSZ];
	char message_from_client[MAXSZ];
	char instruction[MAXSZ];
	char user[MAXSZ];
	char pass[MAXSZ];

	int n;
	int auth;

	bzero(message_from_client,MAXSZ);
	bzero(message_to_client,MAXSZ);
	bzero(instruction,MAXSZ);
	sprintf(message_to_client,"220 FTP ready.\r\n");
		
	send(newsockfd,message_to_client,strlen(message_to_client),0);
	bzero(message_to_client,MAXSZ);
		
	while((n = recv(newsockfd,message_from_client,MAXSZ,0)) > 0)
	{
		message_from_client[n] = '\0';
		strcat(instruction,message_from_client);

		if(strncmp(message_from_client,"USER ",5) == 0)
		{				
			sprintf(message_to_client,"331 Please specify password \r\n");
		}
		if(message_from_client[n-2] == '\r' && message_from_client[n-1] == '\n')
			break;  
				
	}			

	if(instruction[strlen(instruction)-1] == '\n')  
		instruction[strlen(instruction)-1] = '\0'; 
	if(instruction[strlen(instruction)-1] == '\r')  
		instruction[strlen(instruction)-1] = '\0'; 
			
	strcpy(user,instruction + 5);

	send(newsockfd,message_to_client,strlen(message_to_client),0);
	
	bzero(instruction,sizeof(instruction));

	while((n = recv(newsockfd,message_from_client,sizeof(message_from_client),0)) > 0)
	{
		message_from_client[n] = '\0';
		strcat(instruction,message_from_client);

		if(message_from_client[n-2] == '\r' && message_from_client[n-1] == '\n')
			break;  
				
	}			

	if(instruction[strlen(instruction)-1] == '\n')  
		instruction[strlen(instruction)-1] = '\0'; 
	if(instruction[strlen(instruction)-1] == '\r')  
		instruction[strlen(instruction)-1] = '\0'; 
			
	strcpy(pass,instruction+5);
			
	auth = sys_auth_user(user,pass);

	if(auth == 0)
	{
		send(newsockfd,success,strlen(success),0);
		return 1;
	}
	else
	{
		send(newsockfd,fail,strlen(fail),0);
		return -1;
	}
}
