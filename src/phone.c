/*
 * phone.c
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */
#include "../headers/defines.h"
#include "../headers/wrapper.h"
#include "../headers/phone.h"

void phoneWorker(void * phoneID){
	int phone = (int)phoneID;
	char data[MSG_SIZE];
	CLI_Write("Starting Phone Task\n");
	while(1){
		while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
		while(0 > getPhoneData(&phone, data, MSG_SIZE)){
			xSemaphoreGive(networking_sem);
			vTaskDelay(10);
			//CLI_Write("Phone Task Listening Again\n\r");
			while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
		}
		//CLI_Write("Got Input\n\r");
		xSemaphoreGive(networking_sem);
		inputConvert(data);
		xSemaphoreGive(updateLight_sem);
		//CLI_Write("Taking Again\n\r");
		while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
		//CLI_Write("Sending OK\n\r");
		sendPhoneOk(&phone);  //Send Phone has self contained locking
		xSemaphoreGive(networking_sem);
	}
}

static int inputDigitCounter(char * rawData, int start){
	int i = start;
	for(i = start; i < MSG_SIZE && (rawData[i] >= '0' && rawData[i] <= '9'); i++);
	return i - start;
}
void inputConvert(char * rawData){
	int i, lightIndex, temp;
	//convert the first value to the light value, have to minus one cause hue starts count at 1, fucking n00bs
	//If the command is turning the light off, the rest of the message is garbage
	for(i = 0; i < MSG_SIZE; i++){
		if(rawData[i] == 'l'){
			lightIndex = (int)(rawData[i+1] - 48 - 1);
			i = i + 2;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'f'){
			lightsData[lightIndex].on = 0;
			i++;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'o'){
			lightsData[lightIndex].on = 1;
			i++;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'r'){
			if(rawData[i + 1] == 'f'){
				relay.current = 0;
				relay.on = 0;
			}
			if(rawData[i + 1] == 'o'){
				relay.current = 0;
				relay.on = 1;
			}
			i = i + 2;
		}
		if(rawData[i] == 'e'){
			switch(rawData[i + 1]){
				case 'r':
					relayEnable = 1;
					break;
				case 'h':
					hueEnable = 1;
					break;
				case 'i':
					IREnable = 1;
					break;
			}
			i = i + 2;
		}
		if(rawData[i] == 'd'){
			switch(rawData[i + 1]){
				case 'r':
					relayEnable = 0;
					break;
				case 'h':
					hueEnable = 0;
					break;
				case 'i':
					IREnable = 0;
					break;
			}
			i = i + 2;
		}
		if(rawData[i] == 'i'){
			IR.current = 0;
			IR.IRColorValue = (0x10 * (rawData[i + 1] - 48)) + (rawData[i + 2] - 48);
			i = i + 3;
		}
		//read which parameter to update, then figure out the length of the number, copy the char string and update the counter
		if(rawData[i] == 's'){
			i++;
			lightsData[lightIndex].satSize = inputDigitCounter(rawData, i);
			strCpy(rawData + i, lightsData[lightIndex].sat, lightsData[lightIndex].satSize);
			i = i + lightsData[lightIndex].satSize;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'b'){
			i++;
			lightsData[lightIndex].briSize = inputDigitCounter(rawData, i );
			strCpy(rawData + i, lightsData[lightIndex].bri, lightsData[lightIndex].briSize);
			i = i + lightsData[lightIndex].briSize;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'x'){
			i++;
			lightsData[lightIndex].xSize = inputDigitCounter(rawData, i );
			strCpy(rawData + i, lightsData[lightIndex].x, lightsData[lightIndex].xSize);
			i = i + lightsData[lightIndex].xSize;
			lightsData[lightIndex].current = 0;
		}
		if(rawData[i] == 'y'){
			i++;
			lightsData[lightIndex].ySize = inputDigitCounter(rawData, i );
			strCpy(rawData + i, lightsData[lightIndex].y, lightsData[lightIndex].ySize);
			i = i + lightsData[lightIndex].ySize;
			lightsData[lightIndex].current = 0;
		}
	}
}

void sendPhoneOk(int *phoneID){
	while(0 > sendWrapper(*phoneID,"ok",2,0)){
		xSemaphoreGive(networking_sem);
		vTaskDelay(10);
		//CLI_Write("Phone Sending Again\n\r");
		while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
	}
}

_i32 getPhoneData(int *phoneID, char * buf, int len){
	int istatus;
	//fd_set read, activeRead;
	//struct SlTimeval_t time;
	//time.tv_sec = 1;
	//time.tv_usec = 0;
	//SL_FD_ZERO(&read);
	//SL_FD_SET(*phoneID, &read);
	//do{
	//	activeRead = read;
		//status = sl_Select(*phoneID + 1, &activeRead, NULL, NULL, &time);
	//	if(status > 0){
			istatus = sl_Recv(*phoneID, buf, len, 0);
			if(istatus == 0){
				sl_Close(*phoneID);
				xSemaphoreGive(networking_sem);
				CLI_Write(" Phone Disconnect Detected, Deleting Phone\n");
				vTaskDelete(NULL);
			}
			//if(0 == sl_Recv(*phoneID, buf, len, 0))
				//CLI_Write("Phone sending extra shit \n\r");
		//}
	//}while(status <= 0);
	//SL_FD_CLR(*phoneID, &read);
	return istatus;
}
