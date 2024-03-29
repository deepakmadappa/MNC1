/*
 * helper.h
 *
 *  Created on: Sep 15, 2014
 *      Author: adminuser
 */



#ifndef HELPER_H_
#define HELPER_H_

#include <vector>
#include <list>
#include "host.h"

char* GetMyIP(char **outHostname);
std::vector<char*>* tokenize(char *inputString,const  char* delimiter);
char *ToUpper(char* input);
int TCPConnect(char* IP, int nPort, bool exitOnFail, char* outHostName = NULL, char* outIP = NULL);
int TCPRecv(int sd, char* readBuffer, int length, bool exitOnFail);
int TCPSend(int sd, const char* sendBuffer, int length, bool exitOnFail);
bool Contains(std::list<Host*>* clientList, Host* item);
char* SerializeList(std::list<Host*>* clientList);
void PrintClientList(std::list<Host*>* clientList);
void PrintVector(std::vector<Host*>* vector);

#endif /* HELPER_H_ */
