/*
 * led.c
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "timer_fns.h"

//******************************************************************************
//Blink LED using timer
//
//Input parameters:
//	1. on duration
//	2. off duration
//	3. the number of blinks (Use BLINK_FOREVER if LED should blink for an
//		indefinite amount of time)
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

		//Begin with LED on
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);

		//Start the timer
		LEDTimer_Start();
	}
}

//******************************************************************************
//	This function switches LED on
//******************************************************************************
void LED_On()
{
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	LED_Blink_2(1,0,0);
}

//******************************************************************************
//	This function switches LED off
//******************************************************************************
void LED_Off()
{
	//MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
	LED_Blink_2(0,1,0);
}

#if 0
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
#endif
