/*
 * server.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

using namespace std;
virtual Server::Server() {
	mClientList = new list<Host*>();
}

list<Host*>* Server::AcceptRegister(Host* newClient) {
	mClientList->push_back(newClient);
	return mClientList;
}

virtual Server::~Server() {
	delete mClientList;
}



