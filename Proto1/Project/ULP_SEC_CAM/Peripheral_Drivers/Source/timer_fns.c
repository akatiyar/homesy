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
#define RELOADVAL_100MILLISEC			100


void IntHandler_100mSecTimer(void)
{
	Timer_IF_InterruptClear(TIMERA0_BASE);
	Elapsed_100MilliSecs++;
	checkForLight_Flag = 1;
	UART_PRINT("%d ",Elapsed_100MilliSecs);

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

//void TimerBaseIntHandler(void)
//{
//    Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);
//
//    v_TimerOverflows++;
//
//    if( v_TimerOverflows >= (10 * 80000000 / 65536) )
//    {
//    	v_OneSecFlag = 1;
//    }
//}
//
//int32_t InitializeTimer()
//{
//	Timer_IF_Init(PRCM_TIMERA0, APP_PROFILING_TIMER_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
//
//	Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);
//	//Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);
//
//	Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);
//
//	v_TimerOverflows = 0;
//	v_OneSecFlag = 0;
//	return 0;
//}
//
//int32_t StartTimer()
//{
//	MAP_TimerLoadSet(APP_PROFILING_TIMER_BASE, TIMER_A, 0xFFFF);
//	//
//	// Enable the GPT
//	//
//	MAP_TimerEnable(APP_PROFILING_TIMER_BASE, TIMER_A);
//
//	return 0;
//}
//
//int32_t StopTimer()
//{
//	MAP_TimerDisable(APP_PROFILING_TIMER_BASE, TIMER_A);
//
//	return 0;
//}
//
//int32_t GetTimeDuration(float* pfDurationMilli)
//{
//	uint16_t ulCounter;
//
//	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
//	ulCounter = 0xFFFFFFFF - ulCounter;
//	//*pfDurationMilli = /*(v_TimerOverflows * 4294967296.0 / 80000.0) +*/ ((float_t)ulCounter / 80000.0); //in milli sec
//	*pfDurationMilli = (v_TimerOverflows * 65536 / 80000.0) + ((float_t)ulCounter / 80000.0); //in milli sec
//
//	return 0;
//}
//
