// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#include "DAC.h"

void DAC_Init(void){
  // used in Lab 6 and lab 10
	SYSCTL_RCGCGPIO_R |= 0x02;
	
	volatile int i = 0;
	i++;
	i++;
	// DELAY
	
	GPIO_PORTB_DIR_R |= 0x3F; // sets port B0-5 to output
	GPIO_PORTB_DEN_R |= 0x3F;
	GPIO_PORTB_DR8R_R |= 0x3F;
	
}

void DAC_Out(uint32_t data){
  // used in Lab 6 and lab 10
	GPIO_PORTB_DATA_R = data;
}
