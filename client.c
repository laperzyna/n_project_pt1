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

#define MSG_CONFIRM  0

/*
Client connects to server port, server listens to it's own port
Once connected, client reads a file and sends 1024 byte packets
to the server, then it closes the connection.
The server, accepts messages and listens for the connection to be dropped
, that is what tells the server we have finished sending the config file


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

JSON Parser Library
https://github.com/zserge/jsmn
*/

void sendFileToServer(char *filename, struct sockaddr_in s_addr, int connfd);
void sendJSONStringToServer(char *JSON_String, struct sockaddr_in s_addr, int connfd);
void sendPacketTrain(int sockfd, config c, struct sockaddr_in *servadd, char *dataToSend, int numPacketsToSend);

/* TODO
 -change the way the config file is read in, right now we assumed it
 would always be complete and in the correct order so we used index numbers
 to grab the config values, but we should really be trying to get the values
 based on matching their key
    Ex: we should iterate through the token array and check something like
    if (t[i].label == "sourcePort"){
        set the source port
    }
    *NOTE the if statement is wrong check the sample code for a json key equivalent function
    that finds the matches of the key*
- Dont fragment flag 
-TTL is the time to live - value for period of time packet should exist on a computer before disregarded
*/

#define MAX_BUFFER_SIZE 1024
config c;

int main(int argc, char *argv[])
{

    //----LOAD JSON CONFIG FROM FILE
    char *JSON_STRING = loadJSONConfigStringFromFile("config.json");
    loadConfigStructFromConfigJSONString(JSON_STRING, &c);


    //-----OPEN TCP CONNECTION FOR SENDING SERVER FILE------
    int sockfd = 0;    
    struct sockaddr_in serv_addr;   

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(c.portTCP); // port
    serv_addr.sin_addr.s_addr = inet_addr(c.IP);   

    //the server may not be started so try this in a while loop
    while (1){
        /* Attempt a connection */
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("Error : Connect Failed. Trying again in 1 second... \n");
            sleep(1);
        }  else {
            printf("Connected to server!\n");
            break;
        }
    }

    printf("Connected to ip: %s : %d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
    sendJSONStringToServer(JSON_STRING, serv_addr, sockfd);
    // after the file has been sent close the connection
    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
    printf("Closed socket connection!\n");


    //-------OPEN UDP CONNECTION-----
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

   

    // TODO FINISH THIS
    //  set the dont fragment flag
    //  int val = 1;
    //  setsockopt(sockfd, IPPROTO_IP, IP_DONTFRAG, &val, sizeof(val));

    //--------PREPARE THE CHARCTER BUFFERS FOR LOW AND HIGH ENTROPY----------
    // prepare the UDP PAYLOAD options, we will add the ID's in front of this
    // All 0s
    unsigned char lowEntropy[c.udpPayloadSize];
    // use memset to fill this array with 0's
    // after the two ID bytes
    memset(lowEntropy, 0, c.udpPayloadSize);

    unsigned char highEntropy[c.udpPayloadSize];

    // Open the file with random numbers for high entropy
    FILE *fileRand = fopen("highEntropy", "r");
    // check to make sure it is open, exit program if not
    if (fileRand == NULL)
    {
        printf("Could not open the random file!\n");
        exit(1);
    }

    // the first two bytes are the ID so place these characters
    // start at index 2
    // 168 100 151 99 236 70
    for (int i = 0; i < c.udpPayloadSize; i++)
    {
        char curByte;
        fscanf(fileRand, "%c", &curByte);
        // printf("char %c   int:%d\n", curByte, (int)curByte);
        // 0 to 999 i in for loop
        // 2 to 1001 the index below
        highEntropy[i] = curByte;
        // printf("%d ", (int) highEntropy[i]);
    }
    // printf("\n");


    //-------SEND THE ENTROPY PACKET TRAINS
    //send low entropy packet train
    sendPacketTrain(sockfd, c, &serv_addr, lowEntropy, c.numUDPPackets);
    //   
    int timeLeft = c.interMeasurementTime;
    while(timeLeft){
        sleep(1);
        timeLeft--;
        printf("Waiting %d seconds to send second packet train...\n",timeLeft);
        
    }
    sendPacketTrain(sockfd, c, &serv_addr, highEntropy, c.numUDPPackets);

    //-----------CREATE TCP CONNECTION TO RECEIVE COMPRESSION RESULT------
     /* Create a socket first */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    
    //Connect to server with TCP
    //the server may not be started so try this in a while loop
    while (1){
        /* Attempt a connection */
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("Error : Connect Failed. Trying again in 1 second... \n");
            sleep(1);
        }  else {
            printf("Connected to server!\n");
            break;
        }
    }

    printf("Waiting for message from server....\n");
    unsigned char recBuffer[MAX_BUFFER_SIZE];
    memset(recBuffer,0,MAX_BUFFER_SIZE);
    int numBytesRead;
    
    do{
        numBytesRead = recv(sockfd, recBuffer, sizeof(recBuffer), 0);
        recBuffer[numBytesRead] = '\0';
        //XXX
        //0123
    } while(numBytesRead <= 0);
    printf("Recieved from server: %s\n", recBuffer);

    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
    printf("Shutting down...\n");
    // we need to free the space allocated for the json string
    free(JSON_STRING);
    return 0;
}

void sendPacketTrain(int sockfd, config c, struct sockaddr_in *servaddr, char *dataToSend, int numPacketsToSend)
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

    // we need to send multiple messages for use a for loop
    for (int packetID = 0; packetID < numPacketsToSend; packetID++)
    {

        // char can only hold 8 bits
        // so and the number with 8 1's using the 0xFF
        // and this gives us the right 8 bits
        unsigned char idByteRight = packetID & 0xFF;
        // Then we need the left 8 bits, so just shift the whole number
        // right 8 bits and do the same and as above
        unsigned char idByteLeft = (packetID >> 8); //& 0xFF;

        // the size of the message is the number from the config
        //+2 because we have a 2 byte packet id
        unsigned char udpPacket[c.udpPayloadSize + 2];
        udpPacket[0] = idByteLeft;
        udpPacket[1] = idByteRight;
        // dataToSend is either 0's or the random entropy data
        // dataToSend is size 10
        // copy the data into the udpPacket but dont go over
        // the ID
        strncpy(&udpPacket[2], dataToSend, c.udpPayloadSize);
       /* for (size_t i = 2; i < c.udpPayloadSize + 2; i++)
        {
            printf("%d ", (int)udpPacket[i]);
        }
        printf("\n");*/

        printf("Sent packet %d\n", packetID);
        // write(sockfd, test, 2);
        // MSG_CONFIRM is included in <sys/socket.h>
        sendto(sockfd, udpPacket, c.udpPayloadSize + 2, MSG_CONFIRM,
               (const struct sockaddr *)servaddr,
               sizeof(*servaddr));
    }
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
    unsigned char buff[msgSize];
    strcpy(buff, msg);
    write(sockfd, buff, sizeof(buff));
    printf("Sent %s on %d\n", buff, c.portTCP);
    usleep(100 * 1000);
}
