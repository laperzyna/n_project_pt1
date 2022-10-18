#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

/*
Client connects to server port, server listens to it's own port
Once connected, client reads a file and sends 1024 byte packets
to the server, then it closes the connection.
The server, accepts messages and listens for the connection to be dropped
, that is what tells the server we have finished sending the config file

TODO
Client sends UDP packets, server receives


*/

#define SERVER_PORT 8756

void sendFileToServer(char *filename, struct sockaddr_in s_addr, int connfd);

void sendTCPMsg(int sockfd, char *msg)
{

    int msgSize = sizeof(msg);
    char buff[msgSize];
    strcpy(buff, msg);
    write(sockfd, buff, sizeof(buff));
    printf("Sent %s on %d\n", buff, SERVER_PORT);
    usleep(100 * 1000);
}

int main(int argc, char *argv[])
{
    // system("clear");
    int sockfd = 0;
    int bytesReceived = 0;
    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in serv_addr;

    /* Create a socket first */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT); // port
    char *ip = "127.0.0.1";
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    /* Attempt a connection */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connected to ip: %s : %d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
    /*sendTCPMsg(sockfd, "start");
    sendTCPMsg(sockfd, "IP:127.0.0.1");
    sendTCPMsg(sockfd, "SPUDP:8080");
    sendTCPMsg(sockfd, "DPUDP:8081");
    sendTCPMsg(sockfd, "end");*/
    sendFileToServer("config.json", serv_addr, sockfd);
    // after the file has been sent close the connection
    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
    printf("Closed socket connection!\n");

    //----OPEN UDP CONNECTION-----
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    // Filling server information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);    
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    int n, len;

    while (1)
    {
        char c;
        printf("Enter [s] to send hello:");
        scanf("%c", &c);
      /*  if (c == 's')
        {
            sendto(sockfd, "Hello", 5,
                   MSG_CONFIRM, (const struct sockaddr *)&serv_addr,
                   sizeof(serv_addr));
            printf("Hello message sent.\n");
        } else if (c == 'q'){
            break;
        }
		*/
        
    }
    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
    return 0;
}

void sendFileToServer(char *filename, struct sockaddr_in s_addr, int connfd)
{
    printf("Connection accepted and id: %d\n", connfd);
    printf("Connected to Server: %s:%d\n", inet_ntoa(s_addr.sin_addr), ntohs(s_addr.sin_port));
    // write(connfd, fname,256);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open config json");
        return;
    }

    /* Read data from file and send it */
    while (1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[1024] = {0};
        int bytesToRead = 1024;
        int nread = fread(buff, 1, bytesToRead, fp);
        // printf("Bytes read %d \n", nread);

        /* If read was success, send data. */
        if (nread > 0)
        {
            // printf("Sending \n");
            write(connfd, buff, nread);
        }
        if (nread < 1024)
        {
            if (feof(fp))
            {
                printf("End of file\n");
                printf("File transfer completed for id: %d\n", connfd);
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
}