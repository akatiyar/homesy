#include "app.h"
#include "math.h"
const char pcDigits[] = "0123456789"; /* variable used by itoa function */

//*****************************************************************************
//	This function has been taken from CC3200 SDK network_if.c
//! itoa
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters
//!
//!
//
//*****************************************************************************
unsigned short intToASCII(short cNum, char *cString)
{
    char* ptr;
    short uTemp = cNum;
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';

        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}

//******************************************************************************
//	This function blinks the LED
//******************************************************************************
void LED_Blink(uint8_t ucHowManyTimes, float_t fSecsForEachCycle)
{
	    uint8_t i;
	    for(i=0;i<ucHowManyTimes;i++)
	    {
	    	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	    	UtilsDelay((fSecsForEachCycle/2.0)*80000000/6);
	    	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
	    	UtilsDelay((fSecsForEachCycle/2.0)*80000000/6);
	    }
}

void LED_On()
{
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
}

void LED_Off()
{
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
}
