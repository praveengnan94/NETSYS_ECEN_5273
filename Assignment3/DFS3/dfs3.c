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
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <signal.h>


int total_users=0;
FILE * conf_file;
char dfs_path[50];
char directory_path[100];
typedef struct userinfo { 
    char username[50]; 
    char password[50]; 
    } user_info;
user_info user_infos[50];
char *pos;
int socket_desc,c,client_sock;
struct sockaddr_in server;

struct stat st = {0};// to check if directory exists
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
    //PARSING THE DFS.CONF FILE AND STORING ITS RESULTS
   while (1) 
   {
        if (fgets(line,300, conf_file) == NULL) 
            break;
        pch=strtok(line," ");
        strcpy(user_infos[total_users].username,pch);
        strcpy(directory_path,user_infos[total_users].username);
        strcat(directory_path,"/");
       //CHECK if directory under the username exists, else create a directory
        if (stat(directory_path, &st) == -1) {
            printf("New directory created \n");
            mkdir(directory_path, 0777);
        }

        pch=strtok(NULL," ");
        strcpy(user_infos[total_users].password,pch);
        
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
    server.sin_port = htons( 10003 );
     
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

       //if forking, fork from here
        long file_size;
        int nbytes;
        char username[10][40];
        char password[10][40];
        char received_username[60];
        char received_password[60];
        char buffer[200];
        char usr_cmd[10];
        char chunk_numbers[10];
        int chunk_num1,chunk_num2;
        char * file_chunk;
        char file_name[50],file_name1[50],file_name2[50];
        char temp_filenum[2];
        FILE *fp;
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)NULL, NULL);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        int countt;
        int usrname_flag=0;
        //RECEIVE USERNAME AND PASSWORD AND VALIDATE
        if((nbytes =  recv(client_sock,received_username,60,0) ) < 0)
        {
            printf("ERROR recv\n");
            break;
        }

        if ((pos=strchr(received_username, '\n')) != NULL)
            *pos = '\0';
        if((nbytes=send(client_sock, "ACK", strlen("ACK"),0)== -1))
            {
                printf("SENDING ACK TO DFS1 FAILED\n");
            }

        for(countt=0;countt<total_users;countt++)
        {
            if(strcmp(received_username,user_infos[countt].username)==0)
            {
                printf("Received user name %s is present\n",received_username);
                if((nbytes =  recv(client_sock,received_password,60,0) ) < 0)
                    {
                        printf("ERROR in receiving password recv\n");
                        break;
                    }
                if((nbytes=send(client_sock, "ACK", strlen("ACK"),0)== -1))
                    {
                        printf("SENDING ACK TO DFS1 FAILED\n");
                    }
                if ((pos=strchr(received_password, '\n')) != NULL)
                *pos = '\0';
               printf("Received password %s is present\n",received_password);
                if(strcmp(received_password,user_infos[countt].password)==0)
                {
                    printf("Valid username and password\n");
                    usrname_flag=1;
                    break;
                }
                else
                {
                    printf("Invalid username and password credentials\n");
                } 
            }
        }

        if(usrname_flag!=1)
        {
            printf("User name/Password don't match\n");
            strcpy(buffer,"Invalid Username/Password.Please try again.");
            if((nbytes=send(client_sock,buffer, strlen(buffer),0)== -1))
            {
                printf("SENDING ACK TO DFS1 FAILED\n");
            }
            continue;
        }
        //RECEIVE THE USER_COMMAND AND SEND ACK
        if((nbytes = recv(client_sock,buffer,sizeof(buffer),0) ) < 0)
        {
            printf("ERROR in receiving password recv\n");
            break;
        }
        if((nbytes=send(client_sock, "ACK", strlen("ACK"),0)== -1))
        {
            printf("SENDING ACK TO DFS1 FAILED\n");
        }
        strcpy(usr_cmd,buffer);
        if(strcmp(usr_cmd,"PUT")==0)
        {
            printf("PUT Command received\n");
            //RECEIVE FILE CHUNK NUMBERS BASED ON SPLIT VALUE
            if((nbytes = recv(client_sock,buffer,sizeof(buffer),0) ) < 0)
            {
                printf("ERROR in receiving password recv\n");
                break;
            }
            if((nbytes=send(client_sock, "ACK", strlen("ACK"),0)== -1))
            {
                printf("SENDING ACK TO DFS1 FAILED\n");
            }
            strcpy(chunk_numbers,buffer);
            pch=strtok(chunk_numbers," ");
            strcpy(chunk_numbers,pch);
            chunk_num1=atoi(chunk_numbers);
            pch=strtok(NULL,"");
            chunk_num2=atoi(pch);
            printf("CHUNK Nums are %d %d\n",chunk_num1,chunk_num2);

            //RECEIVE FILE NAME AND SEND ACK
            if((nbytes = recv(client_sock,buffer,sizeof(buffer),0) ) < 0)
            {
                printf("ERROR in receiving password recv\n");
                break;
            }
            if((nbytes=send(client_sock, "ACK", strlen("ACK"),0)== -1))
            {
                printf("SENDING ACK TO DFS1 FAILED\n");
            }

            strcpy(file_name,buffer);
            //CHECK if file_name has / in it
            pch=strtok(file_name,"/");
            if(pch!=NULL)
            {
                while(pch!=NULL)
                {
                    printf("pch %s\n",pch);
                    strcpy(file_name,pch);
                    pch=strtok(NULL,"/");

                }
            }
            else
            {
                strcpy(file_name,buffer);
            }
            // printf("FILE ANEM is %s\n",file_name);

            //RECEIVE CHUNK SIZE FIRST THEN THE CONTENTS
            if((nbytes =  recv(client_sock,buffer,sizeof(long),0) ) < 0)
            {
                printf("ERROR recv\n");
                break;
            }
            file_size= atoi(buffer);
            file_chunk = (char*) malloc(file_size);
            if((nbytes =  recv(client_sock,file_chunk,file_size,0) ) < 0)
            {
                printf("ERROR recv\n");
                break;
            }
            strcpy(file_name1,file_name);
            sprintf(temp_filenum,".%d",chunk_num1);
            strcat(file_name1,temp_filenum);
            printf("file name is %s\n",file_name1);
            fp=fopen(file_name1,"w");
            fwrite(file_chunk,sizeof(char),file_size,fp);

            if((nbytes =  recv(client_sock,buffer,sizeof(long),0) ) < 0)
            {
                printf("ERROR recv\n");
                break;
            }
            file_size= atoi(buffer);
            file_chunk = (char*) malloc(file_size);
            if((nbytes =  recv(client_sock,file_chunk,file_size,0) ) < 0)
            {
                printf("ERROR recv\n");
                break;
            }
            strcpy(file_name2,file_name);
            sprintf(temp_filenum,".%d",chunk_num2);
            strcat(file_name2,temp_filenum);
            printf("file name is %s\n",file_name2);
            fp=fopen(file_name2,"w");
            fwrite(file_chunk,sizeof(char),file_size,fp);

            fclose(fp);
            printf("theACCEPT\n");

            close(client_sock);
        }
    }
    return 0;
}