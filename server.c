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

// use the time library to get the current time in milliseconds
long long millis()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    // this structure also has nano seconds if we need it  te.tv_nsec
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

#define MAX 1024
#define MAX_PACKETS_IN_TRAIN 10000
#define TIMEOUT 1000
#define COMPRESSIOIN_THRESHOLD 100

/*
TODO

*/

long long waitForPacketTrain(int sockUDPfd, struct sockaddr_in client_addr)
{
    long long packetTimes[MAX_PACKETS_IN_TRAIN]; // make the a variable at the top but what is MAX num packets in train?
    unsigned char buff[MAX];
    int len = sizeof(client_addr);
    int msgCo = 0;
    long long lastMsgRecTime = -1;
    while (1)
    {
        // we need a non-blocking version of this?
        int numBytesRead = recvfrom(sockUDPfd, buff, MAX,
                                    MSG_WAITALL, (struct sockaddr *)&client_addr,
                                    &len);
        // get the time
        // printf("N is %d\n", n);

        if (numBytesRead != -1)
        {
            // packetTimes[] = { time1, time2, time3}     msgCo is 3
            //                     0     1       2
            packetTimes[msgCo] = millis();
            msgCo++;

            // write down the time of this message
            lastMsgRecTime = millis();

            // buff[n] = '\0';
            // printf("Received %d bytes from Client : %s\n", n, buff);
            // right now each message is only 2 bytes
            int idByte2 = buff[0];
            int idByte1 = buff[1];
            // byte 2 needs to be shift by 8
            idByte2 = idByte2 << 8;
            int packetID = idByte2 + idByte1;
            // printf("PacketID: %d\n", packetID);
            // need to check the data for low or high entropy
            // the logic is easier if we assume it is low entropy
            // and flip that assumption when we get a NON ZERO VALUE
            int isLowEntropy = 1; // use int as boolean for 1 is true 0 is false
            // skip the ID
            // if payload size is 1000, whole buffer is 1002
            // so stay less than 1002
            for (int i = 2; i < numBytesRead; i++)
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
                // printf("GOT LOW ENTROPY\n");
            }
            else
            {
                // printf("GOT HIGH ENTROPY\n");
            }
        }
        else
        {
            // check for timeout only if we have received at least one message so far
            //, check the time right now vs the last message we received
            if (lastMsgRecTime != -1)
            {
                // when we timeout we're not going to actually get another msg so
                msgCo--;

                // if we already got at least one msg
                // if the time right now is 1000ms after the last message we received then we timeout and break out of the loop
                if (millis() - lastMsgRecTime > TIMEOUT)
                {
                    break;
                }
            }
        }
    } // end while
    // at this point we have an array of the times, so packetTimes[0] is the first time
    // and we fixed msgCo to represent the index of the last message received
    // so packeTimes[msgCo] should be the time of the last message
    printf("Received %d packets\n", msgCo);
    return packetTimes[msgCo] - packetTimes[0];
}

int main(int argc, char *argv[])
{

    char *JSON_STRING = loadJSONConfigStringFromFile("config.json");
    config c;
    loadConfigStructFromConfigJSONString(JSON_STRING, &c);

    int connfd = 0, err;
    struct sockaddr_in serv_addr, client_addr;
    int sockTCP = 0, ret;
    char sendBuff[1025];
    int numrv;
    socklen_t clen = 0;
    //----------OPEN THE SOCKET-------------
    sockTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (sockTCP < 0)
    {
        printf("Error in socket creation\n");
        exit(2);
    }

    printf("Socket retrieve success\n");

    //----------BIND TO THE SOCKET-------------
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(c.portTCP);

    ret = bind(sockTCP, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        printf("Error in bind\n");
        exit(2);
    }

    //----------LISTEN TO THE SOCKET-------------
    if (listen(sockTCP, 5) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }

    //----------WAIT FOR CLIENT CONNECTION-------------
    while (1)
    {
        clen = sizeof(client_addr);
        printf("Waiting...\n");
        connfd = accept(sockTCP, (struct sockaddr *)&client_addr, &clen);
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
    close(sockTCP);
    shutdown(sockTCP, SHUT_RDWR);

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

    int len;
    unsigned char buff[MAX];
    len = sizeof(client_addr); // len is value/result
    printf("Waiting for incoming UDP message...\n");
    int numPacketsReceived_low = 0;
    int numPacketsReceived_high = 0;

    // set alarm for timeout
    // https://man7.org/linux/man-pages/man2/alarm.2.html

    // TODO we are assuming that the client will always send the low entropy first
    long long lowEntropyTime = waitForPacketTrain(sockUDPfd, client_addr);
    printf("Waiting for the second packet train....\n");
    long long highEntropyTime = waitForPacketTrain(sockUDPfd, client_addr);

    //TODO sometimes the lowEntropyTime is higher than highEntropy which tries to give a negative number
    //but long long cant hold a negative so it wraps around and gives a HUGE number which is ALWAYS greater than the
    //threshold so lets say if we get a negative from this math that we fix it to zero
    long long timeDifference = highEntropyTime - lowEntropyTime;
    //set the time difference and then check to see if it might be wrong
    //because lowEntropyTime was greater than high
    if (lowEntropyTime > highEntropyTime){
        timeDifference = -1;
        printf("The low entropy time was greater than high\n");
    }

    printf("Low Entropy: %llu\n", lowEntropyTime);
    printf("High Entropy: %llu\n", highEntropyTime);
    printf("Time Difference:  %llu\n", timeDifference);

    char sendMsgToClient[MAX];
    //when using the COMPRESSION_THRESHOLD macro above this if statement was false when the statement was
    // if (12308781234 > 100) so we tried to compare 2 longs incase it was a variable type issue
    if (timeDifference > 100LL)
    {
        strcpy(sendMsgToClient, "Compression Detected!");
    }
    else
    {
        strcpy(sendMsgToClient, "No compression detected.");
    }
    printf("%s\n", sendMsgToClient);

    //----REOPEN A NEW TCP CONNECTION
    //----------OPEN THE SOCKET-------------
    sockTCP = socket(AF_INET, SOCK_STREAM, 0);
    if (sockTCP < 0)
    {
        printf("Error in socket creation\n");
        exit(2);
    }

    printf("Socket retrieve success\n");

    ret = bind(sockTCP, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        printf("Error in bind\n");
        exit(2);
    }

    //----------LISTEN TO THE SOCKET-------------
    if (listen(sockTCP, 5) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }

    //----------WAIT FOR CLIENT CONNECTION-------------
    while (1)
    {
        clen = sizeof(client_addr);
        connfd = accept(sockTCP, (struct sockaddr *)&client_addr, &clen);
        if (connfd < 0)
        {
            printf("Error in accept\n");
            continue;
        }
        else
        {
            printf("Connected to client TCP\n");
            break;
        }
    }

    int numBytesToSend = strlen(sendMsgToClient);
    write(connfd, sendMsgToClient, numBytesToSend);



    close(connfd);
    shutdown(connfd, SHUT_RDWR);
    close(sockTCP);
    shutdown(sockTCP, SHUT_RDWR);
    close(sockUDPfd);
    shutdown(sockUDPfd, SHUT_RDWR);

    return 0;
}
