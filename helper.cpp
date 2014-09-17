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

using namespace std;

vector<char*>* tokenize(char *inputString, char* delimiter) {
	vector<char*>* tokens = new vector<char*>();
	int len = strlen(inputString);
	char* dupString = new char[len];
	strcpy(dupString, inputString);
	char* tok = strtok(dupString, delimiter);
	tokens->push_back(tok);
	while (tok != NULL)
	{
		tok = strtok (NULL, " ,.-");
		tokens->push_back(tok);
	}
	return tokens;
}

char* GetMyIP() {
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
	char *googleIP = "74.125.225.115";

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

	int sd = connect(udpSocket, (struct sockaddr *)&address, sizeof(address));
	return NULL;
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


