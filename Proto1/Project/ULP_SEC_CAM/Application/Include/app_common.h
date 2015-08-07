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

#define FRIDGECAM_ID_SIZE			30		//in Bytes; 12 characters for MAC ID + extra

unsigned short intToASCII(long long cNum, char *cString);
void LED_Blink(uint8_t ucHowManyTimes, float_t fSecsForEachCycle);
void LED_Off();
void LED_On();

void LED_Blink_2(float_t fOnTime_inSecs, float_t fOffTime_inSecs, int8_t ucHowManyTimes);
int32_t Get_FridgeCamID(uint8_t* pucFridgeCamID);
#endif /* APP_COMMON_H_ */
