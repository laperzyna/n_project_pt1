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
#include "jsmn.h"
#include "JsonParse.h"

#define MSG_CONFIRM 0

/*
Client connects to server port, server listens to it's own port
Once connected, client reads a file and sends 1024 byte packets
to the server, then it closes the connection.
The server, accepts messages and listens for the connection to be dropped
, that is what tells the server we have finished sending the config file

TODO
Client sends UDP packets, server receives


*/

/*UDP Packet

https://docs.oracle.com/cd/E36784_01/html/E36875/setsockopt-3socket.html
Prof:

For Part 1 you don't need to create any packets manually.
For set Don't Fragment flag, you can use setsockopt(),
provided in the project resources:

https://beej.us/guide/bgnet/html/#datagram

The process to create a UDP packet for Part 1 is very similar to how
it is presented here: Section 6.3, talker.c.
*/

#define SERVER_PORT 8756

void sendFileToServer(char *filename, struct sockaddr_in s_addr, int connfd);
char *loadJSONConfigStringFromFile(char *filename);
void sendJSONStringToServer(char *JSON_String, struct sockaddr_in s_addr, int connfd);
void sendPacketTrain(int sockfd, config c, struct sockaddr_in *servadd, char *dataToSend);

// TODO
// make function for sending packet train, that takes in entropy bytes
// on server side, for now just time the length that one train takes

// High Entropy
// read the dev file, 1500-2000 bytes only once and use this
// repeatedly for all programs

int main(int argc, char *argv[])
{

    // Load config from file
    char *JSON_STRING = loadJSONConfigStringFromFile("config.json");
    config c;
    loadConfigStructFromConfigJSONString(JSON_STRING, &c);

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
    serv_addr.sin_port = htons(c.sourcePort); // port

    serv_addr.sin_addr.s_addr = inet_addr(c.serverIP);

    /* Attempt a connection */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connected to ip: %s : %d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
    sendJSONStringToServer(JSON_STRING, serv_addr, sockfd);
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
    serv_addr.sin_port = htons(c.sourcePort);
    serv_addr.sin_addr.s_addr = inet_addr(c.serverIP);

    // TODO FINISH THIS
    //  set the dont fragment flag
    //  int val = 1;
    //  setsockopt(sockfd, IPPROTO_IP, IP_DONTFRAG, &val, sizeof(val));

    int n, len;

    while (1)
    {
        char temp;
        printf("Enter [s] to send hello:");
        scanf("%c", &temp);
        if (temp == 's')
        {
            /*sendto(sockfd, "Hello", 5,
                   MSG_CONFIRM, (const struct sockaddr *)&serv_addr,
                   sizeof(serv_addr));
            printf("Hello message sent.\n");*/
            sendPacketTrain(sockfd, c, &serv_addr, NULL);
        }
        else if (temp == 'q')
        {
            break;
        }
    }
    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
    // we need to free the space allocated for the json string
    free(JSON_STRING);
    return 0;
}

void sendPacketTrain(int sockfd, config c, struct sockaddr_in *servaddr, char *dataToSend)
{

    // int numBytesToSend = strlen(JSON_String);
    // char test = 6000;

    // we need to make a packet ID, this is a 2 byte number
    // so we have to use bit shifting to split our
    // starting integer into 2 separate bytes
    //  [byte2 byte1] - 6000
    //  6000 in binary 00010111 01110000
    // to get each byte by itself we add it with 1's
    //                23       112
    //            00010111 01110000
    // idbyte1 &  00000000 11111111

    //           00010111 01110000
    // idbyte2 &  11111111 00000000
    // then bit shift to the right 8

    //char can only hold 8 bits
    //so and the number with 8 1's using the 0xFF
    //and this gives us the right 8 bits
    char idByteRight = c.numUDPPackets & 0xFF;
    //Then we need the left 8 bits, so just shift the whole number
    //right 8 bits and do the same and as above
    char idByteLeft = (c.numUDPPackets >> 8); //& 0xFF;

    // the size of the message is the number from the config
    //+2 because we have a 2 byte packet id
    char udpPacket[c.udpPayloadSize + 2];
    udpPacket[0] = idByteLeft;
    udpPacket[1] = idByteRight;
    //dataToSend is either 0's or the random entropy data
    //dataToSend is size 10
    //copy the data into the udpPacket but dont go over
    //the ID
    strncpy(&udpPacket[2], dataToSend, c.udpPayloadSize);
    

    printf("byte2: %d  byte1: %d\n", (int)idByteLeft, (int)idByteRight);
    // write(sockfd, test, 2);
    // MSG_CONFIRM is included in <sys/socket.h>
    sendto(sockfd, udpPacket, 2, MSG_CONFIRM,
           (const struct sockaddr *)servaddr,
           sizeof(*servaddr));
}

void sendJSONStringToServer(char *JSON_String, struct sockaddr_in s_addr, int connfd)
{
    printf("Connection accepted and id: %d\n", connfd);
    printf("Connected to Server: %s:%d\n", inet_ntoa(s_addr.sin_addr), ntohs(s_addr.sin_port));
    // write(connfd, fname,256);

    int numBytesToSend = strlen(JSON_String);
    write(connfd, JSON_String, numBytesToSend);
}

void sendTCPMsg(int sockfd, char *msg)
{

    int msgSize = sizeof(msg);
    char buff[msgSize];
    strcpy(buff, msg);
    write(sockfd, buff, sizeof(buff));
    printf("Sent %s on %d\n", buff, SERVER_PORT);
    usleep(100 * 1000);
}

// this function takes all the information in the config file
// and puts it into a char array
char *loadJSONConfigStringFromFile(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open config json");
        exit(1);
    }

    // we need to allocate enough space to save the JSON string in a
    // char array
    fseek(fp, 0, SEEK_END); // seek to end of file
    int size = ftell(fp);   // get current file pointer
    fseek(fp, 0, SEEK_SET); // seek back to beginning of file

    char *jsonConfigString = (char *)malloc(sizeof(char) * (size + 1));
    // while we're NOT at the end of the file
    int idx = 0;
    while (idx < size)
    {
        char c = fgetc(fp);
        jsonConfigString[idx] = c;
        idx++;
    }
    // put in the null terminating
    jsonConfigString[idx] = '\0';
    return jsonConfigString;
}
