/*
 * watchdog_timer.c
 *
 *  Created on: 28-Jul-2015
 *      Author: Chrysolin
 */
#include "app.h"
#include "wdt_if.h"
#include "wdt.h"
#include "watchdog.h"
#include "osi.h"

volatile uint32_t g_ulWatchdogCycles;
//*****************************************************************************
//
//! The interrupt handler for the watchdog timer
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void WatchdogIntHandler(void)
{
	//UART_PRINT("wdt\n");
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(!g_ucFeedWatchdog)
    {
//    	UART_PRINT("WDT no feed\n");
//    	MAP_UtilsDelay(100*80000/6);
//    	//MAP_UtilsDelay(100*80000/6);
//    	PRCMSOCReset();
    	//osi_Sleep(100);
    	return;
    }

    //
    // After 10 interrupts, switch On LED6 to indicate system reset
    // and don't clear watchdog interrupt which causes system reset
    //
    if(g_ulWatchdogCycles >= (g_ulAppTimeout_ms/WDT_KICK_GAP))
    {
        //MAP_UtilsDelay(800000);
    	UART_PRINT("WDT app time out\n");
    	MAP_UtilsDelay(100*80000/6);
        return;
    }

    //
    // Clear the watchdog interrupt.
    //
    MAP_WatchdogIntClear(WDT_BASE);
//    GPIO_IF_LedOn(MCU_RED_LED_GPIO);
//    MAP_UtilsDelay(800000);
//    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    //
    // Increment our interrupt counter.
    //
    g_ulWatchdogCycles++;
    //UART_PRINT("W*D*T\n");

}

int16_t WDT_init()
{
#ifdef WATCHDOG_ENABLE
	UART_PRINT("WDT Init\n");
	WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(WDT_KICK_GAP));

	g_ucFeedWatchdog = 1;
	g_ulWatchdogCycles = 0;

	g_ulAppTimeout_ms = APPLICATION_TIMEOUT;
#endif
	return 0;
}
