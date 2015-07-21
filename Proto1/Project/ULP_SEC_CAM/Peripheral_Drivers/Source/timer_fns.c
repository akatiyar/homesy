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

#include "app_common.h"
#include <stdbool.h>
#define APP_PROFILING_TIMER_BASE		TIMERA0_BASE

//#define RELOADVAL_100MILLISEC			0x007A11FF	//(100m*80,000,000-1)
#define RELOADVAL_100MILLISEC			100		//in milli sec
#define RELOADVAL_1SEC					1000	//in milli sec

#define CAMERA_CAPTURE_TIMEOUT			2		//in sec

void IntHandler_100mSecTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_100MilliSecs++;
	checkForLight_Flag = 1;
	//UART_PRINT("%d ",Elapsed_100MilliSecs);

//	if(Elapsed_100MilliSecs%2)
//	{
//		LED_On();
//	}
//	else
//	{
//		LED_Off();
//	}
}

int32_t start_100mSecTimer()
{
	Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, IntHandler_100mSecTimer);
	Timer_IF_Start(TIMERA0_BASE, TIMER_A, RELOADVAL_100MILLISEC);
	Elapsed_100MilliSecs = 0;

	return 0;
}

int32_t stop_100mSecTimer()
{
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);

	return 0;
}

//	Use this function along with 100 milli sec timer only. Before stopping timer
uint32_t get_timeDuration()
{
	uint32_t ulCounter;
	uint32_t ulDurationMilliSec;

	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
	ulCounter = MILLISECONDS_TO_TICKS(RELOADVAL_100MILLISEC) - ulCounter;
	ulDurationMilliSec = ((float_t)Elapsed_100MilliSecs * 100.0) + ((float_t)ulCounter / 80000.0); //in milli sec

	return ulDurationMilliSec;
}


void IntHandler_1Sec_TimeoutTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_1Secs++;

	if(Elapsed_1Secs == CAMERA_CAPTURE_TIMEOUT)
	{
		PRCMSOCReset();
	}

}

int32_t start_1Sec_TimeoutTimer()
{
	Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
	Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, IntHandler_1Sec_TimeoutTimer);
	Timer_IF_Start(TIMERA0_BASE, TIMER_A, RELOADVAL_1SEC);
	Elapsed_100MilliSecs = 0;

	return 0;
}

int32_t stop_1Sec_TimeoutTimer()
{
	Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
	MAP_PRCMPeripheralClkDisable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);

	return 0;
}
