/*
 * helper.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <netdb.h>
#include <list>
#include "host.h"

using namespace std;

vector<char*>* tokenize(char *inputString, const char* delimiter) {
	vector<char*>* tokens = new vector<char*>();
	tokens->clear();
	int len = strlen(inputString);
	char* dupString = new char[len+1];
	strcpy(dupString, inputString);
	char* tok = strtok(dupString, delimiter);
	while (tok != NULL)
	{
		tokens->push_back(tok);
		tok = strtok (NULL, delimiter);
	}
	return tokens;
}

char* GetMyIP(char **outHostname) {
	struct sockaddr_in address;
	//create an UDP socket
	int udpSocket;
	if( (udpSocket = socket(AF_INET , SOCK_DGRAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	const char *googleIP = "74.125.225.115";

	int err = inet_pton(AF_INET, googleIP, &address.sin_addr.s_addr);
	if(err == 0) {
		perror("inet_pton() failed - Invalid address string");
		exit(1);
	}
	else if(err < 0) {
		perror("inet_pton() failed");
		exit(1);
	}
	address.sin_port = htons(80);

	if( connect(udpSocket, (struct sockaddr *)&address, sizeof(address))) {
		perror("connect failed\n");
		exit(1);
	}
	struct sockaddr_in addr;
	unsigned int len = sizeof(sockaddr);


	if( getsockname(udpSocket, (struct sockaddr*)&addr, &len)) {
		perror("getsockname failed\n");
		exit(1);
	}

	char* clientIP = inet_ntoa(addr.sin_addr);
	/*
	char host[1024];
	char service[20];

	// pretend sa is full of good information about the host and port...

	getnameinfo((struct sockaddr*)&addr, sizeof addr, host, sizeof host, service, sizeof service, 0);
	 */

	char *host = new char[50];
	char serv[50];
	if(getnameinfo((struct sockaddr*)&addr, len, host, 50, serv, 50, 0)) {
		perror("getnameinfo failed\n");
		exit(1);
	}
	printf("Host is: %s\nIP: %s\n", host, clientIP);
	*outHostname = new char [strlen(host) + 1];
	strcpy(*outHostname, host);
	return clientIP;
}

char* ToUpper(char* input) {
	char* output = new char[strlen(input) + 1];
	char c;
	int count = 0;
	while((c = input[count]) != '\0') {
		output[count] = toupper(c);
		count++;
	}
	output[count] = '\0';
	return output;
}

int TCPConnect(char* IP, int nPort, bool exitOnFail, char *outHostName /*=NULL*/) {
	//code  copied from http://linux.die.net/man/3/getaddrinfo and http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sd,s;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */
	char strPort[10];
	sprintf(strPort, "%d", nPort);

	s = getaddrinfo(IP, strPort, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	       Try each address until we successfully connect(2).
	       If socket(2) (or connect(2)) fails, we (close the socket
	       and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sd = socket(PF_INET, SOCK_STREAM,
				0);
		if (sd == -1)
			continue;

		if (connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;                  /* Success */

		close(sd);
	}
	if(outHostName!=NULL) {
		char *host = outHostName;
		char serv[50];
		char ipstr[40];
		int err;
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
		void*  addr = &(ipv4->sin_addr);
		inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
		struct sockaddr_in sa;
		sa.sin_family = AF_INET;
		//I have no idea why I have to do inet_ntop and back but it just wouldn't work without it
		inet_pton(AF_INET, ipstr, &sa.sin_addr);

		if(err = getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, 50, serv, 50, 0)) {
			printf("getnameinfo failed: %d%s\n", err,ipstr);
			exit(1);
		}
	}
	return sd;
}

int TCPSend(int sd, const char* sendBuffer, int length, bool exitOnFail) {
	int bytesWritten;
	int remainingBytes = length;
	int index = 0;
	while(remainingBytes > 0)
	{
		if( (bytesWritten = write(sd, &(sendBuffer[index]), remainingBytes))==-1 ) {
			perror("\nwrite failed in Register\n");
			if(exitOnFail)
				exit(1);
			return 1;
		}
		index +=bytesWritten;
		remainingBytes -= bytesWritten;
	}
	return 0;
}

int TCPRecv(int sd, char* readBuffer, int length, bool exitOnFail) {
	int bytesRead;
	int remainingBytes = length;
	int index = 0;
	while (remainingBytes > 0) {
		if((bytesRead = read(sd, &(readBuffer[index]), remainingBytes)) ==-1) {
			perror("\nread failed\n");
			if(exitOnFail)
				exit(1);
			return 1;
		}
		index +=bytesRead;
		remainingBytes -= bytesRead;
	}
	return 0;
}

bool Contains(list<Host*>* clientList, Host* item) {
	for (std::list<Host*>::iterator it = clientList->begin(); it != clientList->end(); it++)
		if(item->IsEqual(*it))
			return true;
	return false;
}

void PrintClientList(list<Host*>* clientList) {
	int index = 1;
	for (std::list<Host*>::iterator it = clientList->begin(); it != clientList->end(); it++) {
		//printf("%d\t%s\t%s\n", index++, (*it)->mIP, (*it)->mPort);
		printf("%-5d%-35s%-20s%-8s\n", index++, (*it)->mHostname, (*it)->mIP, (*it)->mPort);
	}
}

void PrintVector(vector<Host*>* vector) {
	int index = 1;
	printf("\n");
	for(unsigned int i = 0; i < vector->size(); i++) {
		printf("%d\t%s\t%s\t%s\n", index++, vector->at(i)->mHostname, vector->at(i)->mIP, vector->at(i)->mPort);
	}
}

char* SerializeList(std::list<Host*>* clientList) {
	char* returnval = new char[510];
	int index = 0;
	for (std::list<Host*>::iterator it = clientList->begin(); it != clientList->end(); it++) {
		strcpy( (&returnval[index]), (*it)->mIP );
		index += strlen((*it)->mIP);
		returnval[index] = '|';
		index++;
		strcpy( (&returnval[index]), (*it)->mHostname );
		index += strlen((*it)->mHostname);
		returnval[index] = '|';
		index++;
		strcpy( (&returnval[index]), (*it)->mPort );
		index += strlen((*it)->mPort);
		returnval[index] = '|';
		index++;
	}
	returnval[index] = '\0';
	return returnval;
}

