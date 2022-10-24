#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "JsonParse.h"

#define MAX 1024
#define SERVER_PORT 8756

char fname[100];
/*
void* SendFileToClient(int *arg)
{
      int connfd=(int)*arg;
      printf("Connection accepted and id: %d\n",connfd);
      printf("Connected to Clent: %s:%d\n",inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
       write(connfd, fname,256);

        FILE *fp = fopen(fname,"rb");
        if(fp==NULL)
        {
            printf("File opern error");
            return 1;
        }

        //Read data from file and send it
        while(1)
        {
            //First read file in chunks of 256 bytes
            unsigned char buff[1024]={0};
            int nread = fread(buff,1,1024,fp);
            //printf("Bytes read %d \n", nread);

            // If read was success, send data.
            if(nread > 0)
            {
                //printf("Sending \n");
                write(connfd, buff, nread);
            }
            if (nread < 1024)
            {
                if (feof(fp))
        {
                    printf("End of file\n");
            printf("File transfer completed for id: %d\n",connfd);
        }
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }
        }
printf("Closing Connection for id: %d\n",connfd);
close(connfd);
shutdown(connfd,SHUT_WR);
sleep(2);
}*/



int main(int argc, char *argv[])
{
    int connfd = 0, err;
    struct sockaddr_in serv_addr, client_addr;
    int listenfd = 0, ret;
    char sendBuff[1025];
    int numrv;
    socklen_t clen = 0;
    //----------OPEN THE SOCKET-------------
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("Error in socket creation\n");
        exit(2);
    }

    printf("Socket retrieve success\n");

    //----------BIND TO THE SOCKET-------------
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVER_PORT);

    ret = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        printf("Error in bind\n");
        exit(2);
    }

    //----------LISTEN TO THE SOCKET-------------
    if (listen(listenfd, 5) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }

    //----------WAIT FOR CLIENT CONNECTION-------------
    while (1)
    {
        clen = sizeof(client_addr);
        printf("Waiting...\n");
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &clen);
        if (connfd < 0)
        {
            printf("Error in accept\n");
            continue;
        } else {
            break;
        }
       
    }

    //----------WAIT AND RECEIVE CONFIG FILE DATA-------------
    int isConfigFinished = 0;
    char JSON_STRING[MAX], tempBuffer[MAX];
    
	do
	{

		bzero(tempBuffer, MAX);
		// wait to read incoming msg
		int bytesRead = recv(connfd, tempBuffer, sizeof(tempBuffer),0);

        if (bytesRead == -1){
            printf("Socket open but no data was read\n");
        } else if (bytesRead == 0){
            printf("Socket connection was closed\n");
            isConfigFinished = 1;
        } else {
            //abc   to read this we would have read 3 bytes
            //012
            tempBuffer[bytesRead] = '\0';
            strcpy(JSON_STRING,tempBuffer);                  
        }     

	} while (isConfigFinished == 0);
    //put the null terminating in the json string
    
    printf("From client:%s\n", JSON_STRING); 
    config c;    
    loadConfigStructFromConfigJSONString(JSON_STRING, &c);    	
    //close open connections
    close(connfd);
    shutdown(connfd, SHUT_RDWR);



    int sockUDPfd;

    //----------OPEN UDP CONNECTION-------------
    // Creating socket file descriptor 
    if ( (sockUDPfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&client_addr, 0, sizeof(client_addr)); 
        
    // Filling server information 
    serv_addr.sin_family    = AF_INET; // IPv4 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(SERVER_PORT); 
        
    // Bind the socket with the server address 
    if ( bind(sockUDPfd, (const struct sockaddr *)&serv_addr,  
            sizeof(serv_addr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    int len, n; 
    char buff[MAX];
    len = sizeof(client_addr);  //len is value/result 
    printf("Waiting for incoming UDP message");

    while(1){
        n = recvfrom(sockUDPfd, buff, MAX,  
                    MSG_WAITALL, ( struct sockaddr *) &client_addr, &len); 
        buff[n] = '\0'; 
        printf("Client : %s\n", buff);
        //right now each message is only 2 bytes
        int idByte2 = buff[0];
        int idByte1 = buff[1];
        //byte 2 needs to be shift by 8
        idByte2 = idByte2 << 8;
        int packetID = idByte2 + idByte1;
        printf("PacketID: %d\n", packetID);
    }

    close(sockUDPfd);
    shutdown(sockUDPfd, SHUT_RDWR);
    
    
    return 0;
}
