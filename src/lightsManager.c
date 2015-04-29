/*
 * updateLights.c
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */
#include "../headers/defines.h"
#include "../headers/externals.h"
#include "../headers/wrapper.h"
#include "../headers/lightsManager.h"
char header[HEADER_LEN] = "PUT /api/newdeveloper/lights/1/state HTTP/1.1\r\nHost: 192.168.1.2\r\nContent-Type: application/json\r\nContent-Length: 65\r\n\r\n";
lightData lightsData[NUM_OF_LIGHTS];
relayData relay;
IRDataStruct IR;
char data[MSG_SIZE];
_u8 g_Status = 0;
_i16 Lights_ID = -1;
_i16 Server_ID = 0;
int IRRunning = 0;
int relayEnable = 0;
int hueEnable = 0;
int IREnable = 0;
xSemaphoreHandle networking_sem;
xSemaphoreHandle updateLight_sem;

void updateLights(void *pvParam){
	while(1){
		while(!xSemaphoreTake(updateLight_sem, portMAX_DELAY));
		//Update the physical lights by turning the relay off or on
		if(relayEnable){
			if(relay.current == 0){
				relay.current = 1;
				if(relay.on){
					HWREG(GPIO_PORTD_BASE + GPIO_O_DATA) |= RELAY_SET;
				}else{
					HWREG(GPIO_PORTD_BASE + GPIO_O_DATA) |= RELAY_RESET;
				}
				vTaskDelay(1000);
				HWREG(GPIO_PORTD_BASE + GPIO_O_DATA) &= ~(RELAY_SET | RELAY_RESET);
				CLI_Write("Relay Updated\n\r");
			}
		}
		//Update the lights using the philips hue
		if(hueEnable){
			while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
			numChange();
			jsonPut();
			xSemaphoreGive(networking_sem);
		}
		//Update the lights using the IR sensor
		if(IREnable){
			if(IR.current == 0){
				IR.current = 1;
				CLI_Write("Updating IR\n\r");
				startIR();
			}
		}
	}
}

void startIR(){
	while(IRRunning){
		vTaskDelay(100);
	}
	if(IR.IRColorValue != IR_OFF && IR.on == 0){
		IRRunning = 1;
		IRData = 0x00ff0000;
		IRData |= (IR_ON << 8) | (~(IR_ON) & 0xff);
		pwmIRStart();
		oneShotSet(IR_9MS);
		while(IRRunning){
			vTaskDelay(100);
		}
		IR.on = 1;
	}
	if(IR.IRColorValue == IR_OFF)
		IR.on = 0;
	while(IRRunning){
		vTaskDelay(100);
	}
	IRRunning = 1;
	//Change later to reflect lightsdata
	IRData = 0x00ff0000;
	IRData |= (IR.IRColorValue << 8) | (~(IR.IRColorValue) & 0xff);
	pwmIRStart();
	oneShotSet(IR_9MS);
}

int pow(int base, int exp){
	int i, ret;
	ret = 1;
	for(i = 0; i < exp; i++){
		ret = ret * base;
	}
	return ret;
}
//only supports bri atm
int convertToNum(int index){
	int i, ret;
	ret = 0;
	for(i = 0; i < lightsData[index].briSize; i++){
		ret = ret + (lightsData[index].bri[i] - 48) * pow(10, lightsData[index].briSize - 1 - i);
	}
	return ret;
}

void convertFromNum(int index, int value){
	int i;
	if(value <= 0){
		lightsData[index].bri[0] = '1';
		lightsData[index].briSize = 1;
		return;
	}
	if(value > 255){
		lightsData[index].bri[0] = '2';
		lightsData[index].bri[1] = '5';
		lightsData[index].bri[2] = '5';
		lightsData[index].briSize = 3;
		return;
	}
	if(value / 100)
		lightsData[index].briSize = 3;
	else if(value /10)
		lightsData[index].briSize = 2;
	else
		lightsData[index].briSize = 1;
	for(i = 0; i < lightsData[index].briSize; i++){
		lightsData[index].bri[i] = (value/pow(10, lightsData[index].briSize - 1 - i) % 10 + 48);
	}
}

void numChange(){
	int i;
	for(i = 0; i < 3; i++){
		if(lightsData[i].numChangeMode != -1){
			convertFromNum(i, convertToNum(i) + lightsData[i].numChange);
			lightsData[i].numChangeMode = -1;
			lightsData[i].numChange = 0;
		}
	}
}

int connectToLights(){
	SlSockAddrIn_t  Addr;

	    _u16          AddrSize = 0;
	    _i16          Status = 0;
	    _u16 		  Port = 80;
	    /*SlSockNonblocking_t noBlock;
	    noBlock.NonblockingEnabled = 1;*/
	    Addr.sin_family = SL_AF_INET;
	    Addr.sin_port = sl_Htons((_u16)Port);
	    Addr.sin_addr.s_addr = sl_Htonl((_u32)LIGHTS_IP_ADDR);
	    AddrSize = sizeof(SlSockAddrIn_t);
		Lights_ID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
		if( Lights_ID < 0 )
		{
			CLI_Write(" [Lights Connect] Create socket Error \n\r");
			ASSERT_ON_ERROR(Lights_ID);
		}

	    Status = sl_Connect(Lights_ID, ( SlSockAddr_t *)&Addr, AddrSize);
	    if( Status < 0 )
	    {
	        sl_Close(Lights_ID);
	       CLI_Write(" [Lights Connect]  TCP connection Error \n\r");
	        ASSERT_ON_ERROR(Status);
	    }
	    //sl_SetSockOpt(Lights_ID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &noBlock, sizeof(SlSockNonblocking_t));
	    return SUCCESS;
}

int recvFromLights(){
	int istatus = -1;
	//fd_set read, activeRead;
	//struct SlTimeval_t time;
	//time.tv_sec = 1;
	//time.tv_usec = 0;
	//SL_FD_ZERO(&read);
	//SL_FD_SET(Lights_ID, &read);
	while(istatus != 0){
				istatus = sl_Recv(Lights_ID, data, MSG_SIZE, 0);
				//if(istatus < 0);
				//	CLI_Write("Failed to read shit\n\r");
			//}
		//}while(status <= 0);
	}
	//SL_FD_CLR(Lights_ID, &read);
	return SUCCESS;
}

int disconnectLights(){
	int status;
	SlFdSet_t writeFds;
	SlFdSet_t activeWriteFds;
	SlTimeval_t timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	SL_FD_ZERO(&writeFds);
	SL_FD_SET(Lights_ID, &writeFds);
	do{
		activeWriteFds = writeFds;
		status = sl_Select(Lights_ID + 1, NULL, &activeWriteFds, NULL, &timeout);
		if(status > 0){
			while(0 < sl_Close(Lights_ID)){
				CLI_Write("Failed to close\n\r");
			}
		}
	} while(status <= 0);
	SL_FD_CLR(Lights_ID, &writeFds);
	return SUCCESS;
}

void jsonPut(){
	int i;
	for(i = 0; i < 3; i++){
		if(lightsData[i].current == 0){
			while(0 > connectToLights());
			header[LIGHT_INDEX] = (char)(i + 1 + 48); //Change the value in the header url to match which light we are trying to influence.
			//The header is built for the largest message we plan to send.  Should work fine with shorter messages.
			while(0 > sendWrapper(Lights_ID, header, HEADER_LEN, 0));
			//Start building the message.
			while(0 > sendWrapper(Lights_ID, "{\"on\":", 6, 0));
			if(lightsData[i].on){
				while(0 > sendWrapper(Lights_ID, "true", 4, 0));
				while(0 > sendWrapper(Lights_ID, ",\"sat\":", 7, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].sat, lightsData[i].satSize, 0));
				while(0 > sendWrapper(Lights_ID, ",\"bri\":", 7, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].bri, lightsData[i].briSize, 0));
				while(0 > sendWrapper(Lights_ID, ",\"xy\":[0.", 9, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].x, lightsData[i].xSize, 0));
				while(0 > sendWrapper(Lights_ID, ",0.", 3, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].y, lightsData[i].ySize, 0));
				while(0 > sendWrapper(Lights_ID, "]}", 2, 0));
			}else{
				while(0 > sendWrapper(Lights_ID, "false", 5, 0));
				while(0 > sendWrapper(Lights_ID, "}", 1, 0));
			}
			lightsData[i].current = 1;
			recvFromLights();
			disconnectLights();
			CLI_Write("Light updated\n\r");
		}
	}
}
