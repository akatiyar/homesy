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
#include "timer_fns.h"

#define FRIDGECAM_ID_SIZE			30		//in Bytes; 12 characters for MAC ID + extra

unsigned short intToASCII(long long cNum, char *cString);
void LED_Off();
void LED_On();
void LED_Blink_2(float_t fOnTime_inSecs, float_t fOffTime_inSecs, int8_t ucHowManyTimes);
int32_t Get_FridgeCamID(uint8_t* pucFridgeCamID);
#define IS_PUSHBUTTON_PRESSED (!GPIOPinRead(GPIOA1_BASE, GPIO_PIN_0))

#endif /* APP_COMMON_H_ */
