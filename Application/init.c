/*
 * init.c
 *
 *  Created on: 11 Jul 2020
 *      Author: user
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/qei.h"
#include "driverlib/uart.h"
#include "uartstdio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "init.h"

void init(){
    //enable GPIO
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
      TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

       //Unlock GPIOD7 - Like PF0 its used for NMI - Without this step it doesn't work
       HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //In Tiva include this is the same as "_DD" in older versions (0x4C4F434B)
       HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
       HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0;
       //Set Pins to be PHA0 and PHB0
       GPIOPinConfigure(GPIO_PD6_PHA0);
       GPIOPinConfigure(GPIO_PD7_PHB0);

       //GPIO pin interrupt configure PD6
       GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
       GPIOIntRegister(GPIO_PORTD_BASE, PD6IntHandler);
       IntPrioritySet(INT_GPIOD, 5);
       GPIOIntEnable(GPIO_PORTD_BASE, GPIO_INT_PIN_6);


      // IntMasterEnable();
       initPWM();
       initQEI();
       initLoopControl();
}

void initPWM(void){

    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

       GPIOPinConfigure(GPIO_PF1_M1PWM5);
       GPIOPinConfigure(GPIO_PF3_M1PWM7);
       GPIOPinConfigure(GPIO_PF2_M1PWM6);

       GPIOPinTypePWM(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

       PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN|PWM_GEN_MODE_NO_SYNC);
       PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN|PWM_GEN_MODE_NO_SYNC);

       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, Period-1);
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, Period-1);

       PWMGenEnable(PWM1_BASE, PWM_GEN_2);
       PWMGenEnable(PWM1_BASE, PWM_GEN_3);

}

void initQEI(){
    //Set GPIO pins for QEI. PhA0 -> PD6, PhB0 ->PD7. I believe this sets the pull up and makes them inputs
       GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6 |  GPIO_PIN_7);

       //DISable peripheral and int before configuration
       QEIDisable(QEI0_BASE);
       QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);

       // Configure quadrature encoder, use an arbitrary top limit of 1000
       QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_NO_RESET  | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 0xFFFFFFFF);

       // Enable the quadrature encoder.
       QEIEnable(QEI0_BASE);

       //Set position to a middle value so we can see if things are working
       QEIPositionSet(QEI0_BASE, 0);
       pos = 0;
}

void initLoopControl(){
    //init value
      u           = 0;// pid value calculate
      pos_d       = 0; // set point
      error       = 0;// pos error
      error_i     = 0;// integral error
      error_d     = 0;// derivative error
      error_old   = 0;// previous pos error

      Kp = 600;
      Ki = 125; //125
      Kd = 75; //15
      antiWindup = 1;
      dt = 0.001;
      //timer0A trigger enable
      TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()*dt) -1);
      IntEnable(INT_TIMER0A);
      TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
      IntPrioritySet(INT_TIMER0A, 5);
      TimerEnable(TIMER0_BASE, TIMER_A);
}

void setDIR(uint8_t val){
   // UARTprintf("set direction \n");
    if(val == 0)
    {

        PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, false);
        PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);

    }
    else
    {
        PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
        PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);


    }
}
// calibrate
void setPWM(void){

    //rescale to dutycycle %

    int val = abs(u);
    val = (val*100.0)/65535.0;

    if(val < 10) val = 10;
    if(val > 95) val = 95;

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6 , val*Period/100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7 , val*Period/100);

//    UARTprintf("set PWM = %d", (val*Period/100));

}
