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
#include "driverlib/timer.h"
#define MOT_SIZE 25
int encoderFlag = 0;
int encLeftCount;
int encRightCount;
int boolFlag = 1;
int buttonBounce = 0;
int prefMode = 0;
int micEnable = 0;
int motEnable = 0;

void gpioHandle(void){
	int i;
	//Turn left?
	if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & ENC_B){
		if(boolFlag){
			encLeftCount++;
			encoderFlag = 1;
			//xSemaphoreGiveFromISR(updateLight_sem, 0);
			boolFlag = 0;
		}else{
			boolFlag = 1;
		}
	}
	if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & ENC_A){
		if(boolFlag){
			encRightCount++;
			encoderFlag = 1;
			//xSemaphoreGiveFromISR(updateLight_sem, 0);
			boolFlag = 0;
		}else{
			boolFlag = 1;
		}
	}
	if(HWREG(GPIO_PORTF_BASE + GPIO_O_RIS) & KNOB_PB){
    	if(!buttonBounce){
    		switch(prefMode){
				case 0:
					relay.current = 0;
					relay.on = !relay.on;
					break;
				case 1:
					for(i = 0; i < NUM_OF_LIGHTS; i++){
						lightsData[i].current = 0;
						if(lightsData[i].on  == 1)
							lightsData[i].on = 0;
						else
							lightsData[i].on = 1;
					}
					break;
				case 2:
					IR.current = 0;
					if(IR.on)
						IR.IRColorValue = IR_OFF;
					else
						IR.IRColorValue = IR_ON;
					break;
    		}
    		buttonBounce = 1;
    		xSemaphoreGiveFromISR(updateLight_sem, 0);
    	}
    	//portEND_SWITCHING_ISR();
	}
	HWREG(GPIO_PORTF_BASE + GPIO_O_ICR) |= (ENC_A | ENC_B | KNOB_PB);
//	portStruct->ICR |= P2|P3|P4;
}

void Timer0Handler()
{
	int i, mic_value, motionValue, motLongAve, motShortAve;
	static int mic_flag = 0;
	static int mic_count = 0;
	static int motValues[MOT_SIZE];
	static int motIndex = 0;
	static short motFlag = 0;

//	//comes in here every 1 ms
	if(motEnable){
		motionValue = GetADCval(0);
		//motion sensor logic
		if(motFlag){
			motLongAve = 0;
			motShortAve = 0;
			motValues[motIndex % MOT_SIZE] = motionValue;
			for(i = 0; i < MOT_SIZE; i++){
				motLongAve += motValues[i];
				//Only include the 5 most recent for local average
				if((motIndex % MOT_SIZE - i) % MOT_SIZE < 5)
					motShortAve += motValues[i];
			}
			motLongAve = motLongAve / MOT_SIZE;
			motShortAve = motShortAve / 5;
			motIndex++;
			//motion detected, turn shit on
			//if(motShortAve - motLongAve > 2 || motShortAve - motLongAve < -2){
			if(motShortAve - motLongAve > 2 || motShortAve - motLongAve < -2){
				switch(prefMode){
					case 0:
						if(!relay.on){
							relay.current = 0;
							relay.on = 1;
							xSemaphoreGiveFromISR(updateLight_sem, 0);
						}
						break;
					case 1:
						for(i = 0; i < NUM_OF_LIGHTS; i++){
							if(!lightsData[i].on){
								lightsData[i].current = 0;
								lightsData[i].on = 1;
								xSemaphoreGiveFromISR(updateLight_sem, 0);
							}
						}
						break;
					case 2:
						if(!IR.on){
							IR.current = 0;
							IR.IRColorValue = IR_ON;
							xSemaphoreGiveFromISR(updateLight_sem, 0);
						}
						break;
				}
			}
		}else{
			motValues[motIndex] = motionValue;
			motIndex++;
			if(motIndex == MOT_SIZE){
				motFlag = 1;
			}
		}
	}
	if(buttonBounce){
		buttonBounce++;
		if(buttonBounce > 20){
			buttonBounce = 0;
		}
	}
	if(encoderFlag){
		if(encoderFlag == 50){
			encoderFlag = 0;
			boolFlag = 1;
			switch(prefMode){
				case 1:
					for(i = 0; i < NUM_OF_LIGHTS; i++){
						lightsData[i].current = 0;
						lightsData[i].numChangeMode = 0;
						//10's digit is the middle digit
						lightsData[i].numChange += (ENC_SCALE * encRightCount) + (encLeftCount * (-ENC_SCALE));
					}
					break;
				case 2:
					IR.current = 0;
					if(encRightCount > encLeftCount)
						IR.IRColorValue = IR_BRIGHT;
					else if(encRightCount < encLeftCount)
						IR.IRColorValue = IR_DIM;
					break;
			}
			encRightCount = 0;
			encLeftCount = 0;
			xSemaphoreGiveFromISR(updateLight_sem, 0);
		}else{
			encoderFlag++;
		}
	}
	if(micEnable){
		mic_value = GetADCval(1)/41 + 1;
		if(mic_flag == 0){
			if(mic_value > 80){
				switch(prefMode){
					case 0:
						relay.current = 0;
						relay.on = !relay.on;
						break;
					case 1:
						for(i = 0; i < NUM_OF_LIGHTS; i++){
							lightsData[i].current = 0;
							if(lightsData[i].on)
								lightsData[i].on = 0;
							else
								lightsData[i].on = 1;
						}
						break;
					case 2:
						IR.current = 0;
						if(IR.on)
							IR.IRColorValue = IR_OFF;
						else
							IR.IRColorValue = IR_ON;
						break;
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
	}
//	//clearing the interrupt
	HWREG(TIMER0_BASE + TIMER_O_ICR) = 0x01;
}

void Timer1Handle(){

	static int index = -1;
	static int pause = 0;
	TimerDisable(TIMER1_BASE, TIMER_A);
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	if(index == -1){
		//Init pause
		pwmIRStop();
		oneShotSet(IR_4_5MS);
		index++;
	}else if(index == 33){
		index = -1;
		IRRunning = 0;
		pause = 0;
	}else if(index == 32){ //33?
		if(pause){
			pwmIRStop();
			oneShotSet(IR_560uS);
			index++;
		}else{
			//send stop bit
			pwmIRStart();
			oneShotSet(IR_560uS);
			pause = 1;
		}
	}else{
		if(pause){
			pwmIRStop();
			if(IRData & (0x1 << (31 - index))){
				oneShotSet(IR_1650uS);
			}else{
				oneShotSet(IR_560uS);
			}
			pause = 0;
			index++;
		}else{
			pwmIRStart();
			oneShotSet(IR_560uS);
			pause = 1;
		}
	}
	//HWREG(TIMER1_BASE + TIMER_O_ICR) = 0x01;
}
