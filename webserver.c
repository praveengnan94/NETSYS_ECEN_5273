/*
    C socket server example
*/
 
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

void *handler(void *threadp);

pthread_t main_thread;
pthread_attr_t main_sched_attr;
int rt_max_prio, rt_min_prio, min;
struct sched_param main_param;
FILE * conf_file;
 
static struct timespec rtclk_dt = {0, 0};
static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_stop_time = {0, 0};
static struct timespec delay_error = {0, 0};

int nbytes;
unsigned int server_length;

int socket_desc , client_sock , c , read_size,rc;
struct sockaddr_in server , client;


int port_number;
char document_path[50];
char document_path_current_file[50];

typedef struct content_typ { 
    char name[50]; 
    char extension[50]; 
    } content_type;
content_type content_types[50];
int no_of_content_types;
int keep_alive_time;
int pid;

int main(int argc , char *argv[])
{
   //PARSE THE CONF FILE AND STORE THE INFO
   conf_file= fopen("../ws.conf","rb");
   int i=0;
   
   if(conf_file== 0)
   {
    printf("CANNOT OPEN CONF FILE\n\r");
    exit(-1);
   }
   else
   {
    printf("FILE OPENED\n");
    }
   char line[60],*pch;
    //PARSING THE WS.CONF FILE AND STORING ITS RESULTS
   while (1) 
   {
        if (fgets(line,300, conf_file) == NULL) 
            break;
        if(strstr(line,"Listen"))
        {
            pch=strtok(line," ");
            pch=strtok(NULL," ");
            port_number=atoi(pch);
            printf("%d is the port number\n",port_number);
            pch=strtok(NULL," ");
        }
        if(strstr(line,"DocumentRoot")) 
        {
            pch=strtok(line," ");
            pch=strtok(NULL," ");
            pch=strtok(pch,"\"");
            strcpy(document_path,pch);
            printf("%s is the document path\n",document_path);
        }
        if(line[0]=='.')   //For content types
        {
            // printf("%s\n",line);
            pch=strtok(line," ");
            strcpy(content_types[i].name,pch+1); // +1 so that the . does not get copied
            // content_type[i][]=pch;
            pch=strtok(NULL," ");
            // content_type[i][]=pch;
            strcpy(content_types[i].extension,pch);
            char *pos;
            if ((pos=strchr(content_types[i].extension, '\n')) != NULL)
                *pos = '\0';
            // printf("%s %s\n",content_types[i].name,content_types[i].extension);
            i++;
        }
        if(strstr(line,"Keep-Alive"))
        {
            pch=strtok(line," ");
            pch=strtok(NULL," ");
            if(strcmp(pch,"time")==0)
            {
                pch=strtok(NULL," ");
                keep_alive_time= atoi(pch);
                printf("Keep-Alive time is %d\n",keep_alive_time);
            }
        }
   }
   no_of_content_types=i;
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
    server.sin_port = htons( port_number );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 30);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);


    //ACCEPT LOOP 
while(1)
{ 
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    // printf("Connection accepted sock id %d\n",client_sock);
        
    if((pid=fork())==0)
    { // this is the child process
        close(socket_desc); // child doesn't need the listener
        char client_message[2000];
        char post_manipulation[2000];
        char file_contents[2000];
        char file_name[50];
        char file_type[50];
        char * nextLine,*pch;
        char * curLine = client_message;
        char file_content_type[25];
        char h1[40],h2[400],h3[40],h4[40],h5[40];
        char err_path[500];
        char msg_path[500];
        off_t offset = 0;          /* file offset */
        size_t lSize;
        char post_data[50];
        char post_path[50];
        int actual_size;
        FILE * pFile;
        FILE * pFile2;
        FILE * pFile3;
        char subs[100]; // this reads and parses every line of the request from webpage
        int fd;
        
        // printf("PID %d ENTERED\n",pid);
        server_length = sizeof(server);
            
        // struct timeval timeout={keep_alive_time,0}; //set timeout for a large value
        // setsockopt(client_sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
        nbytes = read(client_sock, client_message, sizeof(client_message));

        printf("RECEIVED MESSAGE is %s of size %d\n\n",client_message,nbytes);
        actual_size=nbytes;
        strncpy(post_manipulation,client_message,sizeof(client_message));
   i=0;
   while (curLine) 
   {    
        nextLine = strchr(curLine,'\r');

        if(i==0)
        {
                strncpy(subs,client_message,nextLine-client_message);
                // printf("CURRRRR is %s \n",subs);
        }
        
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line
        
        if(strstr(curLine,"GET"))
        {
            pch=strtok(curLine," ");
            pch=strtok(NULL," ");
            const char *dot = strrchr(pch, '.');
            if(!dot || dot == pch)  //HANDLING ONLY SENDING dIRectory case with no files. 
            {
                strcpy(document_path_current_file,document_path);
                strcat(document_path_current_file,"/index.html");
                strcpy(file_type,"html");
            }
            else
            {
                strcpy(file_type,dot+1);

                strcpy(document_path_current_file,document_path);
                strcat(document_path_current_file,pch);
               
                // printf("FILE TYPE is %s\n",file_type);
                printf("TRYING TO OPEN %s\n",document_path_current_file);
            }

            //FIRST CHECK FOR 501 ERROR, THEN 404 
            int i,conf_type_found=0;
            for(i=0;i<=no_of_content_types;i++)
            {
                if(strcmp(content_types[i].name,file_type)==0)
                {
                    conf_type_found=1;
                    strcpy(file_content_type,content_types[i].extension);
                }
            }
            if(conf_type_found==0)
            {
                strcpy(h1,"HTTP/1.1 501 Not Implemented\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>501 Not Implemented: Invalid File Type : %s\n</body>\n</html>",document_path_current_file);
                strcpy(err_path,document_path);
                strcat(err_path,"/501_error.html");
                fd= open (err_path,O_RDWR);    
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }   

                printf("501 Error\n");
                close(client_sock);
                exit(0);    
            }

            pFile = fopen(document_path_current_file,"rb");//www/graphics/arrowdown.gif
            if (pFile==NULL) 
            {
                    printf("404 Error\n");
                    strcpy(h1,"HTTP/1.1 404 Not Found\n\n");
                    if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                        {
                            printf("SENDING h1 TO SERVER FAILED\n");
                        }

                    sprintf(h2,"<html>\n<body>404 Not Found Reason URL does not exist : %s\n</body>\n</html>",document_path_current_file);

                    strcpy(err_path,document_path);
                    strcat(err_path,"/404_error.html");
                    fd= open (err_path,O_RDWR);
                    write(fd,h2,strlen(h2));
                    if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                        {
                            printf("SENDING FILE NAME TO SERVER FAILED\n");
                        }  
                    fclose(pFile); 
                    close(client_sock);
                    exit(0); 
            }
            fd= open (document_path_current_file,O_RDONLY);
            memset(document_path_current_file,0,sizeof(document_path_current_file));
            // obtain file size:
            fseek (pFile , 0 , SEEK_END);
            lSize = ftell (pFile);
            fseek(pFile,0,SEEK_SET);
            fread(file_contents,lSize,lSize,pFile);

        }
        //POST COMMAND
        if(strstr(client_message,"POST"))
        {
            printf("POST COMMAND\n");
            int ij=actual_size-1;
            int some=0;
            while(client_message[ij-1]!='\r')
            {
                // if((client_message[ij-1]))
                char tempcahr[1];
                tempcahr[1]=client_message[ij];
                post_data[some]=tempcahr[1];
                // printf("psott %c\n",tempcahr[1]);
                some++;
                ij--;
            }
            post_data[some]='\0';
            printf("POST DATA %s\n",post_data);

            char swap[50];

            for(ij=0;ij<=strlen(post_data);ij++)
            {
                swap[ij]=post_data[strlen(post_data)-ij-1];
                // printf("w%c ",swap[ij]);
            }
            swap[(strlen(post_data))+1]='\0';
            strcpy(post_data,swap);
            char temp_index[50];
            strcpy(temp_index,document_path);
            strcat(temp_index,"/index.html");

            printf("POST DATA is %s\n",swap);
            sprintf(h1,"<html><body><pre><h1>%s</h1></pre>",post_data);
            strcpy(post_path,document_path);
            strcat(post_path,"/post.html");

            pFile2 = fopen(post_path, "wa");
            pFile3 = fopen(temp_index,"rb");
            
            fwrite(h1,sizeof(char), strlen(h1), pFile2);

            char byte;

            while (!feof(pFile3)) 
            {
                fread(&byte, sizeof(char), 1, pFile3);
                fwrite(&byte, sizeof(char), 1, pFile2);
            }

            fseek (pFile2 , 0 , SEEK_END);
            lSize = ftell (pFile2);
            fseek(pFile2,0,SEEK_SET);

            fd= open (post_path,O_RDWR);
            if((nbytes=sendfile(client_sock, fd,&offset, lSize)) == -1)
            {
                printf("SENDING FILE NAME TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                fclose(pFile2); 
                fclose(pFile3); 

                nbytes=remove(post_path);
                if(nbytes==0)
                    printf("FILE DELETION SUCC\n");

                close(client_sock);
                exit(0); 
            }
            
        printf("SENDING FILE DONE!\n");
        fclose(pFile3);
        fclose(pFile2);
        close(client_sock);
        exit(0);
        }
        //FOr 400 error method is not GET or POST
        if( (i==0) && (!(strstr(curLine,"POST"))) && (!(strstr(curLine,"GET"))) )
        {
            printf("400 Error Invalid Method\n");
            pch=strtok(curLine," ");
            strcpy(h1,"HTTP/1.1 400 Bad Request\n\n");
            if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                {
                    printf("SENDING h1 TO SERVER FAILED\n");
                }

            sprintf(h2,"<html>\n<body>400 Bad Request Reason: Invalid Method :%s\n</body>\n</html>",pch);

            strcpy(err_path,document_path);
            strcat(err_path,"/404_error.html");
            fd= open (err_path,O_RDWR);
            write(fd,h2,strlen(h2));
            if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                {
                    printf("SENDING FILE NAME TO SERVER FAILED\n");
                }  
            fclose(pFile); 
            close(client_sock);
            exit(0); 
        }
        //FOr 400 error HTTP Version is invalid
        if( (i==0) && (!(strstr(subs,"HTTP/1.1"))) && (!(strstr(subs,"HTTP/1.0"))) )
        {
            printf("400 Error Invalid Version\n");
            pch=strtok(subs," ");
            pch=strtok(NULL," ");
            pch=strtok(NULL," ");
            // printf("PCHHC %s\n",pch);
            strcpy(h1,"HTTP/1.1 400 Bad Request\n\n");
            if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                {
                    printf("SENDING h1 TO SERVER FAILED\n");
                }

            sprintf(h2,"<html>\n<body>400 Bad Request Reason: Invalid HTTP-Version:%s\n</body>\n</html>",pch);

            strcpy(err_path,document_path);
            strcat(err_path,"/404_error.html");
            fd= open (err_path,O_RDWR);
            write(fd,h2,strlen(h2));
            if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                {
                    printf("SENDING FILE NAME TO SERVER FAILED\n");
                }  
            fclose(pFile); 
            close(client_sock);
            exit(0); 
        }


        //For match in Keep alive option
        if(strstr(curLine,"Connection:"))
        {
            printf("KEEEEEEP is %s\n",curLine);
            struct timeval timeout={keep_alive_time,0}; //set timeout for a large value
            setsockopt(client_sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
          
        }

        curLine = nextLine ? (nextLine+1) : NULL;
        i++;
   }
        strcpy(h1,"HTTP/1.1 200 OK\n");

        sprintf(h2,"Content-Type: %s\n",file_content_type);
        
        sprintf(h3,"Content-Length: %ld\n",lSize);
        
        strcpy(h4,"Connection: Keep-Alive\n");

        sprintf(h5,"Keep-alive: timeout=%d\n\n",keep_alive_time);

        if(nbytes=write(client_sock, h1, strlen(h1) == -1))
            {
                printf("SENDING h1 TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            } 
        if(nbytes=write(client_sock, h2, strlen(h2) == -1))
            {
                printf("SENDING h2 TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            }
        if(nbytes=write(client_sock, h3, strlen(h3) == -1))
            {
                printf("SENDING h3 TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            }
        if(nbytes=write(client_sock, h4, strlen(h4)== -1))
            {
                printf("SENDING h4 TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            }
        if(nbytes=write(client_sock, h5, strlen(h5)== -1))
            {
                printf("SENDING h5 TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            }

        //SEND COMMAND NAME TO SERVER
        if((nbytes=sendfile(client_sock, fd,&offset, lSize)) == -1)
            {
                printf("SENDING FILE NAME TO SERVER FAILED\n");
                strcpy(h1,"HTTP/1.1 500 Internal Server Error\n\n");
                if((nbytes=write(client_sock, h1, strlen(h1)== -1)))
                    {
                        printf("SENDING h1 TO SERVER FAILED\n");
                    }

                sprintf(h2,"<html>\n<body>500 Internal Server Error : cannot allocate memory\n</body>\n</html>");

                strcpy(err_path,document_path);
                strcat(err_path,"/500_error.html");
                fd= open (err_path,O_RDWR);
                write(fd,h2,strlen(h2));
                if((nbytes=sendfile(client_sock, fd,&offset, strlen(h2))) == -1)
                    {
                        printf("SENDING FILE NAME TO SERVER FAILED\n");
                    }  
                fclose(pFile); 
                close(client_sock);
                exit(0); 
            }
            
        printf("SENDING FILE DONE!\n");
        fclose(pFile);
        close(client_sock);
        exit(0);    
    }
    else if(pid>0)
    {
        close(client_sock);
    }
    else
    {
        printf("FORK FAILED\n");
    }
    }
    close(socket_desc);
    close(client_sock);
    return 0;
}

