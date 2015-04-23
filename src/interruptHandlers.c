/*
 * interruptHandlers.c
 *
 *  Created on: Apr 20, 2015
 *      Author: CCS
 */

#include "../headers/defines.h"
#include "inc/hw_timer.h"
#include "inc/hw_memmap.h"
#include "../headers/externals.h"

 bool mic_flag;



void gpioHandle(void){
	int i;
	static int flag = 1;
	//Turn left?
	if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & ENC_B){
		if(flag){
			for(i = 0; i < NUM_OF_LIGHTS; i++){
				lightsData[i].current = 0;
				if(lightsData[i].bri[1] == '0'){
					if(lightsData[i].bri[0] != '0'){
						lightsData[i].bri[0] = lightsData[i].bri[0] - 1;
						lightsData[i].bri[1] = '9';
					}
				}else{
					lightsData[i].bri[1] = lightsData[i].bri[1] - 1;
				}
			}
			xSemaphoreGiveFromISR(updateLight_sem, 0);
			flag = 0;
		}else{
			flag = 1;
		}

	}
	//Turn right?
	if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & ENC_A){
		if(flag){
			for(i = 0; i < NUM_OF_LIGHTS; i++){
				lightsData[i].current = 0;
				if(lightsData[i].bri[1] == '9'){
					lightsData[i].bri[0] = lightsData[i].bri[0] + 1;
					lightsData[i].bri[1] = '0';
				}else{
					if(!(lightsData[i].bri[0] == '2' && (lightsData[i].bri[1] == '4' || lightsData[i].bri[1] == '5'))){
						lightsData[i].bri[1] = lightsData[i].bri[1] + 1;
					}
				}
			}
			xSemaphoreGiveFromISR(updateLight_sem, 0);
			flag = 0;
		}else{
			flag = 1;
		}
	}

    if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & KNOB_PB){
    	for(i = 0; i < NUM_OF_LIGHTS; i++){
    		lightsData[i].current = 0;
    		if(lightsData[i].on  == 1)
    			lightsData[i].on = 0;
    		else
    			lightsData[i].on = 1;
    	}
    	xSemaphoreGiveFromISR(updateLight_sem, 0);
    	//portEND_SWITCHING_ISR();
	}
	HWREG(GPIO_PORTF_BASE + GPIO_O_ICR) |= (ENC_A | ENC_B | KNOB_PB);
//	portStruct->ICR |= P2|P3|P4;
}

void Timer0Handler()
{
	int i, mic_value;
	static mic_flag = 0;
	static mic_count = 0;
//	//comes in here every 1 ms
	mic_value = GetADCval(1)/41 + 1;

	if(mic_flag == 0){
		if(mic_value > 80){
			for(i = 0; i < NUM_OF_LIGHTS; i++){
				lightsData[i].current = 0;
				if(lightsData[i].on)
					lightsData[i].on = 0;
				else
					lightsData[i].on = 1;
			}
	 	xSemaphoreGiveFromISR(updateLight_sem, 0);
		mic_flag = 1;
		mic_count = 0;
		}
	}else{
		if(mic_count > 20)
			mic_flag = 0;
		else
			mic_count++;
	}

//	//clearing the interrupt
	HWREG(TIMER0_BASE + TIMER_O_ICR) = 0x01;

}

