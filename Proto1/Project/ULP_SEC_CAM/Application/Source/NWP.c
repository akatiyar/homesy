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

int32_t NWP_SwitchOn()
{
	int32_t lRetVal;

	UART_PRINT("SL 1: %x\n", g_ulSimplelinkStatus);
	if (!(IS_NW_PROCSR_ON(g_ulSimplelinkStatus)))
	{
#ifdef WATCHDOG_ENABLE
		MAP_WatchdogIntClear(WDT_BASE);	//This reloads the relaod counter value to WDT
		g_ucFeedWatchdog = 0;
#endif
		lRetVal = sl_Start(0,0,0);
#ifdef WATCHDOG_ENABLE
		g_ucFeedWatchdog = 1;
#endif
		if(lRetVal >= 0)
		{
			SET_STATUS_BIT(g_ulSimplelinkStatus, STATUS_BIT_NWP_INIT);
		}
		UART_PRINT("SL 2: %x\n", g_ulSimplelinkStatus);
	}
	else
	{
		lRetVal = 0;
	}

	return lRetVal;
}

int32_t NWP_SwitchOff()
{
	int32_t lRetVal;

	UART_PRINT("SL 3: %x\n", g_ulSimplelinkStatus);
	if ((IS_NW_PROCSR_ON(g_ulSimplelinkStatus)))
	{
#ifdef WATCHDOG_ENABLE
		MAP_WatchdogIntClear(WDT_BASE);	//This reloads the relaod counter value to WDT
		g_ucFeedWatchdog = 0;
#endif
		lRetVal = sl_Stop(0xFFFF);
#ifdef WATCHDOG_ENABLE
		g_ucFeedWatchdog = 1;
#endif
		if(lRetVal >= 0)
		{
			CLR_STATUS_BIT_ALL(g_ulSimplelinkStatus);
		}
		UART_PRINT("SL 4: %x\n", g_ulSimplelinkStatus);
	}
	else
	{
		lRetVal = 0;
	}

	return lRetVal;
}
