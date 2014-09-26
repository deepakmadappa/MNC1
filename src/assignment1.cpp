//============================================================================
// Name        : assignment1.cpp
// Author      : dmadappa
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================
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

#include "runnable.h"
#include "host.h"
#include "helper.h"

void PrintUsage();
void HandleUserInput(Runnable*, char*);
using namespace std;


int main(int argc, char* argv[]) {
	bool bIsServerMode = true;
	Runnable *thisProgram = NULL;
	//GetMyIP();
	if(argc != 3) {
		PrintUsage();
	}

	//First arg must be of length 1 and can only be s or c
	if(strlen(argv[1]) != 1) {
		PrintUsage();
	}
	else {
		if(argv[1][0] == 's') {
			bIsServerMode = true;
		} else if (argv[1][0] == 'c') {
			bIsServerMode = false;
		} else {
			PrintUsage();
		}
	}

	int port = strtol(argv[2], NULL, 10);
	if(port == 0) {
		PrintUsage();
	}

	int sockListenNew , addrlen , client_socket[MAX_CLIENTS];

	if(bIsServerMode)
		thisProgram = new Server(port, client_socket);
	else
		thisProgram = new Client(port, client_socket);

	/* Skeleton code to initialize socket and Select copied from https://gist.github.com/silv3rm00n/5604330*/
	int opt = 1;

	int max_sd;
	struct sockaddr_in address;

	//set of socket descriptors
	fd_set readfds;

	//initialise all client_socket[] to 0 so not checked
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		client_socket[i] = 0;
	}

	//create a master socket
	if( (sockListenNew = socket(AF_INET , SOCK_STREAM , 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if( setsockopt(sockListenNew, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );

	//bind the socket to localhost port 8888
	if (bind(sockListenNew, (struct sockaddr *)&address, sizeof(address))<0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", port);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(sockListenNew, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);

	while(true) {
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(sockListenNew, &readfds);
		FD_SET(STDIN, &readfds);
		max_sd = sockListenNew;

		//add child sockets to set
		for (int i = 0 ; i < MAX_CLIENTS ; i++)
		{
			//socket descriptor
			int sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);

			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}

		//printf(">>");
		//fflush(stdout);
		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
		int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}
		/* Code copied from https://gist.github.com/silv3rm00n/5604330 ends*/
		if(FD_ISSET(STDIN, &readfds)) {
			char buffer[50];

			int n = read(STDIN, buffer, 50);
			buffer[n - 1] = '\0'; //replaceing \n with \0 at the end of string
			HandleUserInput(thisProgram, buffer);
			FD_CLR(STDIN, &readfds);
		}
		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(sockListenNew, &readfds))
		{
			thisProgram->AcceptNewConnection(sockListenNew, client_socket);
		}

		//else its some IO operation on some other socket :)
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			int sd = client_socket[i];
			char message[PACKET_SIZE + 1];
			int bytesRead=0;
			if (FD_ISSET(sd , &readfds))
			{
				//struct timeval startTime, endTime, *timeTakenForThisPacket = new struct timeval();
				bytesRead = read( sd , message, PACKET_SIZE);

				//timeTakenForThisPacket->tv_sec = endTime.tv_sec - startTime.tv_sec;
				//timeTakenForThisPacket->tv_usec = endTime.tv_usec - startTime.tv_usec;
				//Check if it was for closing , and also read the incoming message
				if (bytesRead == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s\n" , inet_ntoa(address.sin_addr));

					thisProgram->HandleCloseOnOtherEnd(client_socket, i, sd);
					close( sd );
					client_socket[i] = 0;
				}

				//Echo back the message that came in
				else
				{
					//set the string terminating NULL byte on the end of the data read
					message[bytesRead] = '\0';
					thisProgram->HandleActivityOnConnection(client_socket, i, message);
					//send(sd , message , strlen(message) , 0 );
				}

			}
		}
	}
	return EXIT_SUCCESS;
}

void PrintUsage() {
	printf("USAGE: assigment1 <s/c> <port>\n");
	exit(1);
}

void HandleUserInput(Runnable *thisProgram, char* userInput) {
	char *delim = (char*)" ";
	vector<char*>* tokens = tokenize(userInput, delim);
	int inputSize = tokens->size();
	if(inputSize < 1) {
		printf("\nInvalid command or file name ;)\n");
		return;
	}

	char *commandName = ToUpper(tokens->at(0));

	if(strcmp (commandName, "CREATOR") == 0 ) {
		thisProgram->DisplayCreator();
	} else if(strcmp (commandName, "HELP") == 0 ) {
		thisProgram->DisplayHelp();
	} else if (strcmp (commandName, "MYIP") == 0 ) {
		thisProgram->DisplayIP();
	} else if(strcmp (commandName, "MYPORT") == 0 ) {
		thisProgram->DisplayPort();
	} else if (strcmp (commandName, "REGISTER") == 0) {
		if(inputSize != 3) {
			printf("\nUsage: REGISTER <Server IP> <Server Port>");
			return;
		}
		thisProgram->Register(tokens->at(1), tokens->at(2));
	} else if (strcmp (commandName, "CONNECT") == 0) {
		if(inputSize != 3) {
			printf("\nUsage: CONNECT <Destination> <Port>\n");
			return;
		}
		thisProgram->Connect(tokens->at(1), tokens->at(2));
	} else if(strcmp (commandName, "LIST") == 0 ) {
		//Handle LIST Command
		thisProgram->List();
	} else if (strcmp (commandName, "TERMINATE") == 0) {
		if(inputSize != 2) {
			printf("\nUsage: TERMINATE <Connection ID>\n");
			return;
		}
		thisProgram->Terminate(tokens->at(1));
	} else if(strcmp (commandName, "EXIT") == 0 ) {
		//Handle LIST Command
		thisProgram->Exit();
	} else if (strcmp (commandName, "UPLOAD") == 0) {
		if(inputSize != 3) {
			printf("\nUsage: UPLOAD <Connection ID> <File Name>\n");
			return;
		}
		thisProgram->Upload(tokens->at(1), tokens->at(2), 0);
	} else if (strcmp (commandName, "DOWNLOAD") == 0) {
		if(inputSize < 3) {
			printf("\nUsage: DOWNLOAD <Connection ID 1> <File Name 1> [<Connection ID 2> <File Name 2>] [<Connection ID 3> <File Name 3>]\nFile names cannot have spaces in them\n");
			return;
		}
		if(inputSize == 3) {
			thisProgram->Download(tokens->at(1), tokens->at(2));
		} else if(inputSize == 5) {
			thisProgram->Download(tokens->at(1), tokens->at(2), tokens->at(3), tokens->at(4));
		} else if(inputSize == 7) {
			thisProgram->Download(tokens->at(1), tokens->at(2), tokens->at(3), tokens->at(4), tokens->at(5), tokens->at(6));
		}
		else {
			printf("\nUsage: DOWNLOAD <Connection ID 1> <File Name 1> [<Connection ID 2> <File Name 2>] [<Connection ID 3> <File Name 3>]\nFile names cannot have spaces in them\n");
		}
	} else if(strcmp (commandName, "STATISTICS") == 0 ) {
		//Handle LIST Command
		thisProgram->Statistics();
	} else {
		printf("\nInvalid Command\n");
	}
}

