/*
 * LED_Timer.h
 *
 *  Created on: 31-Jul-2015
 *      Author: Chrysolin
 */

#ifndef PERIPHERAL_DRIVERS_LED_TIMER_H_
#define PERIPHERAL_DRIVERS_LED_TIMER_H_

#define BLINK_FOREVER		-1

uint8_t g_OnTime;
uint8_t g_OffTime;

int8_t g_NoOfBlinks;

int32_t LEDTimer_Enable();
int32_t LEDTimer_Disable();
int32_t LEDTimer_Start();
int32_t LEDTimer_Stop();

#endif /* PERIPHERAL_DRIVERS_LED_TIMER_H_ */
