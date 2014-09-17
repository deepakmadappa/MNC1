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

using namespace std;

Client::Client(int port) {
	mPort = port;
	mClientsList = new std::vector<Host*>();
	mConnectionList = new std::list<Host*>();
}

void Client::DisplayHelp() const {

}

void Client::List(void) const {
	printf("Client LIST");
}

void Client::Register(char* strIP, char* strPort) {
	printf("\nClient Register\n");
}

void Client::Connect(char* strDestination, char* strPort) {
	printf("\nClient Connect\n");
}

void Client::Terminate (char* strConnectionID) {
	printf("\nClient Terminate\n");
}

void Client::Exit() {
	printf("\nClient Exit\n");
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

Client::~Client() {
	delete mClientsList;
	delete mConnectionList;
}





