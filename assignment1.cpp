//============================================================================
// Name        : assignment1.cpp
// Author      : dmadappa
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
int strlen(char *str);
void PrintUsage();
using namespace std;
const int STDIN = 0;


int main(int argc, char* argv[]) {
	bool bIsServerMode = true;
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

	/* Code copied from https://gist.github.com/silv3rm00n/5604330 begins*/
    int opt = 1;
    int sockListenNew , addrlen , new_socket , client_socket[4] , max_clients = 4 , activity, i , valread , sd;
	int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K
    //set of socket descriptors
    fd_set readfds;

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (sockListenNew = socket(AF_INET , SOCK_STREAM , 0)) == 0)
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
    if (bind(sockListenNew, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
	printf("Listener on port %d \n", port);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(sockListenNew, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);

	while(true)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(sockListenNew, &readfds);
        FD_SET(STDIN, &readfds);
        max_sd = sockListenNew;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
				max_sd = sd;
        }

        printf("waiting on select\n");
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        printf("Activity on select\n");
        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        if(FD_ISSET(STDIN, &readfds)) {
        	printf("Input on keyboard");
        }
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(sockListenNew, &readfds))
        {
            if ((new_socket = accept(sockListenNew, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            char message[6] = "Hello";
            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);

					break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }



	/* Code copied from https://gist.github.com/silv3rm00n/5604330 ends*/

	return EXIT_SUCCESS;
}

void PrintUsage() {
	printf("USAGE: Project1.exe <s/c> <port>\n");
	exit(1);
}

int strlen(char *str) {
	int length = 0;
	while(str[length] != '\0')
		length ++;
	return length;
}
