/*
 * main.c - TCP socket sample application
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * Application Name     -   TCP socket
 * Application Overview -   This is a sample application demonstrating how to open
 *                          and use a standard TCP socket with CC3100.
 * Application Details  -   http://processors.wiki.ti.com/index.php/CC31xx_TCP_Socket_Application
 *                          doc\examples\tcp_socket.pdf
 */


#include "simplelink.h"
#include "sl_common.h"

#define APPLICATION_VERSION "1.1.0"

#define SL_STOP_TIMEOUT        0xFF

/* IP addressed of server side socket. Should be in long format,
 * E.g: 0xc0a8010a == 192.168.1.10 */
#define LIGHTS_IP_ADDR         0xc0a8018A //192.168.1.138
#define PORT_NUM        80            /* Port number to be used */

#define BUF_SIZE        1400
#define NO_OF_PACKETS   1000
#define MSG_SIZE 17
#define FLOWCONTROL(func) \
	clean = (func);\
	if(clean < 0)\
		continue;\

/* Application specific status/error codes */
typedef enum{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0,        /* Choosing this number to avoid overlap w/ host-driver's error codes */
    TCP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    TCP_RECV_ERROR = TCP_SEND_ERROR -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

typedef struct lightData{
	int current;
	int on;
	char bri[3];
	int briSize;
	char sat[3];
	int satSize;
	char hue[6];
	int hueSize;
} lightData;

_u8 g_Status = 0;
#define LIGHT_INDEX 29
#define HEADER_LEN 122
_i16 Lights_ID = -1;
_i16 Server_ID = 0;
char header[HEADER_LEN] = "PUT /api/newdeveloper/lights/1/state HTTP/1.1\r\nHost: 192.168.1.133\r\nContent-Type: application/json\r\nContent-Length: 50\r\n\r\n";
lightData lightsData[3];
char data[MSG_SIZE];
/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState();
static _i32 establishConnectionWithAP();
static _i32 initializeAppVariables();
static _i32 TcpServerStart(_u16 Port);
static _i32 phoneConnect(int * phoneID);
static _i32 getPhoneData(int * phoneID, char * buf, int len);
static void displayBanner();
static int connectToLights();
static int disconnectLights();
static void inputConvert(char * rawData);
static void jsonPut();
static void sendPhoneOk(int *phoneID);
static int sendWrapper(int sd, char *pBuf, int Len, int flags);
/*
 * STATIC FUNCTION DEFINITIONS -- End
 */


/*
 * ASYNCHRONOUS EVENT HANDLERS -- Start
 */
/*!
    \brief This function handles WLAN events

    \param[in]      pWlanEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void strCpy(char * str0, char * str1, int size){
	int i = 0;
	for(i = 0; i < size; i++){
		str1[i] = str0[i];
	}
}
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
        CLI_Write(" [WLAN EVENT] NULL Pointer Error \n\r");
    
    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);

            /*
             * Information about the connected AP (like name, MAC etc) will be
             * available in 'slWlanConnectAsyncResponse_t' - Applications
             * can use it if required
             *
             * slWlanConnectAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
             *
             */
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            /* If the user has initiated 'Disconnect' request, 'reason_code' is
             * SL_USER_INITIATED_DISCONNECTION */
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                CLI_Write(" Device disconnected from the AP on application's request \n\r");
            }
            else
            {
                CLI_Write(" Device disconnected from the AP on an ERROR..!! \n\r");
            }
        }
        break;

        default:
        {
            CLI_Write(" [WLAN EVENT] Unexpected event \n\r");
        }
        break;
    }
}

/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication

    \param[in]      pNetAppEvent is the event passed to the handler

    \return         None

    \note

    \warning
*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
        CLI_Write(" [NETAPP EVENT] NULL Pointer Error \n\r");
 
    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

            /*
             * Information about the connection (like IP, gateway address etc)
             * will be available in 'SlIpV4AcquiredAsync_t'
             * Applications can use it if required
             *
             * SlIpV4AcquiredAsync_t *pEventData = NULL;
             * pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
             *
             */
        }
        break;

        default:
        {
            CLI_Write(" [NETAPP EVENT] Unexpected event \n\r");
        }
        break;
    }
}

/*!
    \brief This function handles callback for the HTTP server events

    \param[in]      pHttpEvent - Contains the relevant event information
    \param[in]      pHttpResponse - Should be filled by the user with the
                    relevant response information

    \return         None

    \note

    \warning
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
    CLI_Write(" [HTTP EVENT] Unexpected event \n\r");
}

/*!
    \brief This function handles general error events indication

    \param[in]      pDevEvent is the event passed to the handler

    \return         None
*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    /*
     * Most of the general errors are not FATAL are are to be handled
     * appropriately by the application
     */
    CLI_Write(" [GENERAL EVENT] \n\r");
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler

    \return         None
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(pSock == NULL)
        CLI_Write(" [SOCK EVENT] NULL Pointer Error \n\r");

    switch( pSock->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
            /*
             * TX Failed
             *
             * Information about the socket descriptor and stasttus will be
             * available in 'SlSockEventData_t' - Applications can use it if
             * required
             *
             * SlSockEventData_t *pEventData = NULL;
             * pEventData = & pSock->EventData;
             */
            switch( pSock->EventData.status )
            {
                case SL_ECLOSE:
                    CLI_Write(" [SOCK EVENT] Close socket operation, failed to transmit all queued packets\n\r");
                    break;
                default:
                    CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
                    break;
            }
            break;

        default:
            CLI_Write(" [SOCK EVENT] Unexpected event \n\r");
            break;
    }
}
/*
 * ASYNCHRONOUS EVENT HANDLERS -- End
 */

/*
 * Application's entry point
 */
int main(int argc, char** argv)
{
    _i32 retVal = -1;
    int phone = -1;
    int i;
    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    /* Stop WDT and initialize the system-clock of the MCU
       These functions needs to be implemented in PAL */
    stopWDT();
    initClk();

    /* Configure command line interface */
    CLI_Configure();

    displayBanner();

    /*
     * Following function configures the device to default state by cleaning
     * the persistent settings stored in NVMEM (viz. connection profiles &
     * policies, power policy etc)
     *
     * Applications may choose to skip this step if the developer is sure
     * that the device is in its default state at start of application
     *
     * Note that all profiles and persistent settings that were done on the
     * device will be lost
     */
    retVal = configureSimpleLinkToDefaultState();
    if(retVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == retVal)
        {
            CLI_Write(" Failed to configure the device in its default state \n\r");
        }

        LOOP_FOREVER();
    }

    CLI_Write(" Device is configured in default state \n\r");

    /*
     * Assumption is that the device is configured in station mode already
     * and it is in its default state
     */
    /* Initializing the CC3100 device */
    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) ||
        (ROLE_STA != retVal) )
    {
        CLI_Write(" Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    CLI_Write(" Device started as STATION \n\r");

    /* Connecting to WLAN AP - Set with static parameters defined at the top
    After this call we will be connected and have IP address */
    retVal = establishConnectionWithAP();
    if(retVal < 0)
       {
        CLI_Write(" Failed to establish connection w/ an AP \n\r");
           LOOP_FOREVER();
       }

    CLI_Write(" Connection established w/ AP and IP is acquired \n\r");
    //init default light values
    for(i = 0; i < 3; i++){
    	lightsData[i].on = 0;
    	lightsData[i].current = 1;
    	strCpy("255", lightsData[i].sat, 3);
    	lightsData[i].satSize = 3;
    	strCpy("255", lightsData[i].bri, 3);
    	lightsData[i].briSize = 3;
    	//strCpy("30000", lightsData[i].hue, 5);
    	lightsData[i].hue[0] = '0';
    	lightsData[i].hueSize = 1;
    }

    CLI_Write(" Starting Server \n\r");
    TcpServerStart(110);
    CLI_Write(" Server Up \n\r");
    while(1){
    	if(phone == -1){
    		phoneConnect(&phone);
    	}else{
    		if(0 > getPhoneData(&phone, data, MSG_SIZE))
    			continue;
    		inputConvert(data);
    		jsonPut();
    		sendPhoneOk(&phone);
    	}
    }
    //BsdTcpServer(110);
    sl_Stop(SL_STOP_TIMEOUT);
    return 0;
}

/*!
    \brief This function configure the SimpleLink device in its default state. It:
           - Sets the mode to STATION
           - Configures connection policy to Auto and AutoSmartConfig
           - Deletes all the stored profiles
           - Enables DHCP
           - Disables Scan policy
           - Sets Tx power to maximum
           - Sets power policy to normal
           - Unregisters mDNS services
           - Remove all filters

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/
static _i32 configureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    _u8           val = 1;
    _u8           configOpt = 0;
    _u8           configLen = 0;
    _u8           power = 0;

    _i32          retVal = -1;
    _i32          mode = -1;

    mode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in staion-mode */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything */
            while(!IS_IP_ACQUIRED(g_Status)) { _SlNonOsMainLoopTask(); }
        }

        /* Switch to STA role and restart */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(retVal);

        /* Check if the device is in station again */
        if (ROLE_STA != retVal)
        {
            /* We don't want to proceed if the device is not coming up in station-mode */
            ASSERT_ON_ERROR(DEVICE_NOT_IN_STATION_MODE);
        }
    }

    /* Get the device's version-information */
    configOpt = SL_DEVICE_GENERAL_VERSION;
    configLen = sizeof(ver);
    retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (_u8 *)(&ver));
    ASSERT_ON_ERROR(retVal);

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy) */
    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove all profiles */
    retVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retVal);

    /*
     * Device in station-mode. Disconnect previous connection if any
     * The function returns 0 if 'Disconnected done', negative number if already disconnected
     * Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
     */
    retVal = sl_WlanDisconnect();
    if(0 == retVal)
    {
        /* Wait */
        while(IS_CONNECTED(g_Status)) { _SlNonOsMainLoopTask(); }
    }

    /* Enable DHCP client*/
    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&val);
    ASSERT_ON_ERROR(retVal);

    /* Disable scan */
    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Set Tx power level for station mode
       Number between 0-15, as dB offset from max power - 0 will set maximum power */
    power = 0;
    retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *)&power);
    ASSERT_ON_ERROR(retVal);

    /* Set PM policy to normal */
    retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Unregister mDNS services */
    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove  all 64 filters (8*8) */
    pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(retVal);

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    return retVal; /* Success */
}

/*!
    \brief Connecting to a WLAN Access point

    This function connects to the required AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]  None

    \return     0 on success, negative error-code on error

    \note

    \warning    If the WLAN connection fails or we don't acquire an IP address,
                We will be stuck in this function forever.
*/
static _i32 establishConnectionWithAP()
{
    SlSecParams_t secParams = {0};
    _i32 retVal = 0;

    secParams.Key = PASSKEY;
    secParams.KeyLen = pal_Strlen(PASSKEY);
    secParams.Type = SEC_TYPE;

    retVal = sl_WlanConnect(SSID_NAME, pal_Strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(retVal);

    /* Wait */
    while((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status))) { _SlNonOsMainLoopTask(); }

    return SUCCESS;
}


static _i32 TcpServerStart(_u16 Port){
	    SlSockAddrIn_t  LocalAddr;

	    _u16          AddrSize = 0;
	    _i32          Status = 0;
	    /*SlSockNonblocking_t noBlock;
	    noBlock.NonblockingEnabled = 1;*/
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
	    //sl_SetSockOpt(Server_ID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &noBlock, sizeof(SlSockNonblocking_t));
	    return SUCCESS;
}

static _i32 serverRestart(){
	CLI_Write("Restarting Server\n\r");
	sl_Close(Server_ID);
	TcpServerStart(110);
	return SUCCESS;
}

static _i32 phoneConnect(int * phoneID){
	SlSockAddrIn_t  Addr;
	_i32   Status = 0;
	_u16   AddrSize = 0;
	/*SlSockNonblocking_t noBlock;
	noBlock.NonblockingEnabled = 1;*/
	Status = sl_Listen(Server_ID, 0);
	if( Status < 0 ){
		CLI_Write("Listen Failed, restarting server\n\r");
		while(0 > serverRestart());
		return -1;
	}
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
		CLI_Write(" [TCP Server] Accept connection Error \n\r");
		*phoneID = -1;
		return -1;
	}
	CLI_Write(" Phone Connected\n\r");
	return SUCCESS;
}

static _i32 getPhoneData(int *phoneID, char * buf, int len){
	int status, istatus;
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
			if(istatus <= 0){
				CLI_Write(" [TCP Server] Data recv Error, disconnecting phone \n\r");
				sl_Close(*phoneID);
				*phoneID = -1;
				ASSERT_ON_ERROR(TCP_RECV_ERROR);
			}
			//if(0 == sl_Recv(*phoneID, buf, len, 0))
				//CLI_Write("Phone sending extra shit \n\r");
		//}
	//}while(status <= 0);
	//SL_FD_CLR(*phoneID, &read);
	return SUCCESS;
}

static void sendPhoneOk(int *phoneID){
	while(0 >sendWrapper(*phoneID,"ok",2,0));
}

/*!
    \brief This function initializes the application variables

    \param[in]  None

    \return     0 on success, negative error-code on error
*/
static _i32 initializeAppVariables()
{
    g_Status = 0;

    return SUCCESS;
}

/*!
    \brief This function displays the application's banner

    \param      None

    \return     None
*/
static int connectToLights(){
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
			//CLI_Write(" [Lights Connect] Create socket Error \n\r");
			ASSERT_ON_ERROR(Lights_ID);
		}

	    Status = sl_Connect(Lights_ID, ( SlSockAddr_t *)&Addr, AddrSize);
	    if( Status < 0 )
	    {
	        sl_Close(Lights_ID);
	       //CLI_Write(" [Lights Connect]  TCP connection Error \n\r");
	        ASSERT_ON_ERROR(Status);
	    }
	    //sl_SetSockOpt(Lights_ID, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &noBlock, sizeof(SlSockNonblocking_t));
	    return SUCCESS;
}

static int recvFromLights(){
	int i, status, istatus = -1;
	//fd_set read, activeRead;
	//struct SlTimeval_t time;
	//time.tv_sec = 1;
	//time.tv_usec = 0;
	//SL_FD_ZERO(&read);
	//SL_FD_SET(Lights_ID, &read);
	while(istatus != 0){
				istatus = sl_Recv(Lights_ID, data, MSG_SIZE, 0);
				if(istatus < 0)
					CLI_Write("Failed to read shit\n\r");
			//}
		//}while(status <= 0);
	}
	//SL_FD_CLR(Lights_ID, &read);
	return SUCCESS;
}

static int disconnectLights(){
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

static int inputDigitCounter(char * rawData, int start){
	int i = start;
	for(i = start; i < MSG_SIZE && (rawData[i] >= 48 && rawData[i] < 58); i++);
	return i - start;
}
static void inputConvert(char * rawData){
	int i, lightIndex;
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
		if(rawData[i] == 'h'){
			i++;
			lightsData[lightIndex].hueSize = inputDigitCounter(rawData, i );
			strCpy(rawData + i, lightsData[lightIndex].hue, lightsData[lightIndex].hueSize);
			i = i + lightsData[lightIndex].hueSize;
			lightsData[lightIndex].current = 0;
		}
	}
}

static int sendWrapper(int sd, char *pBuf, int Len, int flags){
	int status, istatus;
	//SlFdSet_t writeFds;
	//SlFdSet_t activeWriteFds;
	//SlTimeval_t         timeout;
	if(Len < 0){
		CLI_Write("Stupid Fucking Bug \n\r");
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
				CLI_Write("Failed to write shit\n\r");
				return -1;
			}
		//}
	//} while(status <= 0);
	//SL_FD_CLR(sd, &writeFds);
	return 0;
}

static void jsonPut(){
	int i;
	for(i = 0; i < 3; i++){
		if(lightsData[i].current == 0){
			while(0 > connectToLights());
			header[LIGHT_INDEX] = (char)(i + 1 + 48); //Change the value in the header url to match which light we are trying to influence.
			//The header is built for the largest message we plan to send.  Should work fine with shorter messages.
			/*
			FLOWCONTROL(sl_Send(Lights_ID, header, HEADER_LEN, 0))
			//Start building the message.
			FLOWCONTROL(sl_Send(Lights_ID, "{\"on\":", 6, 0))
			if(lightsData[i].on){
				FLOWCONTROL(sl_Send(Lights_ID, "true", 4, 0))
			}else{
				FLOWCONTROL(sl_Send(Lights_ID, "false", 5, 0))
				return;
			}
			FLOWCONTROL(sl_Send(Lights_ID, ", \"sat\":", 8, 0))
			FLOWCONTROL(sl_Send(Lights_ID, lightsData[i].sat, lightsData[i].satSize, 0))
			FLOWCONTROL(sl_Send(Lights_ID, ", \"bri\":", 8, 0))
			FLOWCONTROL(sl_Send(Lights_ID, lightsData[i].bri, lightsData[i].briSize, 0))
			FLOWCONTROL(sl_Send(Lights_ID, ", \"hue\":", 8, 0))
			FLOWCONTROL(sl_Send(Lights_ID, lightsData[i].hue, lightsData[i].hueSize, 0))
			FLOWCONTROL(sl_Send(Lights_ID, "}", 1, 0))
			*/
			while(0 > sendWrapper(Lights_ID, header, HEADER_LEN, 0));
			//Start building the message.
			while(0 > sendWrapper(Lights_ID, "{\"on\":", 6, 0));
			if(lightsData[i].on){
				while(0 > sendWrapper(Lights_ID, "true", 4, 0));
				while(0 > sendWrapper(Lights_ID, ", \"sat\":", 8, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].sat, lightsData[i].satSize, 0));
				while(0 > sendWrapper(Lights_ID, ", \"bri\":", 8, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].bri, lightsData[i].briSize, 0));
				while(0 > sendWrapper(Lights_ID, ", \"hue\":", 8, 0));
				while(0 > sendWrapper(Lights_ID, lightsData[i].hue, lightsData[i].hueSize, 0));
				while(0 > sendWrapper(Lights_ID, "}", 1, 0));
			}else{
				while(0 > sendWrapper(Lights_ID, "false", 5, 0));
				while(0 > sendWrapper(Lights_ID, "}", 1, 0));
			}
			lightsData[i].current = 1;
			recvFromLights();
			disconnectLights();
		}
	}
}
static void displayBanner()
{
    CLI_Write("\n\r\n\r");
    CLI_Write(" TCP socket application - Version ");
    CLI_Write(APPLICATION_VERSION);
    CLI_Write("\n\r*******************************************************************************\n\r");
}

