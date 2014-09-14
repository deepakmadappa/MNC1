/*
 * runnable.h
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

#include <vector>
#include <host.h>
#include <list>

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

class Runnable
{

public:
	void DisplayCreator(void) const;
	void DisplayHelp(void) const;
	void DisplayIP(void) const;
	void DisplayPort(void) const;
	virtual void List(void) const;

	virtual ~Runnable();
};

class Server: public Runnable
{
	std::list<Host*> *mClientList;
public:
	std::list<Host*>* AcceptRegister(Host* client);

	virtual Server();
	virtual ~Server();
};

class Client:public Runnable
{
	std::vector<Host*> mClientsList;
	std::list<Host*> mConnectionList;
};


#endif /* RUNNABLE_H_ */
