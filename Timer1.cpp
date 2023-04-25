// Timer1.c
// Runs on LM4F120/TM4C123
// Use TIMER1 in 32-bit periodic mode to request interrupts at a periodic rate
// Daniel Valvano
// Last Modified: 1/17/2020 
// You can use this timer only if you learn how it works

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
  Program 7.5, example 7.6

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include <stdint.h>
#include "Timer1.h"
#include "../inc/tm4c123gh6pm.h"
#include "Module.h"

void (*PeriodicTask1)(void);   // user function
extern "C" void TIMER1A_Handler(void);
extern Module* modules[12];
extern Bomb b;
extern int screen;
int prev_sel = b.getSelector();
extern Wires* w1;
extern Wires* w2;
extern Wires* w3;
extern Math* m1;
extern Math* m2;
extern Math* m3;
extern Button* b1;
extern Button* b2;
extern Button* b3;
extern MorseCode* mc1;
extern MorseCode* mc2;
extern MorseCode* mc3;
extern Timer* t;
int pressed = 0;
int waited = 0;
int presses[3] = {1, 10, 7};
bool onetime = false;
bool tentime = false;
int answer;
extern bool selectormoved;
bool noneSelected();

// ***************** TIMER1_Init ****************
// Activate TIMER1 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer1_Init(void(*task)(void), uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x02;   // 0) activate TIMER1
  PeriodicTask1 = task;          // user function
  TIMER1_CTL_R = 0x00000000;    // 1) disable TIMER1A during setup
  TIMER1_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER1_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER1_TAILR_R = period-1;    // 4) reload value
  TIMER1_TAPR_R = 0;            // 5) bus clock resolution
  TIMER1_ICR_R = 0x00000001;    // 6) clear TIMER1A timeout flag
  TIMER1_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00008000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 37, interrupt number 21
  NVIC_EN0_R = 1<<21;           // 9) enable IRQ 21 in NVIC
  TIMER1_CTL_R = 0x00000001;    // 10) enable TIMER1A
}

void TIMER1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
  (*PeriodicTask1)();                // execute user task
}

void task1() {
		if (screen == 1 && noneSelected()) {
			switch (Key_In()) {
				case 0x01:
					while (Key_In() != 0) {}
					b.moveSelector(0);
					selectormoved = true;
					break;
				case 0x02:
					while (Key_In() != 0) {}
					b.moveSelector(1);
					selectormoved = true;
					break;
				case 0x04:
					while (Key_In() != 0) {}
					b.moveSelector(2);
					selectormoved = true;
					break;
				case 0x08:
					while (Key_In() != 0) {}
					b.moveSelector(3);
					selectormoved = true;
					break;
				default:
					break;
			};
		} 
		else if(screen ==4) {
			if (w1 != nullptr && w1->selected) {
			switch (Key_In()) {
				case 0x01:
					while (Key_In() != 0) {}
					w1->moveSelector(0);
					break;
				case 0x02:
					while (Key_In() != 0) {}
					w1->moveSelector(1);
					break;
				case 0x04:
					while (Key_In() != 0) {}
					w1->cut();
					break;
				case 0x08:
					while (Key_In() != 0) {}
					w1->cut();
					break;
			}
		} else if(w2 != nullptr && w2->selected) {
			switch (Key_In()) {
				case 0x01:
					while (Key_In() != 0) {}
					w2->moveSelector(0);
					break;
				case 0x02:
					while (Key_In() != 0) {}
					w2->moveSelector(1);
					break;
				case 0x04:
					while (Key_In() != 0) {}
					w2->cut();
					break;
				case 0x08:
					while (Key_In() != 0) {}
					w2->cut();
					break;
			}
		} else if (m1 != nullptr && m1->selected) {
			switch (Key_In()) {
				case 0x01:
					while (Key_In() != 0) {}
					m1->select();
					break;
				case 0x02:
					while (Key_In() != 0) {}
					m1->select();
					break;
				case 0x04:
					while (Key_In() != 0) {}
					m1->moveSelector(0);
					break;
				case 0x08:
					while (Key_In() != 0) {}
					m1->moveSelector(1);
					break;
			}
		} else if (m2 != nullptr && m2->selected) {
			switch (Key_In()) {
				case 0x01:
					while (Key_In() != 0) {}
					m2->select();
					break;
				case 0x02:
					while (Key_In() != 0) {}
					m2->select();
					break;
				case 0x04:
					while (Key_In() != 0) {}
					m2->moveSelector(0);
					break;
				case 0x08:
					while (Key_In() != 0) {}
					m2->moveSelector(1);
					break;
			}
		}  else if(mc1 != nullptr && mc1->selected && Key_In() == 1) {
				mc1->check();
		} else if(mc2 != nullptr && mc2->selected && Key_In() == 1) {
			mc2->check();
		}
		else if (b1 != nullptr && b1->selected && Key_In() == 1) {
			answer = presses[b1->determineAnswer()];
			if(answer == 1) {
				onetime = true;
				pressed++;
			}
			else if(answer == 10) {
				pressed++;
				tentime = true;
				if(pressed == 30) {
					b1->solved = true;
					b1->drawSolved(b1->getLocation());
					b.num_solved++;
					tentime = false;
					b1->selected = false;
					screen = 1;
				}
			}
			else if(answer == 7) {
				if(waited > 70) {
					b1->solved = true;
					b1->drawSolved(b1->getLocation());
					b.num_solved++;
					b1->selected = false;
					screen  = 1;
				}
				else {
					b.addStrikes();
				}
			}
		}
		else if(b1 != nullptr && b1->selected && onetime == true) {
				if(pressed > 2) {
					//do something to signify its wrong
					b.addStrikes();
				}
				else if(pressed < 3) {
					b1->solved = true;
					b1->drawSolved(b1->getLocation());
					b.num_solved++;
					b1->selected = false;
					screen = 1;
				}
				onetime = false;
		}
		else if(b1 != nullptr && b1->selected && tentime == true) {
				b.addStrikes();
				tentime = false;
		}
		else if(b1 != nullptr && b1->selected) {
			waited++;
			pressed = 0;
		} 
		else if (b2 != nullptr && b2->selected && Key_In() == 1) {
			answer = presses[b2->determineAnswer()];
			if(answer == 1) {
				onetime = true;
				pressed++;
			}
			else if(answer == 10) {
				pressed++;
				tentime = true;
				if(pressed == 30) {
					b2->solved = true;
					b2->drawSolved(b2->getLocation());
					b2->selected = false;
					b.num_solved++;
					tentime = false;
					screen = 1;
				}
			}
			else if(answer == 7) {
				if(waited > 70) {
					b2->solved = true;
					b2->drawSolved(b2->getLocation());
					b.num_solved++;
					b2->selected = false;
					screen = 1;
				}
				else {
					b.addStrikes();
				}
			}
		}
		else if(b2 != nullptr && b2->selected && onetime == true) {
				if(pressed > 2) {
					b.addStrikes();
				}
				else if(pressed < 3) {
					b2->solved = true;
					b2->drawSolved(b2->getLocation());
					b.num_solved++;
					b2->selected = false;
					screen = 1;
				}
				onetime = false;
		}
		else if(b2 != nullptr && b2->selected && tentime == true) {
				b.addStrikes();
				tentime = false;
		}
		else if(b2 != nullptr && b2->selected) {
			waited++;
			pressed = 0;
		}
		
		else {
			pressed = 0;
			waited = 0;
			tentime = false;
			onetime = false;
		}
}
}

bool noneSelected() {
	for (int i = 0; i < 12; i++) {
		if (modules[i]->selected == true)
			return false;
	}
	return true;
}
