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


#include "../headers/lightsManager.h"
#include "../headers/wrapper.h"
#include "../headers/defines.h"
#include "../headers/phone.h"
#include "../headers/server.h"
#include "../headers/externals.h"

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationIdleHook( void );
void vApplicationTickHook( void );
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook( void );

void vApplicationIdleHook( void )
{
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* This function will only be called if an API call to create a task, queue
    or semaphore fails because there is too little heap RAM remaining. */
    for( ;; );
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
    /* This function will only be called if a task overflows its stack.  Note
    that stack overflow checking does slow down the context switch
    implementation. */
    for( ;; );
}

//Global semaphores

/*
 * GLOBAL VARIABLES -- End
 */

/*
 * STATIC FUNCTION DEFINITIONS -- Start
 */
static _i32 configureSimpleLinkToDefaultState();
static _i32 establishConnectionWithAP();
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

/*!
    \brief This function initializes the application variables

    \param[in]  None

    \return     0 on success, negative error-code on error
*/

/*
 * Application's entry point
 */
int main(int argc, char** argv)
{
    _i32 retVal = -1;
    int i;
    g_Status = 0;
    vSemaphoreCreateBinary(updateLight_sem);
    /* Stop WDT and initialize the system-clock of the MCU
       These functions needs to be implemented in PAL */
    stopWDT();
    initClk();

    initializeTimer0(80000);
    initializeADC();
    lumosGpioConfig();
    pwmInit();

    /* Configure command line interface */
    CLI_Configure();


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
      //  CLI_Write(" Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    CLI_Write(" Device started as STATION \n\r");

    /* Connecting to WLAN AP - Set with static parameters defined at the top
    After this call we will be connected and have IP address */
    retVal = establishConnectionWithAP();
    if(retVal < 0)
       {
      //  CLI_Write(" Failed to establish connection w/ an AP \n\r");
           LOOP_FOREVER();
       }

    CLI_Write(" Connection established w/ AP and IP is acquired \n\r");
    //init default light values
    lightsMode = 1;
    for(i = 0; i < 3; i++){
		lightsData[i].on = 0;
		lightsData[i].current = 1;
		strCpy("150", lightsData[i].sat, 3);
		lightsData[i].satSize = 3;
		strCpy("100", lightsData[i].bri, 3);
		lightsData[i].briSize = 3;
		//strCpy("30000", lightsData[i].hue, 5);
		lightsData[i].hue[0] = '0';
		lightsData[i].hueSize = 1;
		lightsData[i].numChange = 0;
		lightsData[i].numChangeMode = -1;
    }
    vSemaphoreCreateBinary(updateLight_sem);
    vSemaphoreCreateBinary(networking_sem);
    xTaskCreate(updateLights, "Lights Manager", 1536, NULL, 2, NULL); //4096
    xTaskCreate(serverThread, "Server", 1024, NULL, 1, NULL);
    vTaskStartScheduler();

    return 0;
}

