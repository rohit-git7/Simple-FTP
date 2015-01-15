# Simple-FTP
Simple FTP server-client using C
Works only on linux.
Simplified version of FTP using socket programming, UNIX system calls.
I am working on it. I will push the changes. It is just a basic structure.

Files: client.c server.c

Commands that are currently implemented:
#get
#put
#ls
#pwd
#cd

I will add some functionalities like ls,cd,pwd on client side too.
I have not taken into account flags that can be used with commands. That is in my TODO list. 
Also working on chrooting ftp directory on server side.
