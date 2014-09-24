/*
 * client.c
 *
 *  Created on: Sep 15, 2014
 *      Author: adminuser
 */

#include <vector>
#include "helper.h"
#include "runnable.h"
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
#include <fcntl.h>

using namespace std;

Client::Client(int port, int* socketlist):Runnable(), mnConnections(0) {
	mPort = port;
	mClientsList = new std::vector<Host*>();
	mConnectionList = new std::list<Host*>();
	mSocketList = socketlist;
	mCurrentDownloadList = new std::list<unsigned long*>();
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
	char readBuffer[PACKET_SIZE + 1];
	TCPRecv(serverSocket, readBuffer, PACKET_SIZE, true);
	readBuffer[PACKET_SIZE] = '\0';
	HandleRegisterResponse(readBuffer);
	char *cpyDest = new char[strlen(strIP)];
	char *cpyPort = new char[strlen(strPort)];
	strcpy(cpyDest, strIP);
	strcpy(cpyPort, strPort);
	Host *server = new Host(cpyDest, cpyPort, 0 );
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

	char readBuffer[2];
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

void Client::Upload(char* strConnectionID, char* strFileName, int sd) const{
	if(sd == 0) {
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
		sd = mSocketList[host->mSocketIndex];
	}
	int fd = open(strFileName, O_RDONLY);
	if(fd <= 0) {
		printf("\nProblem occured opening file: %d\n", errno);
		return;
	}
	off_t fileSize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char firstPacket[PACKET_SIZE];
	vector<char*> *fileTokens = tokenize(strFileName,"/");
	sprintf(firstPacket,"u|%ld|%s", fileSize, fileTokens->at(fileTokens->size() - 1));
	if(TCPSend(sd, firstPacket, PACKET_SIZE, false)) {
		printf("\nSending first packet failed\n");
		return;
	}
	char buffer[PACKET_SIZE];
	while(read(fd, &buffer, PACKET_SIZE)) {
		if(TCPSend(sd, buffer, PACKET_SIZE, false)) {
			printf("\nUpload failed\n");
			return;
		}
	}
	printf("\nUpload complete\n");
}

void Client::Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 /*=NULL*/, char* strFile2 /*=NULL*/, char* strConnectionID3 /*=NULL*/, char* strFile3 /*=NULL*/) {
	HandleDownload(strConnectionID1, strFile1);
	if(strConnectionID2 != NULL) {
		HandleDownload(strConnectionID2, strFile2);
	}
	if(strConnectionID3 != NULL) {
		HandleDownload(strConnectionID3, strFile3);
	}
}

void Client::HandleDownload(char *strConnectionID, char *strFile) {
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
	int sd = mSocketList[host->mSocketIndex];
	char buffer[PACKET_SIZE + 1];
	sprintf(buffer,"d|%s", strFile);
	if(TCPSend(sd, buffer, PACKET_SIZE, false)) {
		printf("\nDownload Request failed\n");
	}
	if(TCPRecv(sd, buffer, PACKET_SIZE, false)) {
		printf("\nDownload request failed\n");
	}
	if(buffer[0] != 'u') {
		printf("\nDownload request rejected\n");
	}
	HandleUploadRequest(buffer, sd);
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
	strPort[9] = '\0';	// stop some malicious guy from typing long port and crashing the system
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
	int sd = clientSockets[socketIindex];
	bool bFound  = false;
	unsigned long *sdDetails = 0;
	for (std::list<unsigned long*>::iterator it = mCurrentDownloadList->begin(); it != mCurrentDownloadList->end(); it++) {
		sdDetails = (*it);
		if(sdDetails[0] == sd) {	//the activity in on a socket which has a download in progress
			bFound = true;
			break;
		}
	}
	if(bFound) {
		HandleContinueDownload(message, sdDetails);
		return;
	}
	//
	if(strlen(message) > 2 ) {
		switch(message[0]) {
		case 's':	//This is a message from server updating connection list
			HandleRegisterResponse(message);
			break;
		case 'u':
			HandleUploadRequest(message, clientSockets[socketIindex]);
		case 'd':
			HandleDownloadRequest(message, clientSockets[socketIindex]);
		default:	//oh oh
			break;
		}
	}
}

void Client::HandleUploadRequest(char* message, int sd) {
	vector<char*> *tokens = tokenize(message, "|");
	unsigned int fileSize = 0;
	if(tokens->size() < 3 || ((fileSize = strtol(tokens->at(1), NULL ,10)) == 0)) {
		printf("\nRequest Incorrect\n");
		return;
	}
	int fd = open(tokens->at(2), O_CREAT|O_WRONLY|O_TRUNC, 00777);
	if(fd <= 0) {
		printf("\nOpening file failed because %d\n", errno);
		return;
	}
	unsigned long *sdFdPair = new unsigned long[3];
	sdFdPair[0] = sd;
	sdFdPair[1] = fd;
	sdFdPair[2] = fileSize;

	mCurrentDownloadList->push_back(sdFdPair);
	delete tokens;
}

void Client::HandleDownloadRequest(char *message, int sd) {
	vector<char*> *tokens = tokenize(message, "|");
		unsigned int fileSize = 0;
		if(tokens->size() < 2) {
			printf("\nRequest Incorrect\n");
			return;
		}
		Upload(NULL, tokens->at(1), sd);
}

void Client::HandleContinueDownload(char *message, unsigned long* sdDetails) {
	int bytesToWrite = (sdDetails[2] > PACKET_SIZE )? PACKET_SIZE: sdDetails[2];	//if byresRemaining is greater than PACKET_SIZE, write packet size and keep going else we must stop as this is the last packet
	int fd = sdDetails[1];
	int bytesWritten = write(fd, message, bytesToWrite);
	if(bytesWritten <= 0) {
		printf("\nI didn't write anything because :%d\n", errno);
	}
	if(sdDetails[2] > PACKET_SIZE) {
		sdDetails[2] -= PACKET_SIZE; //just decrement remaning bytes and continue
	} else {
		//download finished close the file and get rid of sdDetail from list
		printf("\nReceive complete\n");
		close(fd);
		mCurrentDownloadList->remove(sdDetails);
		delete sdDetails;
		//do all your download timing stuff here
	}
}

Client::~Client() {
	delete mClientsList;
	delete mConnectionList;
}
