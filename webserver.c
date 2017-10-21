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
            printf("%s %s\n",content_types[i].name,content_types[i].extension);
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
    puts("Socket created");
     
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
    listen(socket_desc , 15);
     
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
        printf("Connection accepted\n");
        
    if((pid=fork())==0)
    { // this is the child process
        close(socket_desc); // child doesn't need the listener
        char client_message[2000];
        char file_contents[2000];
        char file_name[50];
        char file_type[50];
        char * nextLine,*pch;
        char * curLine = client_message;
        char file_content_type[25];
        char h1[40],h2[40],h3[40],h4[40],h5[40];
        off_t offset = 0;          /* file offset */
        size_t lSize;
        FILE * pFile;
        int fd;
        
        // printf("PID %d ENTERED\n",pid);
        server_length = sizeof(server);
        nbytes = read(client_sock, client_message, sizeof(client_message));
        //Send the message back to client
        printf("RECEIVED MESSAGE %s\n\n\n",client_message);

       while (curLine) 
       {
          nextLine = strchr(curLine, '\n');
          if (nextLine) *nextLine = '\0';  // temporarily terminate the current line
          
        if(strstr(curLine,"GET"))
        {
            pch=strtok(curLine," ");
            pch=strtok(NULL," ");

            strcpy(document_path_current_file,document_path);
            strcat(document_path_current_file,pch);
            printf("TRYING TO OPEN %s\n",document_path_current_file);

            pFile = fopen(document_path_current_file,"rb");//www/graphics/arrowdown.gif
            if (pFile==NULL) 
            {
                printf("FILE IS NOT PRESENT\n");
                // strcpy(h1,"HTTP/1.1 404 Not Found\n");
                // if((nbytes=sendto(client_sock, h1, sizeof(h1), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
                // {
                //     printf("SENDING h1 TO SERVER FAILED\n");
                // }
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
            // printf("FILE SIZE IS %ld\n",lSize);


            const char *dot = strrchr(pch, '.');
            if(!dot || dot == pch)
                {}
            strcpy(file_type,dot+1);

            printf("FILE TYPE is %s\n",file_type);
            int i;
            for(i=0;i<=no_of_content_types;i++)
            {
                if(strcmp(content_types[i].name,file_type)==0)
                {
                    strcpy(file_content_type,content_types[i].extension);
                }
            }
        }
          curLine = nextLine ? (nextLine+1) : NULL;
       }

        strcpy(h1,"HTTP/1.1 200 OK\n\n");
        sprintf(h2,"Content-Type: %s\n",file_content_type);
        
        sprintf(h3,"Content-Length: %ld\n\n",lSize);
        
        strcpy(h4,"Connection: Keep-alive\n\n");

        strcpy(h5,"Keep-Alive: timeout=15, max=100\n\n");

        // printf("SENDING MESSSAGE\n\n%s%s%s%s\n",h1,h2,h3,h4);
        // if((nbytes=sendto(client_sock, h1, sizeof(h1), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
        //     {
        //         printf("SENDING h1 TO SERVER FAILED\n");
        //     }
        // if((nbytes=sendto(client_sock, h2, sizeof(h2), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
        //     {
        //         printf("SENDING h2 TO SERVER FAILED\n");
        //     }
        // if((nbytes=sendto(client_sock, h3, sizeof(h3), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
        //     {
        //         printf("SENDING h3 TO SERVER FAILED\n");
        //     }
        // if((nbytes=sendto(client_sock, h4, sizeof(h4), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
        //     {
        //         printf("SENDING h4 TO SERVER FAILED\n");
        //     }
        // if((nbytes=sendto(client_sock, h5, sizeof(h4), 0, (struct sockaddr*) &server, sizeof(server))) == -1)
        //     {
        //         printf("SENDING h4 TO SERVER FAILED\n");
        //     }

        if((nbytes=write(client_sock, "HTTP/1.1 200 OK\n\n", sizeof("HTTP/1.1 200 OK\n\n")== -1)))
            {
                printf("SENDING h1 TO SERVER FAILED\n");
            }
        if(nbytes=write(client_sock, h2, strlen(h2) == -1))
            {
                printf("SENDING h2 TO SERVER FAILED\n");
            }
        if(nbytes=write(client_sock, h3, strlen(h3) == -1))
            {
                printf("SENDING h3 TO SERVER FAILED\n");
            }
        if(nbytes=write(client_sock, h4, strlen(h4)== -1))
            {
                printf("SENDING h4 TO SERVER FAILED\n");
            }

        //SEND COMMAND NAME TO SERVER
        if((nbytes=sendfile(client_sock, fd,&offset, lSize)) == -1)
            {
                printf("SENDING FILE NAME TO SERVER FAILED\n");
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
    // close(client_sock);
    // close(socket_desc);	
    return 0;
}

