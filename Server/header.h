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
#include<sys/ioctl.h>
#include<net/if.h>
#include<sys/utsname.h>
#include<sys/wait.h>

#define PORT 8000
#define MAXSZ 1024

char out[MAXSZ];
char fail[]="530 Login incorrect.\r\n";
char success[]="230 Login correct.\r\n";

