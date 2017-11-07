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
#include <openssl/md5.h>

int socket_desc1,socket_desc2,socket_desc3,socket_desc4,c;
struct sockaddr_in client1,client2,client3,client4;
long chunk1_size,chunk2_size,chunk3_size,chunk4_size;
char *dfs1chunk1,*dfs1chunk2,*dfs2chunk1,*dfs2chunk2,*dfs3chunk1,*dfs3chunk2,*dfs4chunk1,*dfs4chunk2;
long dfs1chunk1_size,dfs1chunk2_size,dfs2chunk1_size,dfs2chunk2_size,dfs3chunk1_size,dfs3chunk2_size,dfs4chunk1_size,dfs4chunk2_size;
FILE * conf_file;
FILE * fp;
int port_number1,port_number2,port_number3,port_number4;
char username[60],password[60];
char dfc_path[50];
char user_cmd[50];
char dfs_numbers1[10],dfs_numbers2[10],dfs_numbers3[10],dfs_numbers4[10];//Sending numbers to DFS in PUT

typedef struct dfsfile_names { 
    int dfsfilecount;
    char dirfilename[10][50]; 
    } dfs_filename;
dfs_filename dfsfilename[4];

typedef struct listfiles { 
    int parts[4];
    char filenamewoextension[50]; 
    } list_file;


int main(int argc,char**argv)
{

  if(argv[1]==NULL)
   {
    printf("Please enter a path for dfc.conf file\n");
    exit(-1);
   }
   printf("DFC path is %s\n",argv[1]);

   strcpy(dfc_path,argv[1]);
  //PARSE THE CONF FILE AND STORE THE INFO
   conf_file= fopen(dfc_path,"rb");
   
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
        if(strstr(line,"Server"))
        {
            if(strstr(line,"DFS1"))
            {
              printf("DFS1 address is");
              pch=strtok(line,":");
              pch=strtok(NULL," ");
              port_number1=atoi(pch);
              printf(" %d\n",port_number1);
            }
            if(strstr(line,"DFS2"))
            {
              printf("DFS2 address is");
              pch=strtok(line,":");
              pch=strtok(NULL," ");
              port_number2=atoi(pch);
              printf(" %d\n",port_number2);
            }
            if(strstr(line,"DFS3"))
            {
              printf("DFS3 address is");
              pch=strtok(line,":");
              pch=strtok(NULL," ");
              port_number3=atoi(pch);
              printf(" %d\n",port_number3);
            }
            if(strstr(line,"DFS4"))
            {
              printf("DFS4 address is");
              pch=strtok(line,":");
              pch=strtok(NULL," ");
              port_number4=atoi(pch);
              printf(" %d\n",port_number4);
            }
        }
        if(strstr(line,"Username"))
        {
          printf("Username is");
          pch=strtok(line,":");
          pch=strtok(NULL," ");
          strcpy(username,pch);
          printf(" %s",username);
        }
        if(strstr(line,"Password"))
        {
          printf("Password is");
          pch=strtok(line,":");
          pch=strtok(NULL," ");
          strcpy(password,pch);
          printf(" %s\n",password);
        }
   }
  off_t offset=0;
  int nbytes;
  char buffer[10];
  printf("START CLIENT");
  socket_desc1 = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc1 == -1)
    {
        printf("Could not create socket");
    }
  socket_desc2 = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc2 == -1)
    {
        printf("Could not create socket");
    }
  socket_desc3 = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc3 == -1)
    {
        printf("Could not create socket");
    }
  socket_desc4 = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc4 == -1)
    {
        printf("Could not create socket");
    }
    puts("\n\nSocket created");
     
    //Prepare the sockaddr_in structure
    client1.sin_family = AF_INET;
    client1.sin_port = htons( port_number1 );

    client2.sin_family = AF_INET;
    client2.sin_port = htons( port_number2 );

    client3.sin_family = AF_INET;
    client3.sin_port = htons( port_number3 );

    client4.sin_family = AF_INET;
    client4.sin_port = htons( port_number4 );
    
    if( connect(socket_desc1, (struct sockaddr *)&client1, sizeof(client1)) < 0)
    {
       printf("\n Error : Connection to Socket 1 Failed \n");
       // return 1;
    } 

    if( connect(socket_desc2, (struct sockaddr *)&client2, sizeof(client2)) < 0)
    {
       printf("\n Error : Connection to Socket 2 Failed \n");
       // return 1;
    } 

    if( connect(socket_desc3, (struct sockaddr *)&client3, sizeof(client3)) < 0)
    {
       printf("\n Error : Connection to Socket 3 Failed \n");
       // return 1;
    }

    if( connect(socket_desc4, (struct sockaddr *)&client4, sizeof(client4)) < 0)
    {
       printf("\n Error : Connection to Socket 4 Failed \n");
       // return 1;
    }
  
  while(1)
  { 
    
    printf("ENTER one of the following commands\nLIST\nPUT <file_name>\nGET <file_name>\n\r");
     /* Get the name, with size limit. */
    fgets (user_cmd, sizeof(user_cmd), stdin);

    //SENDING TO DFS1,2,3,4
     //SEND USERNAME AND PASSWORD FIRST
      if((nbytes=send(socket_desc1, username, sizeof(username),0)== -1))
      {
          printf("SENDING username TO DFS1 FAILED\n");
      }
      if((nbytes =  recv(socket_desc1,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }

      if((nbytes=send(socket_desc1, password, sizeof(password),0)== -1))
      {
          printf("SENDING password TO DFS1 FAILED\n");
      }
      if((nbytes=recv(socket_desc1,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      } 
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }
      //DFS2
      if((nbytes=send(socket_desc2, username, sizeof(username),0)== -1))
      {
          printf("SENDING username TO DFS2 FAILED\n");
      }
      if((nbytes =  recv(socket_desc2,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }

      if((nbytes=send(socket_desc2, password, sizeof(password),0)== -1))
      {
          printf("SENDING password TO DFS2 FAILED\n");
      }
      if((nbytes=recv(socket_desc2,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      } 
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }
      //DFS3
      if((nbytes=send(socket_desc3, username, sizeof(username),0)== -1))
      {
          printf("SENDING username TO DFS3 FAILED\n");
      }
      if((nbytes =  recv(socket_desc3,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }

      if((nbytes=send(socket_desc3, password, sizeof(password),0)== -1))
      {
          printf("SENDING password TO DFS3 FAILED\n");
      }
      if((nbytes=recv(socket_desc3,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      } 
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }
      //DFS4
      if((nbytes=send(socket_desc4, username, sizeof(username),0)== -1))
      {
          printf("SENDING username TO DFS1 FAILED\n");
      }
      if((nbytes =  recv(socket_desc4,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }

      if((nbytes=send(socket_desc4, password, sizeof(password),0)== -1))
      {
          printf("SENDING password TO DFS4 FAILED\n");
      }
      if((nbytes=recv(socket_desc4,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      } 
      if(strcmp(buffer,"ACK")!=0)
      {
        printf("ACK for username/password not received\n");
        continue;
      }

    pch=strtok(user_cmd," ");
    char *pos;
    if ((pos=strchr(pch, '\n')) != NULL)
    *pos = '\0';
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

      if(strcmp(pch,"PUT")==0)
      {
        char file_name[50];
        pch =strtok(NULL," ");
        strcpy(file_name,pch);

        if(file_name==NULL)
        {
          printf("ERROR: File name not entered\n");
          continue;
        }
          printf("FILE NAME is %s\n",file_name);

        char *pos;
        if ((pos=strchr(file_name, '\n')) != NULL)
            *pos = '\0';
        fp=fopen(file_name,"rb");
        if(fp==0)
         {
          printf("CANNOT OPEN FILE %s\n",file_name);
          continue;
         }
        int fd=open(file_name,O_RDONLY);
        fseek (fp , 0 , SEEK_END);
        long fSize = ftell (fp);
        fseek(fp,0,SEEK_SET);

        //Assign chunk sizes based on file size
        chunk1_size=fSize/4;
        chunk2_size=fSize/4;
        chunk3_size=fSize/4;
        chunk4_size=fSize-(chunk1_size+chunk2_size+chunk3_size);
        char *md5filecontents = (char*) malloc(fSize + 1);
        char *file = (char*) malloc(fSize + 1);
        fread(file, fSize, 1, fp);

        int bytes,i;
        unsigned char md5sum[MD5_DIGEST_LENGTH];
        MD5_CTX mdContext;
        MD5_Init (&mdContext);
        while ( (bytes = fread (md5filecontents, 1, sizeof(md5filecontents), fp)) != 0)
        MD5_Update (&mdContext, md5filecontents, bytes);
        MD5_Final (md5sum,&mdContext);
        printf("MD5value is ");
        for(i = MD5_DIGEST_LENGTH-1; i >= 0; i--) 
          {
            printf("%x", md5sum[i]);
          }
          printf("\n");
       int split_value=md5sum[MD5_DIGEST_LENGTH-1]%4;
       printf("%xsplit is\n",split_value);
       
        char *chunk1 = (char*) malloc(chunk1_size );
        char *chunk2 = (char*) malloc(chunk2_size );
        char *chunk3 = (char*) malloc(chunk3_size );
        char *chunk4 = (char*) malloc(chunk4_size );

        chunk1=file;
        chunk2=file+chunk1_size;
        chunk3=file+chunk1_size+chunk2_size;
        chunk4=file+chunk1_size+chunk2_size+chunk3_size;
        
        if(split_value==0)
       {
          //DFS1-1,2 ;DFS2-2,3 ;DFS3-3,4; DFS4-4,1
          dfs1chunk1=chunk1;
          dfs1chunk2=chunk2;
          strcpy(dfs_numbers1,"1 2");

          dfs2chunk1=chunk2;
          dfs2chunk2=chunk3;
          strcpy(dfs_numbers2,"2 3");

          dfs3chunk1=chunk3;
          dfs3chunk2=chunk4;
          strcpy(dfs_numbers3,"3 4");

          dfs4chunk1=chunk4;
          dfs4chunk2=chunk1;
          strcpy(dfs_numbers4,"4 1");

          dfs1chunk1_size=chunk1_size;
          dfs1chunk2_size=chunk2_size;
          dfs2chunk1_size=chunk2_size;
          dfs2chunk2_size=chunk3_size;
          dfs3chunk1_size=chunk3_size;
          dfs3chunk2_size=chunk4_size;
          dfs4chunk1_size=chunk4_size;
          dfs4chunk2_size=chunk1_size;
       }
       else if(split_value==1)
       {
          //DFS1-4,1 ;DFS2-1,2 ;DFS3-2,3; DFS4-3,4
          dfs1chunk1=chunk4;
          dfs1chunk2=chunk1;
          strcpy(dfs_numbers1,"4 1");

          dfs2chunk1=chunk1;
          dfs2chunk2=chunk2;
          strcpy(dfs_numbers2,"1 2");

          dfs3chunk1=chunk2;
          dfs3chunk2=chunk3;
          strcpy(dfs_numbers3,"2 3");

          dfs4chunk1=chunk3;
          dfs4chunk2=chunk4;
          strcpy(dfs_numbers4,"3 4");

          dfs1chunk1_size=chunk4_size;
          dfs1chunk2_size=chunk1_size;
          dfs2chunk1_size=chunk1_size;
          dfs2chunk2_size=chunk2_size;
          dfs3chunk1_size=chunk2_size;
          dfs3chunk2_size=chunk3_size;
          dfs4chunk1_size=chunk3_size;
          dfs4chunk2_size=chunk4_size;
       }
       else if(split_value==2)
       {
          //DFS1-3,4 ;DFS2-4,1 ;DFS3-1,2; DFS4-2,3
          dfs1chunk1=chunk3;
          dfs1chunk2=chunk4;
          strcpy(dfs_numbers1,"3 4");

          dfs2chunk1=chunk4;
          dfs2chunk2=chunk1;
          strcpy(dfs_numbers2,"4 1");

          dfs3chunk1=chunk1;
          dfs3chunk2=chunk2;
          strcpy(dfs_numbers3,"1 2");

          dfs4chunk1=chunk2;
          dfs4chunk2=chunk3;
          strcpy(dfs_numbers4,"2 3");

          dfs1chunk1_size=chunk3_size;
          dfs1chunk2_size=chunk4_size;
          dfs2chunk1_size=chunk4_size;
          dfs2chunk2_size=chunk1_size;
          dfs3chunk1_size=chunk1_size;
          dfs3chunk2_size=chunk2_size;
          dfs4chunk1_size=chunk2_size;
          dfs4chunk2_size=chunk3_size;
       }
       else if(split_value==3)
       {
          //DFS1-2,3 ;DFS2-3,4 ;DFS3-4,1; DFS4-1,2
          dfs1chunk1=chunk2;
          dfs1chunk2=chunk3;
          strcpy(dfs_numbers1,"2 3");

          dfs2chunk1=chunk3;
          dfs2chunk2=chunk4;
          strcpy(dfs_numbers2,"3 4");

          dfs3chunk1=chunk4;
          dfs3chunk2=chunk1;
          strcpy(dfs_numbers3,"4 1");

          dfs4chunk1=chunk1;
          dfs4chunk2=chunk2;
          strcpy(dfs_numbers4,"1 2");

          dfs1chunk1_size=chunk2_size;
          dfs1chunk2_size=chunk3_size;
          dfs2chunk1_size=chunk3_size;
          dfs2chunk2_size=chunk4_size;
          dfs3chunk1_size=chunk4_size;
          dfs3chunk2_size=chunk1_size;
          dfs4chunk1_size=chunk1_size;
          dfs4chunk2_size=chunk2_size;
       }

        //SEND COMMAND name and receive ACK
        //DFS1
        pch=strtok(user_cmd," ");
        if((nbytes=send(socket_desc1, pch, strlen(pch),0)== -1))
        {
            printf("SENDING COMMAND TO DFS1 FAILED\n");
        }
        if((nbytes =  recv(socket_desc1,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS2
        pch=strtok(user_cmd," ");
        if((nbytes=send(socket_desc2, pch, strlen(pch),0)== -1))
        {
            printf("SENDING COMMAND TO DFS2 FAILED\n");
        }
        if((nbytes =  recv(socket_desc2,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS3
        pch=strtok(user_cmd," ");
        if((nbytes=send(socket_desc3, pch, strlen(pch),0)== -1))
        {
            printf("SENDING COMMAND TO DFS3 FAILED\n");
        }
        if((nbytes =  recv(socket_desc3,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS4
        pch=strtok(user_cmd," ");
        if((nbytes=send(socket_desc4, pch, strlen(pch),0)== -1))
        {
            printf("SENDING COMMAND TO DFS4 FAILED\n");
        }
        if((nbytes =  recv(socket_desc4,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }

        //SEND FILE CHUNK NUMBERS FIRST
        //DFS1
        if((nbytes=send(socket_desc1, dfs_numbers1, sizeof(dfs_numbers1),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS1 FAILED\n");
        }
        if((nbytes =  recv(socket_desc1 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS2
        if((nbytes=send(socket_desc2, dfs_numbers2, sizeof(dfs_numbers2),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS2 FAILED\n");
        }
        if((nbytes =  recv(socket_desc2 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS3
        if((nbytes=send(socket_desc3, dfs_numbers3, sizeof(dfs_numbers3),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS3 FAILED\n");
        }
        if((nbytes =  recv(socket_desc3 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS4
        if((nbytes=send(socket_desc4, dfs_numbers4, sizeof(dfs_numbers4),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS4 FAILED\n");
        }
        if((nbytes =  recv(socket_desc4 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }

        //SEND FILE NAME 
        printf("FILE ANEMN is %s\n",file_name);
        //DFS1
        if((nbytes=send(socket_desc1, file_name, sizeof(file_name),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS1 FAILED\n");
        }
        if((nbytes =  recv(socket_desc1 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS2
        if((nbytes=send(socket_desc2, file_name, sizeof(file_name),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS2 FAILED\n");
        }
        if((nbytes =  recv(socket_desc2 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS3
        if((nbytes=send(socket_desc3, file_name, sizeof(file_name),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS3 FAILED\n");
        }
        if((nbytes =  recv(socket_desc3 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        //DFS4
        if((nbytes=send(socket_desc4, file_name, sizeof(file_name),0)== -1))
        {
            printf("SENDING dfs_numbers TO DFS4 FAILED\n");
        }
        if((nbytes =  recv(socket_desc4 ,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }


        //SEND CHUNK SIZE FIRST, then the contents
        //DFS1
        sprintf(buffer,"%ld",dfs1chunk1_size);
        if((nbytes=send(socket_desc1, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS1 FAILED\n");
            }
        if((nbytes=send(socket_desc1, dfs1chunk1, dfs1chunk1_size,0)== -1))
            {
                printf("SENDING file TO DFS1 FAILED\n");
            }
        printf("Chunk 1 sent successfully to DFS1 \n");


        sprintf(buffer,"%ld",dfs1chunk2_size);
        if((nbytes=send(socket_desc1, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS1 FAILED\n");
            }
        if((nbytes=send(socket_desc1, dfs1chunk2, dfs1chunk2_size,0)== -1))
            {
                printf("SENDING file TO DFS1 FAILED\n");
            }
        printf("Chunk 2 sent successfully to DFS1 \n");

        //DFS2
        sprintf(buffer,"%ld",dfs2chunk1_size);
        if((nbytes=send(socket_desc2, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS2 FAILED\n");
            }
        if((nbytes=send(socket_desc2, dfs2chunk1, dfs2chunk1_size,0)== -1))
            {
                printf("SENDING file TO DFS2 FAILED\n");
            }
        printf("Chunk 1 sent successfully to DFS2 \n");


        sprintf(buffer,"%ld",dfs2chunk2_size);
        if((nbytes=send(socket_desc2, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS2 FAILED\n");
            }
        if((nbytes=send(socket_desc2, dfs2chunk2, dfs2chunk2_size,0)== -1))
            {
                printf("SENDING file TO DFS2 FAILED\n");
            }
        printf("Chunk 2 sent successfully to DFS2 \n");

        //DFS3
        sprintf(buffer,"%ld",dfs3chunk1_size);
        if((nbytes=send(socket_desc3, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS3 FAILED\n");
            }
        if((nbytes=send(socket_desc3, dfs3chunk1, dfs3chunk1_size,0)== -1))
            {
                printf("SENDING file TO DFS3 FAILED\n");
            }
        printf("Chunk 1 sent successfully to DFS3 \n");


        sprintf(buffer,"%ld",dfs3chunk2_size);
        if((nbytes=send(socket_desc3, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS3 FAILED\n");
            }
        if((nbytes=send(socket_desc3, dfs3chunk2, dfs3chunk2_size,0)== -1))
            {
                printf("SENDING file TO DFS3 FAILED\n");
            }
        printf("Chunk 2 sent successfully to DFS3 \n");

        //DFS4
        sprintf(buffer,"%ld",dfs4chunk1_size);
        if((nbytes=send(socket_desc4, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS4 FAILED\n");
            }
        if((nbytes=send(socket_desc4, dfs4chunk1, dfs4chunk1_size,0)== -1))
            {
                printf("SENDING file TO DFS4 FAILED\n");
            }
        printf("Chunk 1 sent successfully to DFS4 \n");


        sprintf(buffer,"%ld",dfs4chunk2_size);
        if((nbytes=send(socket_desc4, buffer, sizeof(long),0)== -1))
            {
                printf("SENDING file TO DFS4 FAILED\n");
            }
        if((nbytes=send(socket_desc4, dfs4chunk2, dfs4chunk2_size,0)== -1))
            {
                printf("SENDING file TO DFS4 FAILED\n");
            }
        printf("Chunk 2 sent successfully to DFS4 \n");
        chunk1=NULL;
        chunk2=NULL;
        chunk3=NULL;
        chunk4=NULL;
        if((nbytes =  recv(socket_desc1,buffer,60,0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }
        printf("Recevied final ACK\n");
        fclose(fp);
      }
    else if(strcmp(pch,"GET")==0)
    {

    }
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

    else if(strcmp(pch,"LIST")==0)
    {
      printf("list command\n");
      //SEND COMMAND name and receive ACK
      //DFS1
      pch=strtok(user_cmd," ");
      if ((pos=strchr(pch, '\n')) != NULL)
      *pos = '\0';
      if((nbytes=send(socket_desc1, pch, strlen(pch),0)== -1))
      {
          printf("SENDING COMMAND TO DFS1 FAILED\n");
      }
      if((nbytes =  recv(socket_desc1,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      //DFS2
      pch=strtok(user_cmd," ");
      if((nbytes=send(socket_desc2, pch, strlen(pch),0)== -1))
      {
          printf("SENDING COMMAND TO DFS2 FAILED\n");
      }
      if((nbytes =  recv(socket_desc2,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      //DFS3
      pch=strtok(user_cmd," ");
      if((nbytes=send(socket_desc3, pch, strlen(pch),0)== -1))
      {
          printf("SENDING COMMAND TO DFS3 FAILED\n");
      }
      if((nbytes =  recv(socket_desc3,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      //DFS4
      pch=strtok(user_cmd," ");
      if((nbytes=send(socket_desc4, pch, strlen(pch),0)== -1))
      {
          printf("SENDING COMMAND TO DFS4 FAILED\n");
      }
      if((nbytes =  recv(socket_desc4,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      int filecount;
      int totalfilecount=0;

      //RECEIVE NUMBER OF FILES ABOUT TO BE SENT FROM DFS AND FILE NAMES
      //DFS1
      if((nbytes =  recv(socket_desc1,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      filecount=atoi(buffer);
      printf("FILECOUNT reveied is %d\n",filecount);
      // printf("FILECOUNT reveied is %s\n",buffer);
      totalfilecount=totalfilecount+filecount;
      dfsfilename[0].dfsfilecount=filecount;
      for(int i=0;i<filecount;i++)
      {
        if((nbytes =  recv(socket_desc1,dfsfilename[0].dirfilename[i],sizeof(dfsfilename[0].dirfilename[filecount]),0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }  
      }
      //DFS2
      if((nbytes =  recv(socket_desc2,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      filecount=atoi(buffer);
      printf("FILECOUNT reveied is %d\n",filecount);
      // printf("FILECOUNT reveied is %s\n",buffer);
      totalfilecount=totalfilecount+filecount;
      dfsfilename[1].dfsfilecount=filecount;
      for(int i=0;i<filecount;i++)
      {
        if((nbytes =  recv(socket_desc2,dfsfilename[1].dirfilename[i],sizeof(dfsfilename[1].dirfilename[filecount]),0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }  
      }
      //DFS3
      if((nbytes =  recv(socket_desc3,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      filecount=atoi(buffer);
      printf("FILECOUNT reveied is %d\n",filecount);
      // printf("FILECOUNT reveied is %s\n",buffer);
      totalfilecount=totalfilecount+filecount;
      dfsfilename[2].dfsfilecount=filecount;
      for(int i=0;i<filecount;i++)
      {
        if((nbytes =  recv(socket_desc3,dfsfilename[2].dirfilename[i],sizeof(dfsfilename[2].dirfilename[filecount]),0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }  
      }
      //DFS4
      if((nbytes =  recv(socket_desc4,buffer,60,0) ) < 0)
      {
          printf("ERROR in receiving ACK recv\n");
          break;
      }
      filecount=atoi(buffer);
      printf("FILECOUNT reveied is %d\n",filecount);
      // printf("FILECOUNT reveied is %s\n",buffer);
      totalfilecount=totalfilecount+filecount;
      dfsfilename[3].dfsfilecount=filecount;
      for(int i=0;i<filecount;i++)
      {
        if((nbytes =  recv(socket_desc4,dfsfilename[3].dirfilename[i],sizeof(dfsfilename[3].dirfilename[filecount]),0) ) < 0)
        {
            printf("ERROR in receiving ACK recv\n");
            break;
        }  
      }        
  // typedef struct listfiles { 
  //   int parts[4];
  //   char filenamewoextension[50]; 
  //   } list_file;
      list_file *listfile;
      listfile = (list_file*)malloc((totalfilecount*sizeof(list_file)));
      int temptotalfilecount=totalfilecount;
      int validfilenumber=0;
      char *strippedfilename;
      int strippednumber;
      char *reversefilename;
     //FORM FILES FROM RECEIVED DATA AND DISPLAY
      for(int i=0;i<4;i++)//PARSE THRU 4 DFS
      {
        for(int dfscount=0;dfscount<dfsfilename[i].dfsfilecount;dfscount++)//parse through all files of a single DFS
        {
          //remove extension from filename and store in strippedfilename and stripped number
          printf("Recevied file is %s\n",dfsfilename[i].dirfilename[dfscount]);

          // //Do strcmp(filenamewoextension,strippedfilename)
          // for(int i=0;i<totalfilecount;i++)
          // {
          //   if(strcmp((listfile[i].filenamewoextension),strippedfilename)==0)
          //   {
          //     listfile[i].parts[strippednumber]=7;//7 means set
          //     validfilenumber++;
          //     continue;
          //   }
          // }
          // printf("File name not found in listfile\n");
          // strcpy((listfile[validfilenumber].filenamewoextension),strippedfilename);
        }
      }

    }
    else
    {
      printf("Enter a valid command\n");
    }
    // close(socket_desc1);
    // close(socket_desc2);
    // close(socket_desc3);
    // close(socket_desc4);
  }
    return 0;
}