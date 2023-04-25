	// main.cpp
// Runs on LM4F120/TM4C123
// Albert Wang and Chris Fu
// This is a starter project for the ECE319K Lab 10 in C++

// Last Modified: 4/11/23
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1 
// 8*R resistor DAC bit 2 on PB2
// 4*R resistor DAC bit 1 on PB3
// 2*R resistor DAC bit 2 on PB4
// 1*R resistor DAC bit 3 on PB5 (most significant bit)
// LED on PB6
// LED on PB7

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "random.h"
#include "SlidePot.h"
#include "Timer0.h"
#include "Timer1.h"
#include "Module.h"
#include "Sprites.h"
#include "DAC.h"
#include "Sound.h"

SlidePot my(2000,0);
int setUp[12] = {0,2,1,4,1,3,-1,-1,-1,-1,-1,-1};
Module* modules[12];
Timer* t = nullptr;
int num_created_modules = 0;
Bomb b(setUp, 180);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);
extern "C" void TIMER3A_Handler(void);
extern "C" void WaitForInterrupt(void);  // low power mode
extern Bomb b;
extern int prev_sel;
extern bool selected;
extern bool noneSelected();
bool selectorclicked = false;
Wires* w1 = nullptr;
Wires* w2 = nullptr;
Math* m1 = nullptr;
Math* m2 = nullptr;
Button* b1 = nullptr;
Button* b2 = nullptr;
MorseCode* mc1 = nullptr;
MorseCode* mc2 = nullptr;
bool selectormoved = true;
int num_wires = 0, num_math = 0, num_button = 0, num_MorseCode = 0;
int timer_location;
void (*PeriodicTask3)(void);
const unsigned short *explosion[6] = {explosion1, explosion2, explosion3, explosion4, explosion5, explosion6};
int language = 0;
int anim = 5;
int screen = 0; // 0 is menu, 1 is game screen, 2 is if successful, 3 is if bomb exploded, 4 for when a module is selected
bool redrawSelector = false;
uint32_t data, prev_data;
float ADCint = 4096.0/22.0;
float ADCinterval[22] = {ADCint*1, ADCint*2, ADCint*3, ADCint*4, ADCint*5, ADCint*6, ADCint*7, ADCint*8, ADCint*9, ADCint*10,
	 ADCint*11, ADCint*12, ADCint*13, ADCint*14, ADCint*15, ADCint*16, ADCint*17, ADCint*18, ADCint*19, ADCint*20, ADCint*21, ADCint*22};
int mc1flag = 0, mc2flag = 0;
bool ADCFlag = false;

	

// *********** Bomb_Init *************
// create all the objects for each bomb module
// Input: int array, indicating which module to create
// -1: Empty Module
//  0: Timer  (can only be in locations 1 or 4)
//  1: Wires
//  2: Button
//  3: Morse Code
//  4: Math
void Bomb_Init(int* types) {
	for (int i = 0; i < 12; i++) {
		switch (types[i]) {
			case -1:
				modules[i] = new Module(true);
				break;
			case 0: {
				modules[i] = new Timer(i+1);
				t = static_cast<Timer*> (modules[i]);
				timer_location = i+1;
				break;
			}
			case 1:
				if(num_wires < 3) {
					modules[i] = new Wires(i+1);
					if(num_wires == 0)
						w1 = static_cast<Wires*> (modules[i]);
					else
						w2 = static_cast<Wires*> (modules[i]);
					num_wires++;
				}
				break;
			case 2:
				if(num_button < 3) {
					modules[i] = new Button(i+1);
					if(num_button == 0)
						b1 = static_cast<Button*> (modules[i]);
					else
						b2 = static_cast<Button*>(modules[i]);
					num_button++;
				}
				break;
			case 3:
				if(num_MorseCode < 2) {
					modules[i] = new MorseCode(i+1);
					if(num_MorseCode == 0)
						mc1 = static_cast<MorseCode*>(modules[i]);
					else
						mc2 = static_cast<MorseCode*> (modules[i]);
					num_MorseCode++;
				}
				break;
			case 4:
				if(num_math < 3) {
					modules[i] = new Math(i+1);
					if(num_math == 0)
						m1 = static_cast<Math*> (modules[i]);
					else
						m2 = static_cast<Math*> (modules[i]);
					num_math++;
				}
				break;
			default:
				break;
		}
	}
}

int abs(int x) {
	if (x >= 0)
		return x;
	else
		return x*-1;
}

// convert an integer time (in seconds) to a string with the "m:s" format
char* convertTime(int t) {
	int minutes = t / 60;
	int seconds = t % 60;
	char* timeString = new char[5]; // "m:ss\0" requires 4 chars
	timeString[0] = '0' + minutes;
	timeString[1] = ':';
	timeString[2] = '0' + seconds / 10;
	timeString[3] = '0' + seconds % 10;
	timeString[4] = '\0'; // Null-terminate the string
	return timeString;
}

// ***************** Timer3A_Init ****************
// Activate Timer3 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
//          priority 0 (highest) to 7 (lowest)
// Outputs: none
void Timer3A_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
	PeriodicTask3 = task;
  TIMER3_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x10000000; // priority 1
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable timer3A
}

void Timer3A_Stop(void){
  NVIC_DIS1_R = 1<<(35-32);   // 9) disable interrupt 35 in NVIC
  TIMER3_CTL_R = 0x00000000;  // 10) disable timer3
}

void TIMER3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER2A timeout
	(*PeriodicTask3)();
}

void task3() {
	if(modules[b.getSelector()-1]->empty == false && GPIO_PORTF_DATA_R == 0x10 && noneSelected()) {
			while(GPIO_PORTF_DATA_R == 0x10) {}
			(*modules[b.getSelector()-1]).selectModule();
			screen = 4;
			selectorclicked = true;
		}
		else if(modules[b.getSelector()-1]->empty == false && GPIO_PORTF_DATA_R == 0x10 && !noneSelected()) {
			while(GPIO_PORTF_DATA_R == 0x10) {}
			(*modules[b.getSelector()-1]).deselectModule();
			screen = 1;
		}
	
}

void PortF_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x20; 
	while((SYSCTL_PRGPIO_R&0x20) == 0){};
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

void SysTick_Init(uint32_t period){
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x80000000; // priority 0        
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
  // enable interrupts after all initialization is finished
}

void SysTick_Handler(void){	
		 if(!modules[5]->selected)
			 ADCFlag = false;	 
}

int main(void){
	DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  // TExaS_Init();
  Output_Init();
	Key_Init();
	PortF_Init();
	ADC_Init();
	//SysTick_Init(8000000);
	//Timer0_Init(&task0, 80000000);
	//Timer1_Init(&task1, 8000000);
	//Timer3A_Init(&task3, 8000000);
	Wave_Init();
	ST7735_InvertDisplay(1);
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_DrawBitmap(4,80,title,120,37);
	ST7735_SetTextColor(ST7735_WHITE);
	ST7735_SetCursor(6,10);
	ST7735_OutString((char*)"English");
	ST7735_SetCursor(6,12);
	ST7735_OutString((char*)"Espa\xA4ol");
	EnableInterrupts();
	screen = 2;
	/*while (screen == 0) {
		switch (Key_In()) {
			case 0x01: {
				while (Key_In() != 0) {}
				screen = 1;
				break;
			}
			case 0x02: {
				while (Key_In() != 0) {}
				language = 1;
				screen = 1;
				break;
			}
		};
	}
	EnableInterrupts();
	Bomb_Init(setUp);
	ST7735_FillScreen(ST7735_BLACK);
	modules[timer_location-1]->drawModule(timer_location);
	modules[timer_location-1]->Draw(timer_location);*/
	/*while (screen != 0 || screen != 2 || screen != 3) {
		if (noneSelected()) 
			screen = 1;
		else if (!noneSelected())
			screen = 4;
		
		if (b.isSolved()) {
			modules[timer_location-1]->drawModule(timer_location);
			modules[timer_location-1]->Draw(timer_location);
			screen = 2;
			break;
		}
		if (t->flag) {
				b.total_time--;
				t->flag = 0;
				if (b.total_time == 0)
					b.failure();
				char* ts = convertTime(b.total_time);
				modules[timer_location-1]->outTime(ts);
				delete[] ts;
		}
		if (b.hasExploded()) {
				for (; anim >= 0; anim--) {
					ST7735_FillScreen(ST7735_BLACK);
					for (int i = 0; i < 5; i++)
						Delay1ms(1);
					ST7735_DrawBitmap(48,85,explosion[anim],32,32);
				}
				screen = 3;
				break;
			}
		
		if (screen == 1) {	
			b.drawStrike(t->getloc());		
			if(selectormoved && noneSelected()) {
				char* ts = convertTime(b.total_time);
				modules[timer_location-1]->drawModule(timer_location);
				modules[timer_location-1]->Draw(timer_location);
				modules[timer_location-1]->outTime(ts);
				delete[] ts;
				for (int i = 0; i < 6; i++) {
					modules[i]->Draw(i+1);		
					}
				static_cast<MorseCode*>(modules[5])->outFreq();
				b.drawStrike(t->getloc());			
				b.drawSelector();
				selectormoved = false;
			}
			
			else if(noneSelected())
				b.drawSelector();
			
		} else if (screen == 4 && selectorclicked == true) {
			
			ST7735_FillScreen(ST7735_BLACK);
			modules[timer_location-1]->drawModule(timer_location);
			modules[timer_location-1]->Draw(timer_location);
			b.drawStrike(t->getloc());
			for (int i = 0; i < 6; i++) {
				modules[i]->Draw(i+1);
			}
			static_cast<MorseCode*>(modules[5])->outFreq();
			if(modules[5] -> selected) {
					static_cast<MorseCode*>(modules[5])->flashLED();
					ADCFlag = true;
			}
			
			selectorclicked = false;
		} else if(ADCFlag == true) {
			b.drawStrike(t->getloc());
			for (int j = 0; j < 22; j++) {
				
				data = ADC_In();
				if (data >= ADCinterval[j] && data <= ADCinterval[j+1]) {
					
					static_cast<MorseCode*>(modules[5])->setFreq(3600 - j*5);
					static_cast<MorseCode*>(modules[5])->outFreq();
					break;
				}
			 }
			
		}
		
	}*/
	

	if (screen == 2) {
		Wave_complete();
		ST7735_FillScreen(ST7735_BLACK);
		if (language == 1) {
			ST7735_DrawString(4,5,(char*)"FELICIDADES", ST7735_WHITE);
			ST7735_DrawString(5,8,(char*)"PUNTAJE: ", ST7735_WHITE);
			ST7735_SetCursor(12,8);
			ST7735_OutUDec(b.total_time*2);
			// Spanish
		} else {
			ST7735_DrawString(3,5,(char*)"CONGRATULATIONS", ST7735_WHITE);
			ST7735_DrawString(5,8,(char*)"SCORE: ", ST7735_WHITE);
			ST7735_SetCursor(12,8);
			ST7735_OutUDec(b.total_time*2);
			// English
		}
	}
	
	if (screen == 3) {
		Wave_bExplosion();
		ST7735_FillScreen(ST7735_BLACK);
		if (language == 1) {
			ST7735_DrawString(3,5,(char*)"JUEGO TERMINADO", ST7735_RED);
			ST7735_DrawString(5,8,(char*)"PUNTAJE: 0", ST7735_RED);
			// Spanish
		} else {
			ST7735_DrawString(5,5,(char*)"GAME OVER", ST7735_RED);
			ST7735_DrawString(5,8,(char*)"Score: 0", ST7735_RED);
			// English
		}
	}
}
