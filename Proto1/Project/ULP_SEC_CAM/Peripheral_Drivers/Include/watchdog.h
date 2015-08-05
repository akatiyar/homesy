/*
 * watchdog.h
 *
 *  Created on: 28-Jul-2015
 *      Author: Chrysolin
 */

#ifndef APPLICATION_INCLUDE_WATCHDOG_H_
#define APPLICATION_INCLUDE_WATCHDOG_H_

#include <stdint.h>

#define WDT_KICK_GAP				5000	//in milli sec. Can go up to ((2^32)/80M)
									//= 53 sec (approx).
									//Registered WDT interrupt handler will be
									//triggered every WDT_KICK_GAP

#define APPLICATION_TIMEOUT			300000	//in milli sec. 5min = 5*60*1000 msec = 300000
#define USERCONFIG_TIMEOUT			420000	//in milli sec. 7min = 7*60*1000 msec = 600000
//#define APPLICATION_TIMEOUT			30000	//in milli sec

int16_t WDT_init();
void WatchdogIntHandler(void);


#endif /* APPLICATION_INCLUDE_WATCHDOG_H_ */
