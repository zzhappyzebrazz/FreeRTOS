/*
 * led.h
 *
 *  Created on: Mar 16, 2018
 *      Author: Quoc Bao
 */

#ifndef LED_H_
#define LED_H_
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#define LED_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define LED_GPIO_BASE       GPIO_PORTF_BASE


enum ledNumber {LEDRED = 0, LEDGREEN, LEDBLUE};
enum ledState  {OFF=0,ON=1};
void ledInit(void);
void    ledControl(enum ledNumber led, enum ledState State);




#endif /* LED_H_ */
