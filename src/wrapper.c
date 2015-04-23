/*
 * wrapper.c
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */
#include "../headers/defines.h"

int sendWrapper(int sd, char *pBuf, int Len, int flags){
	int istatus;
	//SlFdSet_t writeFds;
	//SlFdSet_t activeWriteFds;
	//SlTimeval_t         timeout;
	if(Len < 0){
		//CLI_Write("Stupid Fucking Bug \n\r");
		return -1;
	}
	//timeout.tv_sec = 1;
	//timeout.tv_usec = 0;
	//SL_FD_ZERO(&writeFds);
	//SL_FD_SET(sd, &writeFds);
	//do{
		//activeWriteFds = writeFds;
		//status = sl_Select(sd + 1, NULL, &activeWriteFds, NULL, &timeout);
		//if(status > 0){
			istatus = sl_Send(sd, pBuf, Len, flags);
			if(istatus < 0){
				//sl_Recv(*phoneID, buf, len, 0);
				if(0 == sl_Recv(sd, pBuf, Len, 0)){
					CLI_Write(" Phone Disconnect Detected, Deleting Phone\n");
					vTaskDelete(NULL);
				}
				//CLI_Write("Failed to write shit\n\r");
				return -1;
			}
		//}
	//} while(status <= 0);
	//SL_FD_CLR(sd, &writeFds);
	return 0;
}
