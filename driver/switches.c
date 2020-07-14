/*
 * switches.c
 *
 *  Created on: Mar 16, 2018
 *      Author: Quoc Bao
 */
#include "switches.h"
void        switchInit(void)
{
    //
    // Enable the GPIO port to which the sensors are connected.
    //
    ROM_SysCtlPeripheralEnable(SW_GPIO_PERIPH);

    //
    // Unlock PF0 so we can change it to a GPIO input
    // Once we have enabled (unlocked) the commit register then re-lock it
    // to prevent further changes.  PF0 is muxed with NMI thus a special case.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    //
    // Set each of the button GPIO pins as an input with a pull-up.
    //
    ROM_GPIODirModeSet(SW_GPIO_BASE, SW1_PIN | SW2_PIN, GPIO_DIR_MODE_IN);
    ROM_GPIOPadConfigSet(SW_GPIO_BASE, SW1_PIN | SW2_PIN,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

}
sw_t        switchState(int SWnumber)
{
    switch    (SWnumber)
        {
            case 1:
                if (GPIOPinRead(SW_GPIO_BASE, SW1_PIN) == 0) return PRESSED;
                else return RELEASED;

            case 2:
                if (GPIOPinRead(SW_GPIO_BASE, SW2_PIN) == 0) return PRESSED;
                else return RELEASED;

        }
}


