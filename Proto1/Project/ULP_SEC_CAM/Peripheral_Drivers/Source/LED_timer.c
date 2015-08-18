/*
 * LED_timer.c
 *
 *  Created on: 31-Jul-2015
 *      Author: Chrysolin
 */
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "timer.h"
#include "timer_fns.h"
#include "math.h"

#include "simplelink.h"

#include "app.h"
#include <stdbool.h>

#include "LED_Timer.h"

uint8_t cnt_LED_Timer;
volatile int8_t g_BlinkCount;

void IntrptHandler_LEDTimer()
{
	Timer_IF_InterruptClear(TIMERA2_BASE);
	cnt_LED_Timer++;
	if(cnt_LED_Timer == (g_OnTime - 1))
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off
		//DEBG_PRINT("Off");
	}
	else if(cnt_LED_Timer == (g_OffTime + g_OnTime - 1))
	{
		MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
		cnt_LED_Timer = 0;
		//DEBG_PRINT("On");
		g_BlinkCount++;
		if(g_BlinkCount == g_NoOfBlinks)
		{
			if(g_NoOfBlinks != BLINK_FOREVER)
			{
				//DEBG_PRINT("Timer off");
				LEDTimer_Stop();
			}
			else
			{
				//DEBG_PRINT("Blink forever");
			}
		}
	}
}
int32_t LEDTimer_Enable()
{
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralReset(PRCM_TIMERA2);

	MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_PERIODIC);
	MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_A, 0);

	Timer_IF_IntSetup(TIMERA2_BASE, TIMER_A, IntrptHandler_LEDTimer);

	return 0;
}
int32_t LEDTimer_Stop()
{
	MAP_TimerDisable(TIMERA2_BASE, TIMER_A);

	return 0;
}
int32_t LEDTimer_Start()
{
	cnt_LED_Timer = 0;
	g_BlinkCount = 0;

	MAP_TimerLoadSet(TIMERA2_BASE,TIMER_A,MILLISECONDS_TO_TICKS(100));
	MAP_TimerEnable(TIMERA2_BASE,TIMER_A);

	return 0;
}
int32_t LEDTimer_Disable()
{
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);

	return 0;
}

