/*
 * reset.c
 *
 *  Created on: 08-Aug-2015
 *      Author: Chrysolin
 */

#include "app.h"

void Reset_byStarvingWDT()
{
#ifdef WATCHDOG_ENABLE
	g_ucFeedWatchdog = 0;
	while(1)
	{
		osi_Sleep(100);
		DEBG_PRINT("@");
	}
#endif
}
