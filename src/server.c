/*
 * server.c
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */
#include "../headers/defines.h"
#include "../headers/server.h"
#include "../headers/phone.h"

void serverThread(void * pvParm){
	int phone = -1;
	xTaskHandle xHandle = NULL;
	while(phone < 0){
		while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
		sl_Close(Server_ID);
		phone = TcpServerStart(110);
		xSemaphoreGive(networking_sem);
	}
	phone = -1;
	CLI_Write("Server Started\n\r");
	while(1){
		while(phone == -1){
			while(!xSemaphoreTake(networking_sem, portMAX_DELAY));
			phoneConnect(&phone);
			xSemaphoreGive(networking_sem);
			vTaskDelay(10);
		}
		xTaskCreate(phoneWorker, "Phone", 1024, (void*)phone, 1, &xHandle);
		if(xHandle == NULL){
			CLI_Write("Failed to create Phone Thread\n");
		}
		//while(!xSemaphoreTake(singlePhone_sem, portMAX_DELAY));
		CLI_Write("Ready for another phone\n\r");
		phone = -1;
	}
}

_i32 phoneConnect(int * phoneID){
	SlSockAddrIn_t  Addr;
	_i32   Status = 0;
	_u16   AddrSize = 0;
#ifdef NON_BLOCKING
	    SlSockNonblocking_t noBlock;
	    noBlock.NonblockingEnabled = 1;
#endif
	/*SlSockNonblocking_t noBlock;
	noBlock.NonblockingEnabled = 1;*/
	/*for(i = 0; i < 10; i++){
		sl_Recv(Server_ID, buf, 100, 0);
		CLI_Write(buf);
	}*/
	AddrSize = sizeof(SlSockAddrIn_t);
    *phoneID = sl_Accept(Server_ID, ( struct SlSockAddr_t *)&Addr,
                             (SlSocklen_t*)&AddrSize);
    //sl_SetSockOpt(phoneID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &noBlock, sizeof(SlSockNonblocking_t));
	if( *phoneID < 0 )
	{
		//CLI_Write(" [TCP Server] Accept connection Error \n\r");
		*phoneID = -1;
		return -1;
	}
#ifdef NON_BLOCKING
	    sl_SetSockOpt(*phoneID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &(noBlock.NonblockingEnabled), sizeof(_u32));
#endif
	CLI_Write(" Phone Connected\n\r");
	return SUCCESS;
}

_i32 TcpServerStart(_u16 Port){
	    SlSockAddrIn_t  LocalAddr;

	    _u16          AddrSize = 0;
	    _i32          Status = 0;
#ifdef NON_BLOCKING
	    SlSockNonblocking_t noBlock;
	    noBlock.NonblockingEnabled = 1;
#endif
	    LocalAddr.sin_family = SL_AF_INET;
	    LocalAddr.sin_port = sl_Htons((_u16)Port);
	    LocalAddr.sin_addr.s_addr = 0;

	    Server_ID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
	    if( Server_ID < 0 )
	    {
	        CLI_Write(" [TCP Server] Create socket Error \n\r");
	        ASSERT_ON_ERROR(Server_ID);
	    }

	    AddrSize = sizeof(SlSockAddrIn_t);
	    Status = sl_Bind(Server_ID, (SlSockAddr_t *)&LocalAddr, AddrSize);
	    if( Status < 0 )
	    {
	        sl_Close(Server_ID);
	        CLI_Write(" [TCP Server] Socket address assignment Error \n\r");
	        ASSERT_ON_ERROR(Status);
	    }
#ifdef NON_BLOCKING
	    sl_SetSockOpt(Server_ID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &(noBlock.NonblockingEnabled), sizeof(_u32));
#endif
	    Status = sl_Listen(Server_ID, 3);
	    return Status;
}
