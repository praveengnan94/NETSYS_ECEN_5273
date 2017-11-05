#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <signal.h>

int socket_desc,port_number,c,client_sock;
struct sockaddr_in server;
long file_size;
int nbytes;
int main(int argc,char**argv)
{
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("\n\nSocket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons( 10002 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 10);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //ACCEPT LOOP 
    while(1)
    { 
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)NULL, NULL);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        
        char buffer[1000];
        FILE *fp;
        //RECEIVE CHUNK SIZE FIRST THEN THE CONTENTS
        if((nbytes =  recv(client_sock,buffer,sizeof(long),0) ) < 0)
        {
            printf("ERROR recv\n");
            break;
        }
        file_size= atoi(buffer);
        char * file_chunk = (char*) malloc(file_size);
        if((nbytes =  recv(client_sock,file_chunk,file_size,0) ) < 0)
        {
            printf("ERROR recv\n");
            break;
        }
        
        fp=fopen("ws1.conf","w");
        fwrite(file_chunk,sizeof(char),file_size,fp);
        fclose(fp);
        printf("theACCEPT\n");

        close(client_sock);
    }
    return 0;
}