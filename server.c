#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

#define PORT_SOURCE 9876
#define PORT_DEST 8756


// Server Client connection opens, closes - then we open a second time and send the actual UDP packets
//    /dev/urandom  file on CPU
//  listen -

int waitForConnection(int sockfd, struct sockaddr_in servaddr, struct sockaddr_in client, int *len)
{
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	int one = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	while (1)
	{
		// Binding newly created socket to given IP and verification
		if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
		{
			printf("socket bind failed...\n");
			usleep(500*1000);
		}
		else
		{
			printf("Socket successfully binded on port %d..\n", PORT);
			break;
		}
	}

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	*len = sizeof(client);

	int isConnected = 0;
	do
	{
		// Accept the data packet from client and verification
		int connfd = accept(sockfd, (SA *)&client, len);
		if (connfd < 0)
		{
			printf("server accept failed...\n");
			usleep(500 * 1000); // wait 500ms to try again
		}
		else
		{
			printf("server accept the client...\n");
			return connfd;
		}
	} while (isConnected == 0);

	return -1;
}

int openUDPSocket(int sockfd, struct sockaddr_in servaddr, struct sockaddr_in client, int *len ){
	// Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&client, 0, sizeof(client)); 
        
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
        
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    *len = sizeof(client);  //len is value/result 
}

// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	connfd = waitForConnection(sockfd, servaddr, cli, &len);
	int isConfigFinished = 0;
	char buff[MAX];
	do
	{

		bzero(buff, MAX);
		// wait to read incoming msg
		read(connfd, buff, sizeof(buff));
		printf("From client: %s\n", buff);
		if (strcmp(buff, "end") == 0)
		{
			isConfigFinished = 1;
		}
		//
	} while (isConfigFinished == 0);
	printf("Loaded config\n");
	close(connfd);
	close(sockfd);
	connfd = waitForConnection(sockfd, servaddr, cli, &len);

	//open udp connection
	


	printf("connected again\n");
	exit(1);
	/*
		int n;
		// infinite loop for chat
		for (;;)
		{
			bzero(buff, MAX);

			// read the message from client and copy it in buffer
			read(connfd, buff, sizeof(buff));
			// print buffer which contains the client contents
			printf("From client: %s\t To client : ", buff);
			bzero(buff, MAX);
			n = 0;
			// copy server message in the buffer
			while ((buff[n++] = getchar()) != '\n')
				;

			// and send that buffer to client
			// write(connfd, buff, sizeof(buff));

			// if msg contains "Exit" then server exit and chat ended.
			if (strncmp("exit", buff, 4) == 0)
			{
				printf("Server Exit...\n");
				break;
			}
		}*/

	// After chatting close the socket
	close(sockfd);
}
