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

#define APP_PROFILING_TIMER_BASE		TIMERA0_BASE

void TimerBaseIntHandler(void)
{
    Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);

    v_TimerOverflows++;
}

int32_t InitializeTimer()
{
	Timer_IF_Init(PRCM_TIMERA0, APP_PROFILING_TIMER_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);

	Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);
	//Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);

	Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);

	v_TimerOverflows = 0;

	return 0;
}

int32_t StartTimer()
{
	MAP_TimerLoadSet(APP_PROFILING_TIMER_BASE, TIMER_A, 0xFFFF);
	//
	// Enable the GPT
	//
	MAP_TimerEnable(APP_PROFILING_TIMER_BASE, TIMER_A);

	return 0;
}

int32_t StopTimer()
{
	MAP_TimerDisable(APP_PROFILING_TIMER_BASE, TIMER_A);

	return 0;
}

int32_t GetTimeDuration(float* pfDurationMilli)
{
	uint16_t ulCounter;

	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
	ulCounter = 0xFFFFFFFF - ulCounter;
	//*pfDurationMilli = /*(v_TimerOverflows * 4294967296.0 / 80000.0) +*/ ((float_t)ulCounter / 80000.0); //in milli sec
	*pfDurationMilli = (v_TimerOverflows * 65536 / 80000.0) + ((float_t)ulCounter / 80000.0); //in milli sec

	return 0;
}

