

// TODO header guards

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "jsmn.h"

#define MAX_SIZE_CONFIG_STRING 15

typedef struct config {
    char serverIP[MAX_SIZE_CONFIG_STRING];
    int sourcePort;
    int destPort;
    int destPortTCPHead;
    int destPortTCPTail;
    int portTCP;
    int udpPayloadSize;
    int interMeasurementTime;
    int numUDPPackets;
    int UDPPacketTTL;
} config;

#define MAX_NUM_TOKENS 50
jsmn_parser p;
// This is an array of tokens
// A token saves the start position of a key and a value
// Ex:
//"IP" : "127.0.0.01"
// This would be a 2 index jsmntok_t array
// where t[0] holds where the key starts
// and t[1] holds where the value starts
jsmntok_t t[MAX_NUM_TOKENS]; /* We expect no more than 128 tokens */

//This function takes in a "string" that holds JSON data
//in the correct json format
void parseJSONFromString(char * JSON_STRING)
{
    jsmn_init(&p);
    int res = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t,
                         sizeof(t) / sizeof(t[0]));
    if (res < 0)
    {
        printf("Failed to parse JSON: %d\n", res);
        exit(1); // if the json cant be parsed we cant do anything
        // stop the program
    }
}

void getStringFromJSON(char * json, char *saveString, int idxOfKeyInTArray)
{
    //"json" starts at the beginning of the JSON String pointer
    // but we have to offset to the start of the value we want
    // to get. The tokener array saves where all the different values
    // start so use the token data to move to the right spot
    json += t[idxOfKeyInTArray + 1].start;
    int numLetters = t[idxOfKeyInTArray + 1].end - t[idxOfKeyInTArray + 1].start;
    strncpy(saveString, json, numLetters);
    saveString[numLetters] = '\0';
}

int getIntFromJSON(char * json, int i)
{
    char saveString[MAX_SIZE_CONFIG_STRING];
    getStringFromJSON(json, saveString, i);
    return atoi(saveString);
}


// this function will load the data from the config file
// into the config structure
void loadConfigStructFromConfigJSONString(char *jsonConfigString, config *c)
{
    // The third party library needs to fill it's tokener array based
    // on the JSOn string
    parseJSONFromString(jsonConfigString); // this should fill t[]

    //from printouts it seems like index 0 in the t array
    //is all of the data, the tokens actually start at 1
     // the 1 at the end is for the first key in our json file
    // the 2 index is the value so all keys should be at
    // odd indexes
    getStringFromJSON(jsonConfigString, c->serverIP, 1);
    c->sourcePort = getIntFromJSON(jsonConfigString, 3);
    c->destPort = getIntFromJSON(jsonConfigString, 5);
    c->destPortTCPHead = getIntFromJSON(jsonConfigString, 7);
    c->destPortTCPTail = getIntFromJSON(jsonConfigString, 9);
    c->portTCP = getIntFromJSON(jsonConfigString, 11);
    c->udpPayloadSize = getIntFromJSON(jsonConfigString, 13);
    c->interMeasurementTime = getIntFromJSON(jsonConfigString, 15);
    c->numUDPPackets = getIntFromJSON(jsonConfigString, 17);
    c->UDPPacketTTL = getIntFromJSON(jsonConfigString, 19);

    printf("Config:\n");
    printf("IP: %s\n", c->serverIP);
    printf("source port: %d\n", c->sourcePort);
    printf("dest port: %d\n", c->destPort);
    printf("dest port tcp head: %d\n", c->destPortTCPHead);
    printf("dest port tcp tail: %d\n", c->destPortTCPTail);
    printf("port tcp: %d\n", c->portTCP);
    printf("udp payload: %d\n", c->udpPayloadSize);
    printf("interMeasurementTime: %d\n", c->interMeasurementTime);
    printf("numUDPPackets: %d\n", c->numUDPPackets);
    printf("UDPPacketTTL: %d\n", c->UDPPacketTTL);

}