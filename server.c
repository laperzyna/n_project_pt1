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
#include <sys/time.h>

//use the time library to get the current time in milliseconds
long long millis() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    //this structure also has nano seconds if we need it  te.tv_nsec
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

#define MAX 1024
#define TIMEOUT 1000

char fname[100];
/*
TODO
Server should only use the TCP port from the config file
for receiving
How do we know which PORT to connect to to receive CONFIG information from client, if we dont have the
PORT in the server

*/

int main(int argc, char *argv[])
{


    char *JSON_STRING = loadJSONConfigStringFromFile("config.json");
    config c;
    loadConfigStructFromConfigJSONString(JSON_STRING, &c);

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
    serv_addr.sin_port = htons(c.portTCP);

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
        }
        else
        {
            break;
        }
    }

    //----------WAIT AND RECEIVE CONFIG FILE DATA-------------
    int isConfigFinished = 0;
    char JSON_STRING_REC[MAX], tempBuffer[MAX];

    do
    {

        bzero(tempBuffer, MAX);
        // wait to read incoming msg
        int bytesRead = recv(connfd, tempBuffer, sizeof(tempBuffer), 0);

        if (bytesRead == -1)
        {
            printf("Socket open but no data was read\n");
        }
        else if (bytesRead == 0)
        {
            printf("Socket connection was closed\n");
            isConfigFinished = 1;
        }
        else
        {
            // abc   to read this we would have read 3 bytes
            // 012
            tempBuffer[bytesRead] = '\0';
            strcpy(JSON_STRING_REC, tempBuffer);
        }

    } while (isConfigFinished == 0);
    // put the null terminating in the json string

    printf("From client:%s\n", JSON_STRING_REC);
    loadConfigStructFromConfigJSONString(JSON_STRING_REC, &c);
    // close open connections
    close(connfd);
    shutdown(connfd, SHUT_RDWR);

    int sockUDPfd;

    //----------OPEN UDP CONNECTION-------------
    // Creating socket file descriptor
    if ((sockUDPfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set this socket to NON BLOCKING, so that we don't get stuck waiting for messages
    // this is necesary to detect a timeout
    // O_NONBLOCK
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    // set the timeout to 100 microseconds, if the socket doesnt have information it returns -1
    read_timeout.tv_usec = 100;
    setsockopt(sockUDPfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Filling server information
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(c.portTCP);

    // Bind the socket with the server address
    if (bind(sockUDPfd, (const struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    // TODO remove payloadsize
    unsigned char buff[c.udpPayloadSize + 2];
    len = sizeof(client_addr); // len is value/result
    printf("Waiting for incoming UDP message...\n");
    int numPacketsReceived_low = 0;
    int numPacketsReceived_high = 0;

//set alarm for timeout 
//https://man7.org/linux/man-pages/man2/alarm.2.html

    long long firstMsgTime = -1;
    long long lastMsgRecTime = -1;

    long long packetTimes[10000]; //make the a variable at the top but what is MAX num packets in train?
    int msgCo = 0;
    while (1)
    {
        // we need a non-blocking version of this?
        n = recvfrom(sockUDPfd, buff, MAX,
                     MSG_WAITALL, (struct sockaddr *)&client_addr,
                     &len);
        // get the time
      // printf("N is %d\n", n);

        if (n != -1)
        {
            
            packetTimes[msgCo] = millis();
            msgCo++;  

            //write down the time of this message
            lastMsgRecTime = millis();         

            // buff[n] = '\0';
            // printf("Received %d bytes from Client : %s\n", n, buff);
            // right now each message is only 2 bytes
            int idByte2 = buff[0];
            int idByte1 = buff[1];
            // byte 2 needs to be shift by 8
            idByte2 = idByte2 << 8;
            int packetID = idByte2 + idByte1;
            printf("PacketID: %d\n", packetID);
            // need to check the data for low or high entropy
            // the logic is easier if we assume it is low entropy
            // and flip that assumption when we get a NON ZERO VALUE
            int isLowEntropy = 1; // use int as boolean for 1 is true 0 is false
            // skip the ID
            // if payload size is 1000, whole buffer is 1002
            // so stay less than 1002
            for (int i = 2; i < c.udpPayloadSize + 2; i++)
            {
                // if we don't have a zero it's not low entropy
                if (buff[i] != 0)
                {
                    isLowEntropy = 0;
                    break; // no need to check anymore
                }
                // printf("%d ", (int) buff[i]);
            }
            // printf("\n");
            // check if this was low entropy or not
            if (isLowEntropy == 1)
            {
                // start timer
                printf("GOT LOW ENTROPY\n");
            }
            else
            {
                printf("GOT HIGH ENTROPY\n");
            }
        } else {
            //check for timeout, check the time right now vs the last message we received
            if (lastMsgRecTime != -1){
                //when we timeout we're not going to actually get another msg so
                msgCo--;

                //if we already got at least one msg
                //if the time right now is 1000ms after the last message we received then we timeout and break out of the loop
                if( millis() - lastMsgRecTime > TIMEOUT){
                    break;
                }
            }
        }
    }
    printf("TIMEOUT:  %ld   -  %ld  difference from first to last msg: %ld\n", packetTimes[msgCo], packetTimes[0],packetTimes[msgCo] - packetTimes[0]);
   
    close(sockUDPfd);
    shutdown(sockUDPfd, SHUT_RDWR);

    return 0;
}
