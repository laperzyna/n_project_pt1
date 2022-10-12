#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

// json parser https://zserge.com/jsmn/

// client sends low entropy (all zeros) to sever, then wait, and send High Entropy

int openSocket(int sockfd, struct sockaddr_in servaddr){
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
	return sockfd;
}

int connectToServer(int sockfd, struct sockaddr_in servaddr)
{

	// socket create and verification
	/*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));*/

	/*int reuse = 1;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("SO_REUSEADDR failed");
		return -1;
	}*/
/*
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("SO_REUSEPORT failed");
		return -1;
	}*/

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	int isConnected = 0;
	do
	{
		// try to connect to client
		if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
		{
			printf("connection with the server on port %d failed...\n", PORT);
			usleep(500 * 1000);
		}
		else
		{
			printf("connected to the server on port %d..\n", PORT);
			isConnected = 1;
		}

	} while (isConnected == 0);

	return sockfd;
}

void sendTCPMsg(int sockfd, char *msg)
{

	int msgSize = sizeof(msg);
	char buff[msgSize];
	strcpy(buff, msg);
	write(sockfd, buff, sizeof(buff));
	printf("Sent %s on %d\n", buff, PORT);
	usleep(100 * 1000);
}
//https://www.geeksforgeeks.org/udp-server-client-implementation-c/
void sendUDP(int sockfd, char * msg){
	int msgSize = sizeof(msg);
	char buff[msgSize];
	strcpy(buff, msg);

	
	printf("Sent %s on %d\n", buff, PORT);
	usleep(100 * 1000);
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// waits and tries to reconnect
	openSocket(sockfd,servaddr);
	sockfd = connectToServer(sockfd, servaddr);
	sendTCPMsg(sockfd, "start");
	sendTCPMsg(sockfd, "IP:127.0.0.1");
	sendTCPMsg(sockfd, "SPUDP:8080");
	sendTCPMsg(sockfd, "DPUDP:8081");
	sendTCPMsg(sockfd, "end");
	close(sockfd);
	//connectToServer(sockfd, servaddr);

/*
	//openSocket for UDP messaging
	openSocket(sockfd,servaddr);
	// Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
    int n, len;         
    sendto(sockfd, "hello", 5, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Hello message sent.\n"); 
*/


	while (1)
	{

		sleep(3);
	}
	close(sockfd);
	exit(1);

	// main loop
	char buff[MAX];
	int n;
	for (;;)
	{
		// zero out the buffer
		bzero(buff, sizeof(buff));
		printf("Enter s to send messages");
		n = 0;

		// wait for the user input to send 2 messages - TESTING
		char cmd;
		scanf(" %c", &cmd);

		if (cmd == 's')
		{
			printf("Sending two messages...\n");
			strcpy(buff, "hello");
			write(sockfd, buff, sizeof(buff));

			sleep(1);

			bzero(buff, sizeof(buff));
			strcpy(buff, "hello2");
			write(sockfd, buff, sizeof(buff));
		}

		/*bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}*/
	}
	// close the socket
	close(sockfd);
}
