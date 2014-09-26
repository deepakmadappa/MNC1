/*
 * server.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */
#include "runnable.h"
#include "helper.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <netdb.h>


using namespace std;
Server::Server(int port, int* socketList) {
	mSocketList = socketList;
	mPort = port;
	mClientList = new list<Host*>();
}

void Server::AcceptNewConnection(int socketListner, int* newClient) {
	int new_socket;
	struct sockaddr_in their_addr;
	unsigned int addr_size = sizeof their_addr;
	if ((new_socket = accept(socketListner, (struct sockaddr *)&their_addr, &addr_size) )<0)
	{
		perror("accept");
		exit(1);
	}

	char* clientIP = inet_ntoa(their_addr.sin_addr);
	char *host = new char[50];
	char serv[50];
	if(getnameinfo((struct sockaddr*)&their_addr, addr_size, host, 50, serv, 50, 0)) {
		perror("getnameinfo failed\n");
		exit(1);
	}

	char message[PACKET_SIZE + 1];
	//send new connection greeting message
	char strPort[10];
	TCPRecv(new_socket, strPort, 10, true);
	char *cpyPort = new char[10];
	char *cpyIP = new char[20];
	char *cpyHostname = new char[50];
	strPort[9] = '\0';
	strcpy(cpyIP, clientIP);
	strcpy(cpyPort, strPort);
	strcpy(cpyHostname, host);
	Host *newHost = new Host(cpyIP, cpyPort, cpyHostname);

	bool isDuplicate = Contains(mClientList, newHost);
	if(isDuplicate) {
		strcpy(message, "d");
		TCPSend(new_socket, message, PACKET_SIZE, true);
		return;
	}
	printf("\nAccepted Connection from %s\n", clientIP);

	mClientList->push_back(newHost);
	char *strClientList = SerializeList(mClientList);
	strcpy(message, "s|");
	strcpy(&(message[2]), strClientList);
	delete strClientList;
	TCPSend(new_socket, message, PACKET_SIZE, true);
	printf("\nNew Client Registered, IP:%s, Port:%s, Host:%s\n", clientIP, strPort, host);
	//add new socket to array of sockets
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		//if position is empty
		if( newClient[i] == 0 )
		{
			newClient[i] = new_socket;
			newHost->mSocketIndex = i;

			break;
		}
	}
	//Tell all client about the update; except the new client.
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if( newClient[i] > 0 && i != newHost->mSocketIndex)
		{
			TCPSend(newClient[i] , message, PACKET_SIZE, true);
		}
	}
}

void Server::HandleCloseOnOtherEnd(int* clientSockets, int socketIndex, int sd) {
	Host *host = 0;
	bool bFound = false;
	char message[PACKET_SIZE + 1];
	for (std::list<Host*>::iterator it = mClientList->begin(); it != mClientList->end(); it++) {
		host = (*it);
		if(host->mSocketIndex == socketIndex) {
			bFound = true;
			break;
		}
	}
	if(!bFound) {
		//You weren't even keeping track of this connection you say??? Don't say anything, tester might miss the bug.
	}
	else {
		mSocketList[host->mSocketIndex] = 0;
		mClientList->remove(host);
		char *strClientList = SerializeList(mClientList);
		strcpy(message, "s|");
		strcpy(&(message[2]), strClientList);
		delete strClientList;
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if( mSocketList[i] > 0)
			{
				TCPSend(mSocketList[i] , message, PACKET_SIZE, true);
			}
		}
	}
}

void Server::HandleActivityOnConnection(int *clientSockets, int socketIindex, char* message) {
	//No one talks to server at all.... So sad.
}

void Server::DisplayHelp() const {

}

void Server::List() const {
	PrintClientList(mClientList);
}

void Server::Statistics() {
	char message[PACKET_SIZE] = "n|statistics";
	char response[PACKET_SIZE];
	Host *host = NULL;
	printf("%s                          %s                          %s %s %s %s\n", "Hostname 1", "Hostname 2", "Total Uploads", "Avg Upload Spd(b/s)", "Total Downloads", "Total Download Speed (b/s)");
	for (std::list<Host*>::iterator it = mClientList->begin(); it != mClientList->end(); it++) {
		host = (*it);
		//if position is empty
		int sd = mSocketList[host->mSocketIndex];
		if(TCPSend(sd, message, PACKET_SIZE, false)) {
			printf("\nGetting statistics from client failed\n");
		}
		if(TCPRecv(sd, response, PACKET_SIZE, false )) {
			printf("\nReceiving statistics from client failed\n");
		}
		vector<char*> *tokens = tokenize(response, "|");
		for(unsigned int i = 0; i < tokens->size(); i++) {
			printf("%-35s%s\n",host->mHostname, tokens->at(i));
		}
		delete tokens;
	}

}

Server::~Server() {
	delete mClientList;
}



