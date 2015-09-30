/*
 * timer_fns.c
 *
 *  Created on: 21-Apr-2015
 *      Author: Chrysolin
 */
#include <stdbool.h>

#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "timer.h"
#include "timer_fns.h"
#include "math.h"

#include "simplelink.h"

#include "app_common.h"

#define APP_PROFILING_TIMER_BASE		TIMERA0_BASE
//#define RELOADVAL_100MILLISEC			0x007A11FF	//(100m*80,000,000-1)
#define RELOADVAL_100MILLISEC			100		//in milli sec
#define RELOADVAL_1SEC					1000	//in milli sec
#define CAMERA_CAPTURE_TIMEOUT			2		//in sec

uint32_t g_ulReloadVal;

void ISR_periodicInterrupt_timer();
void IntHandler_100mSecTimer(void);
void IntHandler_1Sec_TimeoutTimer(void);

//* * * * * * * * * * * * 100ms timer. TimerA0 is used * * * * * * * * * * * * *

//******************************************************************************
//	Start the 100ms timer
//
//	Enable clock, configure timer, set reload value and interrupt handler and
//	start timer
//******************************************************************************
int32_t start_100mSecTimer()
{
	//DEBG_PRINT("100msTimr Start\n");
	Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, IntHandler_100mSecTimer);
	Timer_IF_Start(TIMERA0_BASE, TIMER_A, RELOADVAL_100MILLISEC);
	Elapsed_100MilliSecs = 0;

	return 0;
}

//******************************************************************************
//	100ms interrupt handler
//
//	Clear interrupt, set a bit, increase the number of 100 milliseconds lapsed
//******************************************************************************
void IntHandler_100mSecTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_100MilliSecs++;
	checkForLight_Flag = 1;
}

//******************************************************************************
//	Get the time duration since the start of 100ms timer
//
//	return	duration lapsed since start of timer in milli seconds
//
//	Use this function along with 100 milli sec timer only. Before stopping timer
//******************************************************************************
uint32_t get_timeDuration()
{
	uint32_t ulCounter;
	uint32_t ulDurationMilliSec;

	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
	ulCounter = MILLISECONDS_TO_TICKS(RELOADVAL_100MILLISEC) - ulCounter;
	ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 100.0) + ((float_t)ulCounter / 80000.0); //in milli sec
	//ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 1000.0) + ((float_t)ulCounter / 8000.0); //in .1 milli sec
	//ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 100000.0) + ((float_t)ulCounter / 80.0); //in .001 milli sec

	return ulDurationMilliSec;
}

//******************************************************************************
//	Stop the 100ms timer
//
//	Stop timer and disable clock to the timer peripheral
//******************************************************************************
int32_t stop_100mSecTimer()
{
	//DEBG_PRINT("100msTimr Stop\n");
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
	return 0;
}

//* * * * * * * * * * * * * 1 s timer. TimerA0 is used * * * * * * * * * * * * *

//******************************************************************************
//	Start the 1s timer
//
//	Enable clock, configure timer, set reload value and interrupt handler and
//	start timer
//******************************************************************************
int32_t start_1Sec_TimeoutTimer()
{
	//DEBG_PRINT("1sTimr Start\n");
	Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, IntHandler_1Sec_TimeoutTimer);
	Timer_IF_Start(TIMERA0_BASE, TIMER_A, RELOADVAL_1SEC);
	Elapsed_100MilliSecs = 0;

	return 0;
}

//******************************************************************************
//	1s interrupt handler
//
//	Clear interrupt, set a bit if camera timeout happened, increase the number
//	of seconds lapsed
//******************************************************************************
void IntHandler_1Sec_TimeoutTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_1Secs++;

	if(Elapsed_1Secs == CAMERA_CAPTURE_TIMEOUT)
	{
		captureTimeout_Flag  = 1;
	}
}

//******************************************************************************
//	Stop the 1s timer
//
//	Stop timer and disable clock to the timer peripheral
//******************************************************************************
int32_t stop_1Sec_TimeoutTimer()
{
	//DEBG_PRINT("1sTimr Stop\n");
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);

	return 0;
}

//* * * * * * * * * Generic periodic timer. TimerA1 is used * * * * * * * * * *
// 	This timer issues a periodic interrupt and sets a flag
//	Period can be specified
//	Use this for actions that have to be done periodically like reading data
//	from a peripheral device. Uses TimerA1
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

//******************************************************************************
//	Start the periodic timer
//
//	param	period of the timer in milli seconds
//
//	Enable clock, configure timer, set reload value and interrupt handler and
//	start timer
//******************************************************************************
int32_t start_periodicInterrupt_timer(float_t f_InterruptInterval_ms)
{
	//DEBG_PRINT("PerTimr Start\n");
	Timer_IF_Init(PRCM_TIMERA1, TIMERA1_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA1_BASE, TIMER_A, ISR_periodicInterrupt_timer);
	g_ulReloadVal = (uint32_t)(f_InterruptInterval_ms * (float_t)SYSTEM_CLOCK/1000.0F);
	MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,g_ulReloadVal);
	MAP_TimerEnable(TIMERA1_BASE,TIMER_A);

	doPeriodicAction_flag = 0;

	return 0;
}

//******************************************************************************
//	Periodic timer interrupt handler
//
//	Clear interrupt, set a bit
//******************************************************************************
void ISR_periodicInterrupt_timer()
{
	doPeriodicAction_flag = 1;
	Timer_IF_InterruptClear(TIMERA1_BASE);
}

//******************************************************************************
//	Reload the period to the counter
//******************************************************************************
int32_t reload_periodicTimer()
{
	MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,g_ulReloadVal);
	doPeriodicAction_flag = 0;

	return 0;
}

//******************************************************************************
//	Stop the periodic timer
//
//	Stop timer and disable clock to the timer peripheral
//******************************************************************************
int32_t stop_periodicInterrupt_timer()
{
	//DEBG_PRINT("PerTimr Stop\n");
	Timer_IF_Stop(TIMERA1_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);

	return 0;
}

//* * * * * * * * * * * * * LED timer. TimerA2 is used * * * * * * * * * * * * *
//	It is a periodic timer with a period of 100 ms. Has a special interrupt used
//	only for blinking LED with different duty cycles and durations
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
uint8_t cnt_LED_Timer;
volatile int8_t g_BlinkCount;

//******************************************************************************
//	LED timer interrupt handler
//
//	Does the following:
//		Clear interrupt
//		Increments count
//		Toggles LED GPIO and reset count on end of on time or off time
//		If blink count is over, timer is stopped
//******************************************************************************
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

//******************************************************************************
//	Enables LED timer
//
//	Enable clock to the timer, configure timer and set up the interrupt handler
//******************************************************************************
int32_t LEDTimer_Enable()
{
	MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
	MAP_PRCMPeripheralReset(PRCM_TIMERA2);

	MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_PERIODIC);
	MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_A, 0);

	Timer_IF_IntSetup(TIMERA2_BASE, TIMER_A, IntrptHandler_LEDTimer);

	return 0;
}

//******************************************************************************
//	Stop the LED timer
//******************************************************************************
int32_t LEDTimer_Stop()
{
	MAP_TimerDisable(TIMERA2_BASE, TIMER_A);

	return 0;
}

//******************************************************************************
//	Start the LED timer
//
//	Reload value is set to 100ms and timer is started to run
//******************************************************************************
int32_t LEDTimer_Start()
{
	cnt_LED_Timer = 0;
	g_BlinkCount = 0;

	MAP_TimerLoadSet(TIMERA2_BASE,TIMER_A,MILLISECONDS_TO_TICKS(100));
	MAP_TimerEnable(TIMERA2_BASE,TIMER_A);

	return 0;
}

//******************************************************************************
//	Disable clock to LED timer
//******************************************************************************
int32_t LEDTimer_Disable()
{
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);

	return 0;
}
