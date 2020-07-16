/*
 * init.h
 *
 *  Created on: 11 Jul 2020
 *      Author: user
 */

#ifndef APPLICATION_INIT_H_
#define APPLICATION_INIT_H_

volatile unsigned long Period;

volatile uint16_t uMax;
volatile uint16_t uMin;
volatile int16_t pos;

volatile int16_t pos_d;
volatile float error;
volatile float u;
volatile int Kp,Kd,Ki;
volatile float error_i,error_d,error_old;
volatile float dt;
volatile int antiWindup;

void initPWM(void);
void initQEI(void);
void init(void);
void initLoopControl(void);
void PD6IntHandler(void);

void setDIR(uint8_t val);
void setPWM(void);

#endif /* APPLICATION_INIT_H_ */
