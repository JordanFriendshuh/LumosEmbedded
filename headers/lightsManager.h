/*
 * lightsManager.h
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */

#ifndef LIGHTSMANAGER_H_
#define LIGHTSMANAGER_H_

extern int lightsMode;

void updateLights(void *pvParam);
int disconnectLights();
int recvFromLights();
void jsonPut();
void numChange();
int connectToLights();
#endif /* LIGHTSMANAGER_H_ */
