/*
 * NWP.c
 *
 *  Created on: 28-Jul-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "simplelink.h"
#include "wdt.h"
#include "rom_map.h"

//******************************************************************************
//	This function starts NWP, if NWP is not already started
//******************************************************************************
int32_t NWP_SwitchOn()
{
	int32_t lRetVal;

	DEBG_PRINT("SL 1: %x\n", g_ulSimplelinkStatus);

	// Do only if NWP is in off state/ stopped state now
	if (!(IS_NW_PROCSR_ON(g_ulSimplelinkStatus)))
	{
		// Clear watchdog and stop feeding it. If sl_start() does not exit,
		//device is reset by WDT
#ifdef WATCHDOG_ENABLE
		MAP_WatchdogIntClear(WDT_BASE);	//This reloads the relaod counter value to WDT
		g_ucFeedWatchdog = 0;
#endif
		//Start network processor
		lRetVal = sl_Start(0,0,0);

		// Stop starving watchdog
#ifdef WATCHDOG_ENABLE
		g_ucFeedWatchdog = 1;
#endif

		//Indicate in the Simple link status global variable that NWP is on
		if(lRetVal >= 0)
		{
			SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_NWP_INIT);
		}
		DEBG_PRINT("SL 2: %x\n", g_ulSimplelinkStatus);
	}
	else
	{
		lRetVal = 0;
	}

	return lRetVal;
}

//******************************************************************************
//	This function stops NWP, if NWP is on
//******************************************************************************
int32_t NWP_SwitchOff()
{
	int32_t lRetVal;

	DEBG_PRINT("SL 3: %x\n", g_ulSimplelinkStatus);

	// Do only if NWP is in on state now
	if ((IS_NW_PROCSR_ON(g_ulSimplelinkStatus)))
	{
		// Clear watchdog and stop feeding it. If sl_stop() does not exit,
		//device is reset by WDT
#ifdef WATCHDOG_ENABLE
		MAP_WatchdogIntClear(WDT_BASE);	//This reloads the relaod counter value to WDT
		g_ucFeedWatchdog = 0;
#endif

		//Stop network processor
		lRetVal = sl_Stop(0xFFFF);

		// Stop starving watchdog
#ifdef WATCHDOG_ENABLE
		g_ucFeedWatchdog = 1;
#endif
		//Indicate in the Simple link status global variable that NWP is off
		if(lRetVal >= 0)
		{
			CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		}
		DEBG_PRINT("SL 4: %x\n", g_ulSimplelinkStatus);
	}
	else
	{
		lRetVal = 0;
	}

	return lRetVal;
}
