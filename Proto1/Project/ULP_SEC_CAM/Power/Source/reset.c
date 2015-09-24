/*
 * reset.c
 *
 *  Created on: 08-Aug-2015
 *      Author: Chrysolin
 */

#include "app_common.h"

#include "wdt.h"
#include "rom_map.h"

//Warning: use this only within OS task
//Reset happens in 7 to 12 seconds
void Reset_byStarvingWDT()
{
#ifdef WATCHDOG_ENABLE
	LED_Blink_2(.2,.2,5);
	osi_Sleep(.2*2*5*1000);
	LED_Off();
	//MAP_WatchdogIntClear(WDT_BASE);	//This reloads the relaod counter value to WDT
	g_ucFeedWatchdog = 0;
	while(1)
	{
		osi_Sleep(100);
		DEBG_PRINT("@");
	}
#endif
}
