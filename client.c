#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>



#define BUF_SIZE 500


int main(argc char *argv[]){

	struct addrinfo hintsUDP;
	struct addrinfo *resultUDP, *rpUDP;
	int getaddrinfoValueUDP;

	memset(&hintsUDP, 0, sizeof(hintsUDP));
	


	int udpSocket;
	int optionValue = 1;
	struct sockaddr_in udpSockAddr;

	udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP
	if(udpSocket == 0){
		if(setsockopt(udpSocket,SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue))== 0){
			if(bind(udpSocket, ))
		}	
	}

	

	



	
}
