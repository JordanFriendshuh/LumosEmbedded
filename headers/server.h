/*
 * server.h
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */

#ifndef SERVER_H_
#define SERVER_H_
#include "defines.h"

void serverThread(void * pvParm);
_i32 phoneConnect(int * phoneID);
_i32 serverRestart();
_i32 TcpServerStart(_u16 Port);

#endif /* SERVER_H_ */
