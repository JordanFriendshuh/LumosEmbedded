/*
 * externals.c
 *
 *  Created on: Apr 22, 2015
 *      Author: Nick
 */
#include "../headers/defines.h"
#include "inc/hw_nvic.h"
//#include "inc/tm4c123gh6pm.h"
#include "../headers/lm4f120h5qr.h"

volatile uint16_t scaled_adc0_right = 0;

void lumosGpioConfig(){

	gpio_enable_port(GPIO_PORTF_BASE);
	gpio_config_digital_enable(GPIO_PORTF_BASE, KNOB_GRN | KNOB_BLUE | ENC_B | ENC_A | KNOB_PB);
	gpio_config_enable_output(GPIO_PORTF_BASE, KNOB_GRN | KNOB_BLUE);
	gpio_config_enable_input(GPIO_PORTF_BASE, ENC_A | ENC_B | KNOB_PB);
	HWREG(GPIO_PORTF_BASE + GPIO_O_DATA) |= (KNOB_GRN | KNOB_BLUE);

	HWREG(GPIO_PORTF_BASE + GPIO_O_IS ) &= ~(ENC_A | ENC_B | KNOB_PB);
	HWREG(GPIO_PORTF_BASE + GPIO_O_IBE ) &= ~(ENC_A | ENC_B | KNOB_PB);
	HWREG(GPIO_PORTF_BASE + GPIO_O_IEV ) &= ~(ENC_A | ENC_B | KNOB_PB);
	HWREG(GPIO_PORTF_BASE + GPIO_O_IEV ) |= (ENC_A | ENC_B | KNOB_PB);
	HWREG(GPIO_PORTF_BASE + GPIO_O_IM ) |= (ENC_A | ENC_B | KNOB_PB);

	gpio_enable_port(GPIO_PORTA_BASE);
	gpio_config_digital_enable(GPIO_PORTA_BASE, KNOB_RED);
	gpio_config_enable_output(GPIO_PORTA_BASE, KNOB_RED);

	gpio_enable_port(GPIO_PORTD_BASE);
	gpio_config_digital_enable(GPIO_PORTD_BASE, RELAY_RESET | RELAY_SET);
	gpio_config_enable_output(GPIO_PORTD_BASE, RELAY_RESET | RELAY_SET);
	//portStruct->IS &= ~(P2|P3|P4);
	//portStruct->IBE &= ~(P2|P3|P4);
//	portStruct->IEV &= ~(P2|P3|P4);
//	portStruct->IEV |= (P2|P3|P4);
//	portStruct->IM |= (P2|P3|P4);
	HWREG(NVIC_PRI4) = (HWREG(NVIC_PRI4) & 0x0FFFFFFF) | 0x40000000; //Priority 2
	HWREG(NVIC_EN0) |= NVIC_EN0_INT30;
}

void i2c_gpio_config(void){
	gpio_enable_port(GPIO_PORTD_BASE);
	gpio_config_digital_enable(GPIO_PORTD_BASE, OPT_I2C_SCL | OPT_I2C_SDA);
	gpio_config_open_drain(GPIO_PORTD_BASE, OPT_I2C_SDA);
	gpio_config_port_control(GPIO_PORTD_BASE, OPT_I2C_SCL | OPT_I2C_SDA);
}

void i2c_config(void){
	i2c_gpio_config();
	initializeI2CMaster(I2C3_BASE);
}

void initializeTimer0(uint32_t count)
{
	uint32_t wait;
	SYSCTL_RCGCTIMER_R   |=   SYSCTL_RCGCTIMER_R0;     // Turn on the clock to timer0
	wait = SYSCTL_RCGC1_R;
	wait=0;
	while(wait <50)
	{
			wait++;
	}

	TIMER0_CTL_R    &=  ~TIMER_CTL_TAEN;           // Disable the timer while we are configuring it.
  TIMER0_CFG_R    =   TIMER_CFG_32_BIT_TIMER;    // Configure for 32-bit Mode
  TIMER0_TAMR_R   =   TIMER_TAMR_TAMR_PERIOD;    // Put it into periodic mode
  TIMER0_TAILR_R  =   count;                  // Load the count value
	TIMER0_IMR_R    |=  TIMER_IMR_TATOIM;          // Enable Interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R & 0x0FFFFFFF) | 0x40000000; //Priority 2
	NVIC_EN0_R |= NVIC_EN0_INT19; 									//Enable timer0A interrupt
  TIMER0_CTL_R    |=  TIMER_CTL_TAEN;            // Turn the timer on

}

/****************************************************************************
 ****************************************************************************/
void initializeADC(void)
{
  uint32_t delay;


    // Enable Port E
    // See the schematics, page 3, on the class website to determine
    // which pins to configure
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    while(delay <1000){
			delay++;
		}
    // Set the direction as an input
	HWREG(GPIO_PORTE_BASE + GPIO_O_DIR) &= 0xFFF3;

    // Set the alternate function
	HWREG(GPIO_PORTE_BASE + GPIO_O_AFSEL) &= 0xFFF3;

    // Disable the Digital Enable
	HWREG(GPIO_PORTE_BASE + GPIO_O_DEN) &= 0xFFF3;

    // Enable Analog
	HWREG(GPIO_PORTE_BASE + GPIO_O_AMSEL) &= 0xFFFC;



    // Enable ADC
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    delay=0;
		while(delay <1000){
			delay++;
		}

    // Make sure to look at the #defines found in lm4f120h5qr.h.
    // Line 3367 starts the bit mask definitions for the ADC.

    // (Step 1 from Data Sheet)
    // Disable Sample Sequencer #3
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;
		//ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN2;

    // ((Step 2 from Data Sheet)
    // Configure the sample sequencer so Sample Sequencer #3 (EM3)
    // is triggered by the processor
    ADC0_EMUX_R &= ~ADC_ACTSS_ASEN3;

    // (Step 3 from Data Sheet)
    // Clear the Sample Input Select for Sample Sequencer #3
    ADC0_SSMUX0_R &= ~ADC_SSMUX0_MUX2_M;

    // (Step 4 from Data Sheet)
    // Configure the Sample Sequencer #3 control register.  Make sure to set
    // the 1st Sample Interrupt Enable and the End of Sequence bits
    ADC0_SSCTL3_R |= 0x6;

    // Not shown in the data sheet.  See SAC register
    // Clear Averaging Bits
    ADC0_SAC_R &= 0xFFF8;

    // Not shown in the data sheet.  See SAC register
    // Average 64 samples
    ADC0_SAC_R |= 0x6;

    // Do NOT enable the Sequencer after this.  This is done in GetADCval

}

//*****************************************************************************
// Get the ADC value of a given ADC Channel
//*****************************************************************************
uint32_t GetADCval(uint32_t Channel)
{
  uint32_t result;

  ADC0_SSMUX3_R = Channel;      // Set the channel
  ADC0_ACTSS_R  |= ADC_ACTSS_ASEN3; // Enable SS3
  ADC0_PSSI_R = ADC_PSSI_SS3;     // Initiate SS3

  while(0 == (ADC0_RIS_R & ADC_RIS_INR3)); // Wait for END of conversion
  result = ADC0_SSFIFO3_R & 0x0FFF;     // Read the 12-bit ADC result
  ADC0_ISC_R = ADC_ISC_IN3;         // Acknowledge completion

  return result;
}

//*****************************************************************************
