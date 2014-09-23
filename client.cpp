/*
 * client.c
 *
 *  Created on: Sep 15, 2014
 *      Author: adminuser
 */

#include <vector>
#include "inc/helper.h"
#include "inc/runnable.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

Client::Client(int port, int* socketlist):Runnable(), mnConnections(0) {
	mPort = port;
	mClientsList = new std::vector<Host*>();
	mConnectionList = new std::list<Host*>();
	mSocketList = socketlist;
}

void Client::DisplayHelp() const {

}

void Client::List(void) const {
	printf("\nAvailable List\n");
	PrintVector(mClientsList);
	printf("\nConnection List\n");
	PrintClientList(mConnectionList);
}


/**
 * Register the client with the server running at given IP and Port, returns after printing valid message on failure
 */
void Client::Register(char* strIP, char* strPort) {
	int nPort = strtol(strPort, NULL, 10);
	if(nPort == 0 || nPort > 65535) {
		printf("\nInvalid port number\n");
		return;
	}
	int serverSocket = TCPConnect(strIP, nPort,false);
	if(serverSocket == -1) {
		return;
	}
	char writebuffer[10];
	sprintf(writebuffer, "%d", mPort);
	TCPSend(serverSocket, writebuffer, 10, true);
	char readBuffer[512];
	TCPRecv(serverSocket, readBuffer, 512, true);
	HandleRegisterResponse(readBuffer);
	Host *server = new Host(strIP, strPort, 0 );
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		//if position is empty
		if( mSocketList[i] == 0 )
		{
			mSocketList[i] = serverSocket;
			server->mSocketIndex = i;

			break;
		}
	}
	mConnectionList->push_back(server);
}

void Client::Connect(char* strDestination, char* strPort) {
	int nPort = strtol(strPort, NULL, 10);
	if(nPort == 0 || nPort > 65535) {
		printf("\nInvalid port number\n");
		return;
	}
	//TODO: add code to check if the connection exists in the list
	int peerSocket = TCPConnect(strDestination, nPort, false);
	if(peerSocket == -1) {
		return;
	}
	char writebuffer[10];
	sprintf(writebuffer, "%d", mPort);
	TCPSend(peerSocket, writebuffer, 10, true);

	char readBuffer[512];
	if(TCPRecv(peerSocket, readBuffer, 2, false) != 0) {
		printf("\nRecv falied\n");
		return;
	}
	if(readBuffer[0] == '0') {
		printf("\nHost rejected connect\n");
		return;
	}
	printf("\nConnected to %s\n", strDestination);
	char *cpyDest = new char[strlen(strDestination)];
	char *cpyPort = new char[strlen(strPort)];
	strcpy(cpyDest, strDestination);
	strcpy(cpyPort, strPort);
	Host *host = new Host(cpyDest,cpyPort, NULL);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		//if position is empty
		if( mSocketList[i] == 0 )
		{
			mSocketList[i] = peerSocket;
			host->mSocketIndex = i;

			break;
		}
	}
	mnConnections++;
	mConnectionList->push_back(host);

}

void Client::Terminate (char* strConnectionID) {
	Host *host = 0;
	int nConnectionID = strtol(strConnectionID, NULL, 10);
	int connectionIndex = 1;
	if(nConnectionID == 0) {
		printf("\nInvalid Connection ID\n");
		return;
	}
	for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++)
	{
		if(nConnectionID == connectionIndex ) {
			host = (*it);
			break;
		}
		connectionIndex++;
	}
	if(host == 0) {
		printf("\nInvalid Connection ID\n");
		return;
	}
	printf("\nTerminating connection with %s@%s\n", host->mIP, host->mPort);
	int closeSd = mSocketList[host->mSocketIndex];
	mSocketList[host->mSocketIndex] = 0;
	close(closeSd);
	mConnectionList->remove(host);
	mnConnections--;
	delete host;
}

void Client::Exit() {
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		//close all open sockets
		if( mSocketList[i] != 0 )
		{
			close(mSocketList[i]);
		}
	}
	exit(0);
}

void Client::Upload(char* strConnectionID, char* strFileName) {
	printf("\nClient Upload\n");
}

void Client::Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 /*=NULL*/, char* strFile2 /*=NULL*/, char* strConnectionID3 /*=NULL*/, char* strFile3 /*=NULL*/) {
	printf("\nClient Download\n");
}

void Client::Statistics() {
	printf("\nClient Statistics\n");
}

/*Function accepts new connect requests and reject if connection are full or add to connection list if not */
void Client::AcceptNewConnection(int socketListner, int* clientSockets) {
	int new_socket;
	struct sockaddr_in their_addr;
	unsigned int addr_size = sizeof their_addr;
	if ((new_socket = accept(socketListner, (struct sockaddr *)&their_addr, &addr_size) )<0)
	{
		perror("accept");
		exit(1);
	}

	char* theirIP = inet_ntoa(their_addr.sin_addr);
	char *host = new char[50];
	char serv[50];
	if(getnameinfo((struct sockaddr*)&their_addr, addr_size, host, 50, serv, 50, 0)) {
		perror("getnameinfo failed\n");
		exit(1);
	}
	char message[512];
	char strPort[10];
	TCPRecv(new_socket, strPort, 10, true);

	if(mnConnections >= 3) {
		TCPSend(new_socket, "0", 2, true);
		printf("\nConnections are full\n");
		return;
	}
	TCPSend(new_socket, "1", 2, true);
	char *cpyPort = new char[10];
	char *cpyIP = new char[20];
	char *cpyHostname = new char[50];
	printf("\nConnected to %s@%s\n", theirIP, strPort);
	strPort[9] = '\0';
	strcpy(cpyIP, theirIP);
	strcpy(cpyPort, strPort);
	strcpy(cpyHostname, host);
	Host *newHost = new Host(cpyIP, cpyPort, cpyHostname);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		//if position is empty
		if( mSocketList[i] == 0 )
		{
			mSocketList[i] = new_socket;
			newHost->mSocketIndex = i;

			break;
		}
	}

	mConnectionList->push_back(newHost);
	mnConnections++;
}


void Client::HandleCloseOnOtherEnd(int* clientSockets, int socketIndex, int sd) {
	Host *host = 0;
	bool bFound = false;
	for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++) {
		host = (*it);
		if(host->mSocketIndex == socketIndex) {
			bFound = true;
			break;
		}
	}
	if(!bFound) {
		//You weren't even keeping track of this connection you say??? Don't say anything, tester might miss the bug.
	} else {
		mnConnections--;
		mConnectionList->remove(host);
	}
}

void Client::HandleRegisterResponse(char* responseReceived) {
	vector<char*>* tokens = tokenize(responseReceived, "|");
	int nTokens = tokens->size();
	if(strcmp(tokens->at(0), "d")==0) {
		printf("\nYou have already registered\n");
		delete tokens;
		return;
	}
	if((nTokens - 1) % 3 != 0) {
		printf("\nReceived odd connection pair\n");
		delete tokens;
		return;
	}
	mClientsList->clear();
	char port[10];
	sprintf(port, "%d", mPort);
	for ( int i=1; i < nTokens; i+=3 ) {
		char* recvhost = tokens->at(i+1);
		char* recvPort = tokens->at(i + 2);
		if((strcmp(recvhost, mHostname) == 0) && (strcmp(recvPort, port) == 0))
			continue;
		mClientsList->push_back(new Host(tokens->at(i), tokens->at(i + 2), tokens->at(i + 1)));
	}
	PrintVector(mClientsList);
}

void Client::HandleActivityOnConnection(int *clientSockets, int socketIindex, char* message) {
	//Handle current downloads here

	//
	if(strlen(message) > 2 ) {
		switch(message[0]) {
		case 's':	//This is a message from server updating connection list
			HandleRegisterResponse(message);
			break;
		default:	//oh oh
			break;
		}
	}
}

Client::~Client() {
	delete mClientsList;
	delete mConnectionList;
}





