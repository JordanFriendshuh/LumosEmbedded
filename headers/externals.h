/*
 * externals.h
 *
 *  Created on: Apr 22, 2015
 *      Author: Nick
 */

#ifndef EXTERNALS_H_
#define EXTERNALS_H_


void lumosGpioConfig();

void i2c_gpio_config(void);

void i2c_config(void);

void initializeTimer0(uint32_t count);

/****************************************************************************
 ****************************************************************************/
void initializeADC(void);
//*****************************************************************************
// Get the ADC value of a given ADC Channel
//*****************************************************************************
uint32_t GetADCval(uint32_t Channel);

//*****************************************************************************
void pwmInit();
//*****************************************************************************
// The refresh rate will go from 0-75Hz.
//*****************************************************************************
void scale_ad(void);

#endif /* EXTERNALS_H_ */
