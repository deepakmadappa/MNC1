/*
 * host.h
 *
 *  Created on: Sep 14, 2014
 *      Author: adminuser
 */

#ifndef HOST_H_
#define HOST_H_

#include<string.h>

class Host
{
public:
	char* mIP;
	char* mPort;
	char* mHostname;
	int mSocketIndex;

	Host(char* ip, char* port, char *hostname): mSocketIndex(0), mIP(ip), mPort(port),mHostname(hostname) {

	}

	bool IsEqual(Host *otherHost) {
		if( (strcmp(otherHost->mIP, mIP) == 0) && (strcmp(otherHost->mPort, mPort) == 0) ){
			return true;
		}
		return false;
	}
};




#endif /* HOST_H_ */
