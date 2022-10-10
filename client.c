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
    hintsUDP.ai_family = PF_INET;    // Allow IPv4 or IPv6
    hintsUDP.ai_socktype = SOCK_DGRAM; // Datagram socket
    hintsUDP.ai_flags = AI_PASSIVE;    // Any IP address (DHCP)
    hintsUDP.ai_protocol = IPPROTO_UDP;          // Any protocol
    hintsUDP.ai_canonname = NULL;
    hintsUDP.ai_addr = NULL;
    hintsUDP.ai_next = NULL;

    getaddrinfoValueUDP = getaddrinfo(NULL, "8221", &hintsUDP, &resultUDP);
    if (getaddrinfoValueUDP != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoValueUDP));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind().
      If socket() or bind() fails, we close the socket
      and try the next address. 
    */

    for (rpUDP = resultUDP; rpUDP != NULL; rpUDP = rpUDP->ai_next) {
        int udpSocket = socket(rpUDP->ai_family, rpUDP->ai_socktype, rpUDP->ai_protocol);
        if (udpSocket == -1)
            continue;

        if (bind(udpSocket, rpUDP->ai_addr, rpUDP->ai_addrlen) == 0)
        	if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue)) == 0){
        		for(int i = 0; i <= 64; i++){
        			if(pfds[i].revents & POLLIN){
        				if(pfds[i].fd == udpSocket)
        					recvfrom(udpSocket);
        			}
        		}
        	}
            break;  // Success

        close(udpSocket);
    }
	}

	

	



	
}
