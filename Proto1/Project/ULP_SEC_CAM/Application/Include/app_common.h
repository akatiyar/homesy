/*
 * app_common.h
 *
 *  Created on: 09-Apr-2015
 *      Author: Chrysolin
 */

#ifndef APP_COMMON_H_
#define APP_COMMON_H_

#include "app.h"
#include "math.h"
#include "LED_timer.h"

unsigned short intToASCII(long long cNum, char *cString);
void LED_Blink(uint8_t ucHowManyTimes, float_t fSecsForEachCycle);
void LED_Off();
void LED_On();

void LED_Blink_2(float_t fOnTime_inSecs, float_t fOffTime_inSecs, int8_t ucHowManyTimes);

#endif /* APP_COMMON_H_ */
