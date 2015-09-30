/*
 * watchdog_timer.c
 *
 *  Created on: 28-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"

#include "wdt_if.h"
#include "wdt.h"
#include "osi.h"

#include "watchdog.h"

//*****************************************************************************
//! The interrupt handler for the watchdog timer
//!		If g_ucFeedWatchdog is not cleared,  it clears the interrupt, reloading
//!		the reload value
//!		If application timeout happened, interrupt isnot cleared
//!		Keeps track of application time
//!
//! \param  None
//! \return None
//*****************************************************************************
void WatchdogIntHandler(void)
{
	//DEBG_PRINT("wdt\n");
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(!g_ucFeedWatchdog)
    {
		//DEBG_PRINT("WDT no feed\n");
		//PRCMSOCReset();
    	return;
    }

    //
    // After 10 interrupts, switch On LED6 to indicate system reset
    // and don't clear watchdog interrupt which causes system reset
    //
    if(g_ulWatchdogCycles >= (g_ulAppTimeout_ms/WDT_KICK_GAP))
    {
    	DEBG_PRINT("WDT app time out\n");
    	MAP_UtilsDelay(100*80000/6);
        return;
    }

    /*// If apptimeout/WDT_notfed, take corrective action
    // WDT reset due to fault ISR will not be taken
    if((!g_ucFeedWatchdog) || (g_ulWatchdogCycles >= (g_ulAppTimeout_ms/WDT_KICK_GAP)))
    {
		if(g_ulWatchdogCycles >= (g_ulAppTimeout_ms/WDT_KICK_GAP))
		{
			//MAP_UtilsDelay(800000);
			DEBG_PRINT("WDT app time out\n");
			//MAP_UtilsDelay(100*80000/6);

			g_ulWatchdogCycles = 0;
		}
		else if(!g_ucFeedWatchdog)
		{
			//g_ucFeedWatchdog = 1;	//Continue to feed the WDT
		}

    	if(!correctiveActionDone)
    	{
    		g_ucFeedWatchdog = 1;	//Continue to feed the WDT
    		CorrectiveAction();
    		g_ucFeedWatchdog = 0;	//Starve the WDT
    	}
    	else
    	{
    		return;
    	}
    }*/

    //
    // Clear the watchdog interrupt.
    //
    MAP_WatchdogIntClear(WDT_BASE);

    //
    // Increment our interrupt counter.
    //
    g_ulWatchdogCycles++;
}

//*****************************************************************************
//	Initialize the watch dog timer
//
//	param	none
//	return	0
//
//	Enables the watchdog timer if in Release mode or if WATCHDOG_ENABLE is
//	defined
//	Disables watchdog if not in release mode and WATCHDOG_ENABLE is not defined
//*****************************************************************************
int16_t WDT_init()
{
	//DEBG_PRINT("WDT Init\n");
	WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(WDT_KICK_GAP));

	//	Stalling while using debugger
	MAP_WatchdogStallEnable(WDT_BASE);	//CS

//Release will always have watchdog enabled. In debug, watchdog can be enabled
//by defining WATCKDOG_ENABLE macro
#ifdef USB_DEBUG
#ifndef WATCHDOG_ENABLE
    	DEBG_PRINT("WDT deinit\n");
    	MAP_PRCMPeripheralClkDisable(PRCM_WDT, PRCM_RUN_MODE_CLK);
#endif
#endif

	g_ucFeedWatchdog = 1;
	g_ulWatchdogCycles = 0;

	g_ulAppTimeout_ms = APPLICATION_TIMEOUT;

	return 0;
}
