/*
 * helper.h
 *
 *  Created on: Sep 15, 2014
 *      Author: adminuser
 */

#include <vector>

#ifndef HELPER_H_
#define HELPER_H_

std::vector<char*>* tokenize(char *inputString, char* delimiter);
char *ToUpper(char* input);
char* GetMyIP();


#endif /* HELPER_H_ */
