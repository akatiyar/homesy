/*
 * timer_fns.c
 *
 *  Created on: 21-Apr-2015
 *      Author: Chrysolin
 */
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "timer.h"
#include "timer_fns.h"
#include "math.h"

#include "simplelink.h"

#include "app_common.h"
#include <stdbool.h>
#define APP_PROFILING_TIMER_BASE		TIMERA0_BASE

//#define RELOADVAL_100MILLISEC			0x007A11FF	//(100m*80,000,000-1)
#define RELOADVAL_100MILLISEC			100		//in milli sec
#define RELOADVAL_1SEC					1000	//in milli sec

#define CAMERA_CAPTURE_TIMEOUT			2		//in sec

void ISR_periodicInterrupt_timer();
void IntHandler_100mSecTimer(void);
void IntHandler_1Sec_TimeoutTimer(void);

uint32_t g_ulReloadVal;
//******************************************************************************
// 100ms timer. TimerA0 is used
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
void IntHandler_100mSecTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_100MilliSecs++;
	checkForLight_Flag = 1;
	//DEBG_PRINT("%d ",Elapsed_100MilliSecs);

//	if(Elapsed_100MilliSecs%2)
//	{
//		LED_On();
//	}
//	else
//	{
//		LED_Off();
//	}
}
//	Use this function along with 100 milli sec timer only. Before stopping timer
uint32_t get_timeDuration()
{
	uint32_t ulCounter;
	uint32_t ulDurationMilliSec;

	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
	ulCounter = MILLISECONDS_TO_TICKS(RELOADVAL_100MILLISEC) - ulCounter;
	ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 100.0) + ((float_t)ulCounter / 80000.0); //in milli sec
	//ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 1000.0) + ((float_t)ulCounter / 8000.0); //in .1 milli sec

	return ulDurationMilliSec;
}
int32_t stop_100mSecTimer()
{
	//DEBG_PRINT("100msTimr Stop\n");
	//DEBG_PRINT("e.1\n");
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	//DEBG_PRINT("e.2\n");
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
	//DEBG_PRINT("e.3\n");
	return 0;
}


//******************************************************************************
// 100ms timer. TimerA0 is used
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
void IntHandler_1Sec_TimeoutTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_1Secs++;

	if(Elapsed_1Secs == CAMERA_CAPTURE_TIMEOUT)
	{
		captureTimeout_Flag  = 1;
	}

}
int32_t stop_1Sec_TimeoutTimer()
{
	//DEBG_PRINT("1sTimr Stop\n");
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);

	return 0;
}


//******************************************************************************
// This timer issues a periodic interrupt and sets a flag, period can be
//specified. Use this for actions that have to be done periodically like reading
//data from a peripheral.
//Uses TimerA1
//******************************************************************************
int32_t start_periodicInterrupt_timer(float_t f_InterruptInterval_ms)
{

	DEBG_PRINT("PerTimr Start\n");
	Timer_IF_Init(PRCM_TIMERA1, TIMERA1_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA1_BASE, TIMER_A, ISR_periodicInterrupt_timer);

	//Timer_IF_Start(TIMERA1_BASE, TIMER_A, RELOADVAL_1SEC);
	g_ulReloadVal = (uint32_t)(f_InterruptInterval_ms * (float_t)SYSTEM_CLOCK/1000.0F);
	MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,g_ulReloadVal);
	MAP_TimerEnable(TIMERA1_BASE,TIMER_A);

	doPeriodicAction_flag = 0;
//	Elapsed_100MilliSecs = 0;//Rm

	return 0;
}
void ISR_periodicInterrupt_timer()
{
	//DEBG_PRINT("i");
	/*if(doPeriodicAction_flag != 0)
	{
		DEBG_PRINT("Watchout!\n");
	}*/
	doPeriodicAction_flag = 1;
	Timer_IF_InterruptClear(TIMERA1_BASE);
//	DEBG_PRINT("i");
//	Elapsed_100MilliSecs++;//Rm
//	if(Elapsed_100MilliSecs >= 2000)
//	{
//		DEBG_PRINT("Stp timer");
//		stop_periodicInterrupt_timer();
//	}
}
int32_t reload_periodicTimer()
{
	MAP_TimerLoadSet(TIMERA1_BASE,TIMER_A,g_ulReloadVal);
	doPeriodicAction_flag = 0;
	return 0;
}
int32_t stop_periodicInterrupt_timer()
{
	DEBG_PRINT("PerTimr Start\n");
	Timer_IF_Stop(TIMERA1_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);

	return 0;
}


