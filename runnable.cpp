/*
 * runnable.cpp
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

#include "inc/runnable.h"
#include <stdio.h>

void Runnable::DisplayCreator(void) const {
	printf("Deepak Madappa, dmadappa, dmadappa@buffalo.edu\n");
	printf("I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
}

void Runnable::DisplayHelp(void) const {

}

void Runnable::DisplayIP(void) const {
	printf("\nCommonIP\n");
}

void Runnable::DisplayPort(void) const {
	printf("\nPort: %d\n", mPort);
}

void Runnable::List(void) const {
	printf("hello");
}

void Runnable::Register(char* strIP, char* strPort) {
	printf("\nInvalid Command\n");
}

void Runnable::Connect(char* strDestination, char* strPort) {
	printf("\nInvalid Command\n");
}

void Runnable::Terminate (char* strConnectionID) {
	printf("\nInvalid Command\n");
}

void Runnable::Exit() {
	printf("\nInvalid Command\n");
}

void Runnable::Upload(char* strConnectionID, char* strFileName) {
	printf("\nInvalid Command\n");
}

void Runnable::Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 /*=NULL*/, char* strFile2 /*=NULL*/, char* strConnectionID3 /*=NULL*/, char* strFile3 /*=NULL*/) {
	printf("\nInvalid Command\n");
}

void Runnable::Statistics() {
	printf("\nInvalid Command\n");
}

Runnable::~Runnable() {

}


