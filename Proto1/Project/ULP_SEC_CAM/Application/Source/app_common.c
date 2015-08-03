#include "app_common.h"
#include "math.h"
const char pcDigits[] = "0123456789"; /* variable used by itoa function */

#include "LED_Timer.h"
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
unsigned short intToASCII(long long cNum, char *cString)	//Tag:CS
{
    char* ptr;
    //short uTemp = cNum;
    long long uTemp = cNum;		//Tag:CS
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';
        *++cString = '\0';
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
    *ptr = '\0';			//Tag:CS
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return (length+1);	//Tag:CS
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
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	LED_Blink_2(1,0,0);
}

void LED_Off()
{
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
	LED_Blink_2(0,1,0);
}


//******************************************************************************
//Blink LED using timer - specifying different on and off times and the number
//of blinks
//******************************************************************************
void LED_Blink_2(float_t fOnTime_inSecs, float_t fOffTime_inSecs, int8_t ucHowManyTimes)
{
	LEDTimer_Stop();	//Stopping the Timer, in case it is already running
						//(ie previous LED blink is happening)

	//Keep the LED off
	if(fOnTime_inSecs == 0)
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);	//LED off
	}
	//Keep the LED on
	else if (fOffTime_inSecs == 0)
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	}
	//Blink LED
	else
	{
		//Set on time, off time and number of blinks
		g_OffTime = 10.0*fOffTime_inSecs;
		g_OnTime = 10.0*fOnTime_inSecs;
		g_NoOfBlinks = ucHowManyTimes;

		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//Begin with LED on

		//Start the timer
		LEDTimer_Start();
	}
}
