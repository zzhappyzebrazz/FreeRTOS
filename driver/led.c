/*
 * led.c
 *
 *  Created on: Mar 16, 2018
 *      Author: Quoc Bao
 */
#include "led.h"
#include "driverlib/gpio.h"

const uint32_t      ledSetVal[3] = {1<<1,1<<2,1<<3};
const   uint32_t    ledPin[3]   =   {GPIO_INT_PIN_1,GPIO_INT_PIN_2,GPIO_INT_PIN_3};
void ledInit(void)
{
    SysCtlPeripheralEnable(LED_GPIO_PERIPH);
    GPIOPinTypeGPIOOutput(LED_GPIO_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

}
void    ledControl(enum ledNumber led, enum ledState State)
{
    if (State)  GPIOPinWrite(LED_GPIO_BASE,ledPin[led], ledPin[led]);
    else GPIOPinWrite(LED_GPIO_BASE,ledPin[led], 0);
}


