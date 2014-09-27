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
	mDisconnectedHostList = new std::list<Host*>();
}

void Client::DisplayHelp() const {
	//writing printf statements :(
	printf("CREATOR: displays disclaimer\n\
	HELP: Deja vu\n\
	MYIP: Displays my ip\n\
	MYPORT: Displayes my port\n\
	REGISTER <server IP> <port_no>: Registers with server\n\
	CONNECT <destination> <port no>: Connects to peer\n\
	LIST: Lists all the connections\n\
	TERMINATE <connection id>: Terminates a connection\n\
	EXIT: Terminates all connections and dies, good bye cruel world\n\
	UPLOAD <connection id> <file name>: uploads the file to corresponding connection\n\
	DOWNLOAD <connection id 1> <file1> <connection id 2> <file2> <connection id 3> <file3>: Downloads files from corresponding connections\n\
	STATISTICS: Diplays preious transfer information\n");

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

	if(((strcmp(strIP, mIP) == 0) || (strcmp(strIP, mHostname) == 0)) && nPort == mPort) {
		printf("\nStop trying to talk to youself\n");
		return;
	}
	char *hostName = new char[100];
	char *hostIP = new char[15];
	int serverSocket = TCPConnect(strIP, nPort, false, hostName, hostIP);
	if(serverSocket == -1) {
		printf("\nUnable to connect to the destination, please verify the details\n");
		return;
	}
	char writebuffer[10];
	sprintf(writebuffer, "%d", mPort);
	TCPSend(serverSocket, writebuffer, 10, true);
	char readBuffer[PACKET_SIZE + 1];
	TCPRecv(serverSocket, readBuffer, PACKET_SIZE, true);
	readBuffer[PACKET_SIZE] = '\0';
	if(HandleRegisterResponse(readBuffer))
		return;
	char *cpyPort = new char[strlen(strPort)];
	strcpy(cpyPort, strPort);
	Host *server = new Host(hostIP, cpyPort, hostName);
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
	mConnectionList->push_front(server);
}

void Client::Connect(char* strDestination, char* strPort) {
	int nPort = strtol(strPort, NULL, 10);
	if(nPort == 0 || nPort > 65535) {
		printf("\nInvalid port number\n");
		return;
	}
	if(((strcmp(strDestination, mIP) == 0) || (strcmp(strDestination, mHostname) == 0)) && nPort == mPort) {
		printf("\nStop trying to talk to youself\n");
		return;
	}
	Host *h = NULL;
	for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++)
	{
		h = (*it);
		if(((strcmp(strDestination, h->mIP) == 0) || (strcmp(strDestination, h->mHostname) == 0)) && strcmp(strPort, h->mPort) == 0) {
			printf("\nWe are already connected, stop being needy\n");
			return;
		}
	}

	//check if the connection exists in the list

	bool bExistsInList = false;
	for (int i=0; i < mClientsList->size(); i++)
	{
		h = mClientsList->at(i);
		if(((strcmp(strDestination, h->mIP) ==0) || (strcmp(strDestination, h->mHostname) == 0)) && strcmp(strPort, h->mPort)==0) {
			bExistsInList = true;
			break;
		}
	}
	if(!bExistsInList) {
		printf("\nThe host doesn't exist int he ServerList, please verify the details again\n");
		return;
	}

	char *hostName = new char[100];
	char *hostIP = new char[15];
	int peerSocket = TCPConnect(strDestination, nPort, false, hostName, hostIP);
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
	char *cpyPort = new char[strlen(strPort)];
	strcpy(cpyPort, strPort);
	Host *host = new Host(hostIP,cpyPort, hostName);
	Host *disconnectedHost = NULL;
	if( (disconnectedHost = FindPreviousConnection(host)) != NULL) {
		delete host;
		host = disconnectedHost;
	}

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
	mDisconnectedHostList->push_back(host);
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

/**
 * Reusing the same function for both upload and download, when called for upload strConnectionID will have value and sd = 0
 * when called from download it'll be other way
 * The application is going to block on this function till the whole file is transferred in both cases
 */
void Client::Upload(char* strConnectionID, char* strFileName, int sd) const{
	Host *host = 0;

	if(sd == 0) {
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
	struct timeval start, end;
	if(gettimeofday(&start, NULL) == -1) {
		perror("Getting time of day failed");
		exit(1);
	}
	int fd = open(strFileName, O_RDONLY);
	if(fd <= 0) {
		printf("\nProblem occured opening file: %d\n", errno);
		if(sd != 0) {
			char msg[PACKET_SIZE] = "f";
			TCPSend(sd, msg, PACKET_SIZE, false);
		}
		return;
	}
	off_t fileSize = lseek(fd, 0, SEEK_END);
	printf("\nSending file: %s\n", strFileName);
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
			printf("\nTransfer failed\n");
			return;
		}
	}
	if(gettimeofday(&end, NULL) == -1) {
		perror("Getting time of day failed");
		exit(1);
	}
	if(host == NULL) {
		for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++) {
			host = (*it);
			if(mSocketList[host->mSocketIndex] == sd)
				break;
		}
	}
	unsigned long totalTime = (end.tv_sec - start.tv_sec) * 1000 + ((end.tv_usec - start.tv_usec)/1000);
	//unsigned long rate = (fileSize * 8 * 1000) / ((totalTime == 0)?1:totalTime );	//we dont want divide by zero now
	double totalTimeInSec = (double)totalTime / (double)1000.0;
	double rate = ((double)(fileSize * 8))/((totalTimeInSec == 0)?1:totalTimeInSec);
	printf("\nTx : %s -> %s, File Size: %lu Bytes, Time Taken: %f seconds, Tx Rate: %f bits/second\n", this->mHostname, host->mHostname, fileSize, totalTimeInSec, rate);

	//add details to maintain statistics
	host->mTotalBytesUploaded += fileSize;
	host->mTotalSendTime += totalTime;
	host->mnUploads++;


}

void Client::Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 /*=NULL*/, char* strFile2 /*=NULL*/, char* strConnectionID3 /*=NULL*/, char* strFile3 /*=NULL*/) {
	HandleDownloadCommandFromUser(strConnectionID1, strFile1);
	if(strConnectionID2 != NULL) {
		HandleDownloadCommandFromUser(strConnectionID2, strFile2);
	}
	if(strConnectionID3 != NULL) {
		HandleDownloadCommandFromUser(strConnectionID3, strFile3);
	}
}

void Client::HandleDownloadCommandFromUser(char *strConnectionID, char *strFile) {
	Host *host = 0;
	int nConnectionID = strtol(strConnectionID, NULL, 10);
	int connectionIndex = 1;
	if(nConnectionID == 0 || nConnectionID == 1) {
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
		return;
	}
	if(TCPRecv(sd, buffer, PACKET_SIZE, false)) {
		printf("\nDownload request failed\n");
		return;
	}
	if(buffer[0] != 'u') {
		printf("\nDownload request rejected, please check the file name\n");
		return;
	}
	printf("\nDownloading file: %s from: %s\n", strFile, host->mHostname);
	HandleTransferRequest(buffer, sd);
}

void Client::Statistics() {
	printf("\nClient Statistics\n");
	Host *host = NULL;
	printf("%s                          %s %s %s %s\n", "Host Name", "Total Uploads", "Avg Upload Spd(b/s)", "Total Downloads", "Total Download Speed (b/s)");
	for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++) {
		host = (*it);
		//if(connectionIndex == 1)
		//	continue; //skip the server
		if(host->mnDownloads != 0 || host->mnUploads != 0) {
			double totalUploadTimeinSec = (double)(host->mTotalSendTime) / (double)1000.0;
			double urate = ((double)(host->mTotalBytesUploaded * 8))/((totalUploadTimeinSec == 0)?1:totalUploadTimeinSec);
			double totalDownloadTimeinSec = (double)(host->mTotalReceiveTime) / (double)1000.0;
			double drate = ((double)(host->mTotalBytesDownloaded * 8))/((totalDownloadTimeinSec == 0)?1:totalDownloadTimeinSec);
			printf("%-35s %14d %12.3f %17d %12.3f\n", host->mHostname, host->mnUploads, urate, host->mnDownloads, drate);
		}
	}

	for (std::list<Host*>::iterator it = mDisconnectedHostList->begin(); it != mDisconnectedHostList->end(); it++) {
		host = (*it);
		//if(connectionIndex == 1)
		//	continue; //skip the server
		if(host->mnDownloads != 0 || host->mnUploads != 0) {
			double totalUploadTimeinSec = (double)(host->mTotalSendTime) / (double)1000.0;
			double urate = ((double)(host->mTotalBytesUploaded * 8))/((totalUploadTimeinSec == 0)?1:totalUploadTimeinSec);
			double totalDownloadTimeinSec = (double)(host->mTotalReceiveTime) / (double)1000.0;
			double drate = ((double)(host->mTotalBytesDownloaded * 8))/((totalDownloadTimeinSec == 0)?1:totalDownloadTimeinSec);
			printf("%-35s %14d %12.3f %17d %12.3f\n", host->mHostname, host->mnUploads, urate, host->mnDownloads, drate);
		}
	}
}


/*Function accepts new connect requests and reject if connection are full or add to connection list if not */
//NOTE Register command on self still blocks
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
	char myPort[10];
	strPort[9] = '\0';	// stop some malicious guy from typing long port and crashing the system
	sprintf(myPort, "%d", mPort);

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
	strcpy(cpyIP, theirIP);
	strcpy(cpyPort, strPort);
	strcpy(cpyHostname, host);
	Host *newHost = new Host(cpyIP, cpyPort, cpyHostname);
	Host *disconnectedHost = NULL;
	if( (disconnectedHost = FindPreviousConnection(newHost)) != NULL) {
		delete newHost;
		newHost = disconnectedHost;
	}
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
		mDisconnectedHostList->push_back(host);
	}
}

int Client::HandleRegisterResponse(char* responseReceived) {
	vector<char*>* tokens = tokenize(responseReceived, "|");
	int nTokens = tokens->size();
	if(strcmp(tokens->at(0), "d")==0) {
		printf("\nYou have already registered\n");
		delete tokens;
		return 1;
	}
	if((nTokens - 1) % 3 != 0) {
		printf("\nReceived odd connection pair\n");
		delete tokens;
		return 1;
	}
	mClientsList->clear();
	char port[10];
	sprintf(port, "%d", mPort);
	for ( int i=1; i < nTokens; i+=3 ) {
		char* recvhost = tokens->at(i+1);
		char* recvPort = tokens->at(i + 2);
		if((strcmp(recvhost, mHostname) == 0 || strcmp(tokens->at(i), mIP) == 0) && (strcmp(recvPort, port) == 0))
			continue;	//this is your own details skip it
		mClientsList->push_back(new Host(tokens->at(i), tokens->at(i + 2), tokens->at(i + 1)));
	}
	printf("\nReceived a list from server");
	PrintVector(mClientsList);
	return 0;
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
		HandlePacketOnSocketWithOngoingTransfer(message, sdDetails, socketIindex);
		return;
	}
	//
	if(strlen(message) > 2 ) {
		switch(message[0]) {
		case 's':	//This is a message from server updating connection list
			HandleRegisterResponse(message);
			break;
		case 'u':
			printf("\nPreparing to receive file\n");
			HandleTransferRequest(message, clientSockets[socketIindex]);
			break;
		case 'd':
			HandleDownloadRequestFromOtherSide(message, clientSockets[socketIindex]);
			break;
		case 'n':
			HandleStatisticsRequestFromServer(clientSockets[socketIindex]);
			break;
		default:	//oh oh
			break;
		}
	}
}

void Client::HandleStatisticsRequestFromServer(int sd) {
	Host *host = NULL;
	int connectionIndex = 1;
	char returnMessage[PACKET_SIZE] = {'\0'};
	printf("received stat request\n");
	for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++) {
		host = (*it);
		if(connectionIndex == 1) {
			connectionIndex++;
			continue; //ignore server
		}
		if(host->mnDownloads != 0 || host->mnUploads !=0) {
			double totalUploadTimeinSec = (double)(host->mTotalSendTime) / (double)1000.0;
			double urate = ((double)(host->mTotalBytesUploaded * 8))/((totalUploadTimeinSec == 0)?1:totalUploadTimeinSec);
			double totalDownloadTimeinSec = (double)(host->mTotalReceiveTime) / (double)1000.0;
			double drate = ((double)(host->mTotalBytesDownloaded * 8))/((totalDownloadTimeinSec == 0)?1:totalDownloadTimeinSec);
			sprintf(returnMessage, "%s|%-35s %14d %12.3f %17d %12.3f", returnMessage, host->mHostname, host->mnUploads, urate, host->mnDownloads, drate);
		}
	}
	for (std::list<Host*>::iterator it = mDisconnectedHostList->begin(); it != mDisconnectedHostList->end(); it++) {
		host = (*it);
		if(host->mnDownloads != 0 || host->mnUploads !=0) {
			double totalUploadTimeinSec = (double)(host->mTotalSendTime) / (double)1000.0;
			double urate = ((double)(host->mTotalBytesUploaded * 8))/((totalUploadTimeinSec == 0)?1:totalUploadTimeinSec);
			double totalDownloadTimeinSec = (double)(host->mTotalReceiveTime) / (double)1000.0;
			double drate = ((double)(host->mTotalBytesDownloaded * 8))/((totalDownloadTimeinSec == 0)?1:totalDownloadTimeinSec);
			sprintf(returnMessage, "%s|%-35s %14d %12.3f %17d %12.3f", returnMessage, host->mHostname, host->mnUploads, urate, host->mnDownloads, drate);
		}
	}
	if(TCPSend(sd, returnMessage, PACKET_SIZE, false)) {
		printf("Sending Statistics to server Failed\n");
		return;
	}
}

void Client::HandleTransferRequest(char* message, int sd) {
	vector<char*> *tokens = tokenize(message, "|");
	unsigned int fileSize = 0;
	if(tokens->size() < 3 || ((fileSize = strtol(tokens->at(1), NULL ,10)) == 0)) {
		printf("\nRequest Incorrect\n");
		return;
	}
	struct timeval *time= new struct timeval();
	if(gettimeofday(time, NULL) == -1) {
		perror("Getting time of day failed");
		exit(1);
	}
	int fd = open(tokens->at(2), O_CREAT|O_WRONLY|O_TRUNC, 00777);
	if(fd <= 0) {
		printf("\nOpening file failed because %d\n", errno);
		return;
	}
	char *fileName = new char[strlen(tokens->at(2)) + 1];
	strcpy(fileName, tokens->at(2));

	unsigned long *socketDownloadDetails = new unsigned long[6];
	socketDownloadDetails[0] = sd;
	socketDownloadDetails[1] = fd;
	socketDownloadDetails[2] = fileSize; //for keeping remaining size
	socketDownloadDetails[3] = fileSize;	//for keeping total size
	socketDownloadDetails[4] = (unsigned long) fileName;	//storing the address of fileName pointer. A bit dodgy way of doing things but it works, I don't want to re write the whole data structure.
	socketDownloadDetails[5] = (unsigned long) time;
	g_bCurrentlyDownloading = true;
	mCurrentDownloadList->push_back(socketDownloadDetails);
	delete tokens;
}

void Client::HandleDownloadRequestFromOtherSide(char *message, int sd) {
	vector<char*> *tokens = tokenize(message, "|");
	if(tokens->size() < 2) {
		printf("\nRequest Incorrect\n");
		return;
	}
	Upload(NULL, tokens->at(1), sd);
}

void Client::HandlePacketOnSocketWithOngoingTransfer(char *message, unsigned long* socketDownloadDetails, int socketIndex) {
	int bytesToWrite = (socketDownloadDetails[2] > PACKET_SIZE )? PACKET_SIZE: socketDownloadDetails[2];	//if byresRemaining is greater than PACKET_SIZE, write packet size and keep going else we must stop as this is the last packet
	int fd = socketDownloadDetails[1];
	struct timeval end, *start;

	int bytesWritten = write(fd, message, bytesToWrite);


	if(bytesWritten <= 0) {
		printf("\nI didn't write anything because :%d\n", errno);
	}
	if(socketDownloadDetails[2] > PACKET_SIZE) {
		socketDownloadDetails[2] -= PACKET_SIZE; //just decrement remaning bytes and continue
	} else {
		//last packet has arrived, wrap up the transfer
		Host *host = NULL;
		for (std::list<Host*>::iterator it = mConnectionList->begin(); it != mConnectionList->end(); it++) {
			host = (*it);
			if(host->mSocketIndex == socketIndex)
				break;
		}
		//download finished close the file and get rid of sdDetail from list
		unsigned long fileSize = socketDownloadDetails[3];
		printf("File: %s transfer complete", (char*)socketDownloadDetails[4]);
		start = (struct timeval*) socketDownloadDetails[5];
		if(gettimeofday(&end, NULL) == -1) {
			perror("Getting time of day failed");
			exit(1);
		}
		unsigned long totalTime = (end.tv_sec - start->tv_sec) * 1000 + ((end.tv_usec - start->tv_usec)/1000);
		//unsigned long rate = (fileSize * 8 * 1000) / ((totalTime == 0)?1:totalTime );	//we dont want divide by zero now
		double totalTimeInSec = (double)totalTime / (double)1000.0;
		double rate = ((double)(fileSize * 8))/((totalTimeInSec == 0)?1:totalTimeInSec);
		printf("\nRx : %s -> %s, File Size: %lu Bytes, Time Taken: %f seconds, Tx Rate: %f bits/second\n", host->mHostname, this->mHostname, fileSize, totalTimeInSec, rate);
		close(fd);
		mCurrentDownloadList->remove(socketDownloadDetails);
		delete (char*)socketDownloadDetails[4];
		delete (struct timeval*) socketDownloadDetails[5];
		delete socketDownloadDetails;

		//Update host details for STATISTICS
		host->mnDownloads++;
		host->mTotalBytesDownloaded += fileSize;
		host->mTotalReceiveTime += totalTime;

		if(mCurrentDownloadList->empty()) {
			g_bCurrentlyDownloading = false;
		}
	}
}

Host* Client::FindPreviousConnection(Host* prevHost) {
	Host *host = NULL;
	for (std::list<Host*>::iterator it = mDisconnectedHostList->begin(); it != mDisconnectedHostList->end(); it++) {
		host = (*it);
		if(host->IsEqual(prevHost)) {

			break;
		}

	}
	if(host) {
		mDisconnectedHostList->remove(host);
	}
	return host;
}

Client::~Client() {
	delete mClientsList;
	delete mConnectionList;
	delete mDisconnectedHostList;
}
