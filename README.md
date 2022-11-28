# Networks Final Project Part 1

## Table of Contents
1. [Description](#description)
2. [Instructions](#instructions)
3. [Incomplete Features](#incomplete-features)
7. [Contributers](#contributers)

## Description
This project is an example of End-to-End Detection of Network Compression. It is an application that uses a server and a client as well as a third party JSON parser and an inputed config file. This application will detect compression upon the different two UDP packet trains, one with high entropy data and one with low entropy data.

## Instructions
In order to run this program:
1. Clone the repo from github or include the following files in the same folder:
    - highEntropy
    - jsmn.h (https://github.com/zserge/jsmn) <-- link to this third party parser
    - JsonParse.h
    - Makefile
    - client.c
    - server.c
    - a config.json file
2. Once you have the correct file structure you may simply just run "make client" and "make server" in order to compile the two programs
3. In order to run the program you must open two terminals
    - in one terminal write "./client [your config.json file]"
    - in the other terminal write "./server [your config.json file]"


## Incomplete Features
N/A We were able to fullfill all required features

## Contributers
Maleke Hanson <br>
Lidia Perzyna
