/*
 * phone.h
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */

#ifndef PHONE_H_
#define PHONE_H_
void phoneWorker(void * phoneID);
void inputConvert(char * rawData);
void sendPhoneOk(int *phoneID);
_i32 getPhoneData(int *phoneID, char * buf, int len);

#endif /* PHONE_H_ */
