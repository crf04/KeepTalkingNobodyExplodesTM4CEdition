// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014
#ifndef __SOUND_H__ // do not include more than once
#define __SOUND_H__
#include <stdint.h>

void Sound_Init(void);
void Sound_Play(const uint8_t *pt, uint32_t count);
void Sound_Shoot(void);
void Sound_Killed(void);
void Sound_Explosion(void);

void Sound_Fastinvader1(void);
void Sound_Fastinvader2(void);
void Sound_Fastinvader3(void);
void Sound_Fastinvader4(void);
void Sound_Highpitch(void);

void Timer2A_Init(uint32_t period, uint32_t priority);
void Timer2A_Start(void);
void Timer2A_Handler(void);
void Wave_Start(const uint8_t *pt, uint32_t count);
void Wave_Stop(void);
void Wave_Init(void);
void Wave_bExplosion(void);
void Wave_complete(void);

#endif


