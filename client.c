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

	struct sockaddr_in udpSockAdrr;
	udpSockAdrr.sin_family = PF_INET;
	udpSockAddr.sin_port = htons(8221); //Change port
	inet_pton(PF_INET, "10.10.13.255", &mySockAddr.sin_addr); //NEED THE IP ADDRESS FOR THE STRING

	struct addrinfo hintsUDP;
	struct addrinfo *resultUDP, *rpUDP;
	int getaddrinfoValueUDP;
 	
 	memset(&hintsUDP, 0, sizeof(hintsUDP));
    hintsUDP.ai_family = PF_INET;    // Allow IPv4 or IPv6
    hintsUDP.ai_socktype = SOCK_DGRAM; // Datagram socket
    hintsUDP.ai_flags = AI_PASSIVE;    // Any IP address (DHCP)
    hintsUDP.ai_protocol = IPPROTO_UDP;   // Any protocol
    hintsUDP.ai_canonname = NULL;
    hintsUDP.ai_addr = NULL;
    hintsUDP.ai_next = NULL;

    getaddrinfoValueUDP = getaddrinfo(NULL, "8221", &hintsUDP, &resultUDP); //Change port number here 
    if (getaddrinfoValueUDP != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoValueUDP)); //Standard error message for printing getAddrInfo error
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind().
      If socket() or bind() fails, we close the socket
      and try the next address. 
    */

    for (rpUDP = resultUDP; rpUDP != NULL; rpUDP = rpUDP->ai_next) {  //for loop to continuously try a new socket until we can successfully create one
        int udpSocket = socket(rpUDP->ai_family, rpUDP->ai_socktype, rpUDP->ai_protocol); //create socket here with the addr info hints 
        if (udpSocket == -1)  //if we cannot create a socket with these hints then we will continue the for-loop and make a new one
            continue;

        if (bind(udpSocket, rpUDP->ai_addr, rpUDP->ai_addrlen) == 0) //try to bind the socket to the connection 
        	if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue)) == 0){ //Set the sockopts so that we can use the socket again without closing it 
				if(connect(udpSocket, (struct sockaddr*)&udpSockAdrr, sizeof(udpSockAdrr)));
        		//send a message to the server here after we simply connect to it
        	}
            break;  // Success

        close(udpSocket);
    }
    
	}

	

	



	
}
