/*
 * server.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */
#include "inc/runnable.h"
#include "inc/helper.h"
#include <string.h>
#include <stdio.h>


using namespace std;
Server::Server(int port) {
	mPort = port;
	mClientList = new list<Host*>();
}

list<Host*>* Server::AcceptRegister(Host* newClient) {
	mClientList->push_back(newClient);
	return mClientList;
}
/*
//Tokenizes user command and takes corresponding action
void Server::HandleUserInput(char *userCommand) {
	vector<char*>* tokens = tokenize(userCommand, "|\n");
	char *commandName = ToUpper(tokens->at(0));

	if(strcmp (commandName, "LIST") == 0 ) {
		//Handle LIST Command
		List();
	}

}*/

void Server::List() const {
	printf("Server List");
}

Server::~Server() {
	delete mClientList;
}



