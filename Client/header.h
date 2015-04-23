#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<stdlib.h>//sizeof
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<ctype.h>//isdigit()
#include<fcntl.h>//open()
#include<dirent.h>
#include<sys/stat.h>//stat()
#include<grp.h>
#include<pwd.h>
#include<time.h>

#define PORT 21
#define MAXSZ 4096

#define MIN_VALUE -1
#define INITIALISE 0
#define MIN_IP 0
#define MAX_IP 255

char passive[]="PASV\r\n";
