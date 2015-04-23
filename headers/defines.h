/*
 * defines.h
 *
 *  Created on: Apr 9, 2015
 *      Author: CCS
 */

#ifndef DEFINES_H_
#define DEFINES_H_
#include "simplelink.h"
#include "sl_common.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "gpio.h"
#include "i2c.h"
#define APPLICATION_VERSION "1.1.0"
#define NON_BLOCKING

#define SL_STOP_TIMEOUT        0xFF

/* IP addressed of server side socket. Should be in long format,
 * E.g: 0xc0a8010a == 192.168.1.10 */
#define LIGHTS_IP_ADDR         0xc0a80102 //192.168.1.2
#define PORT_NUM        80            /* Port number to be used */

#define BUF_SIZE        1400
#define NO_OF_PACKETS   1000
#define MSG_SIZE 17
#define FLOWCONTROL(func) \
	clean = (func);\
	if(clean < 0)\
		continue;\

#define NUM_OF_LIGHTS 3
#define LIGHT_INDEX 29
#define HEADER_LEN 122
#define P0 (1 << 0)
#define P1 (1 << 1)
#define P2 (1 << 2)
#define P3 (1 << 3)
#define P4 (1 << 4)
#define P5 (1 << 5)
#define P6 (1 << 6)
#define P7 (1 << 7)

#define KNOB_RED 		P7
#define IR_LED_CTRL 	P6
#define OPT_IRQ 		P3
#define RELAY_RESET 	P7
#define RELAY_SET 		P6
#define OPT_I2C_SDA 	P1
#define OPT_I2C_SCL 	P0
#define MOTION_SENSE 	P3
#define MIC_AMPED 		P2
#define KNOB_PB 		P4
#define ENC_A 			P3
#define ENC_B 			P2
#define KNOB_BLUE 		P1
#define KNOB_GRN 		P0


extern xSemaphoreHandle updateLight_sem;
extern xSemaphoreHandle networking_sem;
extern void strCpy(char * str0, char * str1, int size);
extern volatile uint16_t scaled_adc0_right;


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

extern _u8 g_Status;
extern _i16 Lights_ID;
extern _i16 Server_ID;
extern char header[HEADER_LEN];
extern lightData lightsData[NUM_OF_LIGHTS];
extern char data[MSG_SIZE];


#endif /* DEFINES_H_ */
