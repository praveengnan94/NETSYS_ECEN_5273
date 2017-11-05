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
char username[10][40];
char password[10][40];
int total_users=0;
FILE * conf_file;
char dfs_path[50];

typedef struct userinfo { 
    char username[50]; 
    char password[50]; 
    } user_info;
user_info user_infos[50];

int main(int argc,char**argv)
{
   
   if(argv[1]==NULL)
   {
    printf("Please enter a path for dfs.conf file\n");
    exit(-1);
   }
   printf("DFS path is %s\n",argv[1]);

   strcpy(dfs_path,argv[1]);
    //PARSE THE CONF FILE AND STORE THE INFO
   conf_file= fopen(dfs_path,"rb");
   if(conf_file== 0)
   {
    printf("CANNOT OPEN CONF FILE\n\r");
    exit(-1);
   }
   else
   {
    printf("CONF FILE OPENED\n");
    }
   char line[60],*pch;
    //PARSING THE WS.CONF FILE AND STORING ITS RESULTS
   while (1) 
   {
        if (fgets(line,300, conf_file) == NULL) 
            break;
        pch=strtok(line," ");
        strcpy(user_infos[total_users].username,pch);
        pch=strtok(NULL," ");
        strcpy(user_infos[total_users].password,pch);
        char *pos;
        if ((pos=strchr(user_infos[total_users].password, '\n')) != NULL)
            *pos = '\0';
        printf("%s %s\n",user_infos[total_users].username,user_infos[total_users].password);
        total_users++;
   }
   printf("TOTAL users are %d\n",total_users );
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created\n");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons( 10001 );
     
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
        
        char buffer[10];
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