//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "debug.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driver/led.h"
#include "driverlib/systick.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/qei.h"
#include "init.h"
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>FreeRTOS Example (freertos_demo)</h1>
//!
//! This application demonstrates the use of FreeRTOS on Launchpad.
//!
//! The application blinks the user-selected LED at a user-selected frequency.
//! To select the LED press the left button and to select the frequency press
//! the right button.  The UART outputs the application status at 115,200 baud,
//! 8-n-1 mode.
//!
//! This application utilizes FreeRTOS to perform the tasks in a concurrent
//! fashion.  The following tasks are created:
//!
//! - An LED task, which blinks the user-selected on-board LED at a
//!   user-selected rate (changed via the buttons).
//!
//! - A Switch task, which monitors the buttons pressed and passes the
//!   information to LED task.
//!
//! In addition to the tasks, this application also uses the following FreeRTOS
//! resources:
//!
//! - A Queue to enable information transfer between tasks.
//!
//! - A Semaphore to guard the resource, UART, from access by multiple tasks at
//!   the same time.
//!
//! - A non-blocking FreeRTOS Delay to put the tasks in blocked state when they
//!   have nothing to do.
//!
//! For additional details on FreeRTOS, refer to the FreeRTOS web page at:
//! http://www.freertos.org/
//
//*****************************************************************************


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************

static void EncoderTask(void *pvParameters);
static void PIDTask(void *pvParameters);
//static void LEDRedTask(void *pvParameters);

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************

xSemaphoreHandle Encoder_sem;
xSemaphoreHandle PID_sem;
xQueueHandle pos_queue = NULL;

void setDIR(uint8_t val);
void setPWM(void);

extern void PD6IntHandler(void){
    GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_6);
    long task_woken = 0;
    xSemaphoreGiveFromISR(Encoder_sem, &task_woken);
    if(task_woken){
        portYIELD_FROM_ISR(task_woken);
    }
}

void Timer0AIntHandler(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    long task_woken = 0;
    xSemaphoreGiveFromISR(PID_sem, &task_woken);
    if(task_woken){
        portYIELD_FROM_ISR(task_woken);
    }
}

int main(void)
{
        SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                           SYSCTL_OSC_MAIN);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        GPIOPinConfigure(GPIO_PA0_U0RX);
        GPIOPinConfigure(GPIO_PA1_U0TX);
        GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
        ConfigureUART();
        pos_queue = xQueueCreate( 10, sizeof( float ) );
        init();
        xTaskCreate( EncoderTask, "Encoder Task", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
        xTaskCreate( PIDTask,   "PID Task ", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
//        xTaskCreate( LEDGreenTask, "LEDTask 3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        vTaskStartScheduler();
    while(1)
    {
    }
}

static void EncoderTask(void *pvParameters)
{
    while (1)
    {
        if(xSemaphoreTake(Encoder_sem,portMAX_DELAY))
        {
            pos = QEIPositionGet(QEI0_BASE);
                if(pos >  1320){
                    pos = pos - 1320;
                    UARTprintf("reset positive \n");
                }
                if(pos < -1320){
                    pos = pos + 1320;
                    UARTprintf("reset negative \n");
                }
                UARTprintf("%d \n" ,QEIPositionGet(QEI0_BASE));
                if(pos == 0)
                {
                    UARTprintf("DONT MOVE\n");
                }
           long ok = xQueueSend(pos_queue, &pos, 100);
           if(ok)
           {
               UARTprintf("queue sended");
           }
        }
    }
}


static void PIDTask(void *pvParameters)
{
    while (1)
    {
        if(xSemaphoreTake(Encoder_sem,portMAX_DELAY))
        {
            if(xQueueReceive(pos_queue, &pos, 100))
                if(pos >= -2 && pos <= 2){
                        PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, false);
                        PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, false);
                    }
                    else {
                       error = pos_d - pos;
                       error_d = (error - error_old)/dt;
                       u = Kp*error + Ki*error_i + Kd*error_d;

                       if(abs(u) >= 65535 && (((error >= 0) && (error_i >= 0)) || ((error <0) && (error_i < 0))))
                       {
                           if(antiWindup)
                           {
                               error_i = error_i;
                           }
                           else // if no ani windup
                           {
                               error_i = error_i + dt*1.0*error; // rectangular integration
                           }
                       }
                       else
                       {
                           error_i = error_i + dt*1.0*error;
                       }

                       error_old = error; // store for next calculate
                       int temp = abs(u);
                       if(u>=0)
                       {
                //           UARTprintf("u value %d \n", u);
                //           UARTprintf("u value abs %d \n", temp);
                           setDIR(0);
                           setPWM();
                       }
                       else
                       {
                //           UARTprintf("u value %d \n", u);
                //           UARTprintf("u value abs %d \n", temp);
                           setDIR(1);
                           setPWM();
                       }
                    }
        }
    }
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
