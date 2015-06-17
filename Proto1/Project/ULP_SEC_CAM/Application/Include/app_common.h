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

unsigned short intToASCII(short cNum, char *cString);
void LED_Blink(uint8_t ucHowManyTimes, float_t fSecsForEachCycle);
void LED_Off();
void LED_On();

#endif /* APP_COMMON_H_ */
