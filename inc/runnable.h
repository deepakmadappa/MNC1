/*
 * runnable.h
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

#include <vector>
#include "host.h"
#include <list>

#ifndef RUNNABLE_H_
#define RUNNABLE_H_


const int STDIN = 0;
const int MAX_CLIENTS = 4;
const int PACKET_SIZE = 512;

class Runnable
{
protected:
	int mPort;
	char *mIP;
	char *mHostname;
protected:
	Runnable();
public:
	/*All Public Commands*/
	virtual void DisplayCreator() const;
	virtual void DisplayHelp() const;
	virtual void DisplayIP() const;
	virtual void DisplayPort() const;
	virtual void List() const;
	virtual void Register(char* strIP, char* strPort);
	virtual void Connect(char* strDestination, char* strPort);
	virtual void Terminate (char* strConnectionID);
	virtual void Exit();
	virtual void Upload(char* strConnectionID, char* strFileName, int sd) const;
	virtual void Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 = 0, char* strFile2 = 0, char* strConnectionID3 = 0, char* strFile3 = 0);
	virtual void Statistics();
	/*Public Commands End*/

	virtual void AcceptNewConnection(int socketListner, int* clientSockets) = 0;
	virtual void HandleCloseOnOtherEnd(int* clientSockets, int socketIndex, int sd) = 0;
	virtual void HandleActivityOnConnection(int *clientSockets, int socketIindex, char* message) = 0;

	virtual ~Runnable();
};

class Server: public Runnable
{
	std::list<Host*> *mClientList;

public:
	Server(int port);

	void List(void) const;
	virtual void DisplayHelp() const;

	virtual void AcceptNewConnection(int socketListner, int* clientSockets);
	virtual void HandleCloseOnOtherEnd(int* clientSockets, int socketIndex, int sd);
	virtual void HandleActivityOnConnection(int *clientSockets, int socketIindex, char* message);
	virtual ~Server();
};

class Client:public Runnable
{
	std::vector<Host*>* mClientsList;
	std::list<Host*>* mConnectionList;
	std::list<unsigned long*>* mCurrentDownloadList;	//Holds an array of sd, fd, and remaining file size
	int *mSocketList;
	int mnConnections;

public:
	Client(int port, int*);

	void List(void) const;
	virtual void DisplayHelp() const;
	virtual void Register(char* strIP, char* strPort);
	virtual void Connect(char* strDestination, char* strPort);
	virtual void Terminate (char* strConnectionID);
	virtual void Exit();
	virtual void Upload(char* strConnectionID, char* strFileName, int sd) const;
	virtual void Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 = 0, char* strFile2 = 0, char* strConnectionID3 = 0, char* strFile3 = 0);
	virtual void Statistics();

	virtual void AcceptNewConnection(int socketListner, int* clientSockets);
	virtual void HandleCloseOnOtherEnd(int* clientSockets, int socketIndex, int sd);
	virtual void HandleActivityOnConnection(int *clientSockets, int socketIindex, char* message);
	void HandleRegisterResponse(char*);
	void HandleUploadRequest(char* message, int sd);
	void HandleContinueDownload(char* message, unsigned long* sdDetails);
	void HandleDownload(char *strConnection, char *strFile);
	void HandleDownloadRequest(char *message, int sd);
	virtual ~Client();
};


#endif /* RUNNABLE_H_ */
