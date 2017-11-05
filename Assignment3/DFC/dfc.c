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
int port_number1,port_number2,port_number3,port_number4;
char username[60],password[60];
char dfc_path[50];

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
     
    // while(1)
    // { 
      
      if( connect(socket_desc1, (struct sockaddr *)&client1, sizeof(client1)) < 0)
      {
         printf("\n Error : Connect Failed \n");
         return 1;
      } 

      if( connect(socket_desc2, (struct sockaddr *)&client2, sizeof(client2)) < 0)
      {
         printf("\n Error : Connect Failed \n");
         return 1;
      } 


      // if( connect(socket_desc3, (struct sockaddr *)&client3, sizeof(client3)) < 0)
      // {
      //    printf("\n Error : Connect Failed \n");
      //    return 1;
      // } 

      // if( connect(socket_desc4, (struct sockaddr *)&client4, sizeof(client4)) < 0)
      // {
      //    printf("\n Error : Connect Failed \n");
      //    return 1;
      // } 
      
      FILE *fp;
      int fd=open("../dfc.conf",O_RDONLY);
      fp=fopen("../dfc.conf","rb");
      // fscanf(fp,"%s",buffer);
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
     
    //   else if(split_value==1)

    // else if (split_value==2)


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

        dfs2chunk1=chunk2;
        dfs2chunk2=chunk3;

        dfs3chunk1=chunk3;
        dfs3chunk2=chunk4;

        dfs4chunk1=chunk4;
        dfs4chunk2=chunk1;

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

        dfs2chunk1=chunk1;
        dfs2chunk2=chunk2;

        dfs3chunk1=chunk2;
        dfs3chunk2=chunk3;

        dfs4chunk1=chunk3;
        dfs4chunk2=chunk4;

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

        dfs2chunk1=chunk4;
        dfs2chunk2=chunk1;

        dfs3chunk1=chunk1;
        dfs3chunk2=chunk2;

        dfs4chunk1=chunk2;
        dfs4chunk2=chunk3;

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

        dfs2chunk1=chunk3;
        dfs2chunk2=chunk4;

        dfs3chunk1=chunk4;
        dfs3chunk2=chunk1;

        dfs4chunk1=chunk1;
        dfs4chunk2=chunk2;

        dfs1chunk1_size=chunk2_size;
        dfs1chunk2_size=chunk3_size;
        dfs2chunk1_size=chunk3_size;
        dfs2chunk2_size=chunk4_size;
        dfs3chunk1_size=chunk4_size;
        dfs3chunk2_size=chunk1_size;
        dfs4chunk1_size=chunk1_size;
        dfs4chunk2_size=chunk2_size;
     }

   //SENDING TO DFS1
      //SEND CHUNK SIZE FIRST, then the contents
      sprintf(buffer,"%ld",dfs1chunk1_size);
      if((nbytes=send(socket_desc1, buffer, sizeof(long),0)== -1))
          {
              printf("SENDING file TO DFS1 FAILED\n");
          }
      if((nbytes=send(socket_desc1, dfs1chunk1, dfs1chunk2_size,0)== -1))
          {
              printf("SENDING file TO DFS1 FAILED\n");
          }
      printf("Chunk 1 sent successfully to \n");


      sprintf(buffer,"%ld",dfs1chunk2_size);
      if((nbytes=send(socket_desc2, buffer, sizeof(long),0)== -1))
          {
              printf("SENDING file TO DFS1 FAILED\n");
          }
      if((nbytes=send(socket_desc2, dfs1chunk2, dfs1chunk2_size,0)== -1))
          {
              printf("SENDING file TO DFS1 FAILED\n");
          }
      printf("Chunk 2 sent successfully to \n");
      
      fclose(fp);
      close(socket_desc1);
      chunk1=NULL;
      // chunk2=NULL;
    // }
    return 0;
}