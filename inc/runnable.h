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

class Runnable
{
protected:
	int mPort;


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
	virtual void Upload(char* strConnectionID, char* strFileName);
	virtual void Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 = 0, char* strFile2 = 0, char* strConnectionID3 = 0, char* strFile3 = 0);
	virtual void Statistics();
	/*Public Commands End*/
	virtual ~Runnable();
};

class Server: public Runnable
{
	std::list<Host*> *mClientList;
	std::list<Host*>* AcceptRegister(Host* client);

public:
	Server(int port);

	void List(void) const;
	virtual void DisplayHelp() const;
	virtual ~Server();
};

class Client:public Runnable
{
	std::vector<Host*>* mClientsList;
	std::list<Host*>* mConnectionList;


public:
	Client(int port);

	void List(void) const;
	virtual void DisplayHelp() const;
	virtual void Register(char* strIP, char* strPort);
	virtual void Connect(char* strDestination, char* strPort);
	virtual void Terminate (char* strConnectionID);
	virtual void Exit();
	virtual void Upload(char* strConnectionID, char* strFileName);
	virtual void Download(char* strConnectionID1, char* strFile1, char* strConnectionID2 = 0, char* strFile2 = 0, char* strConnectionID3 = 0, char* strFile3 = 0);
	virtual void Statistics();

	virtual ~Client();
};


#endif /* RUNNABLE_H_ */
