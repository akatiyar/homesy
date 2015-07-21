/*
 * timer_fns.h
 *
 *  Created on: 21-Apr-2015
 *      Author: Chrysolin
 */

#ifndef TIMER_FNS_H_
#define TIMER_FNS_H_

#include "app.h"
#include "timer_if.h"

volatile uint32_t Elapsed_100MilliSecs;
volatile uint32_t Elapsed_1Secs;
volatile uint8_t checkForLight_Flag;

int32_t start_100mSecTimer();
int32_t stop_100mSecTimer();
uint32_t get_timeDuration();

int32_t start_1Sec_TimeoutTimer();
int32_t stop_1Sec_TimeoutTimer();

//volatile uint32_t v_TimerOverflows;
//volatile uint32_t v_OneSecFlag;
//
//int32_t InitializeTimer();
//int32_t StartTimer();
//int32_t StopTimer();
//int32_t GetTimeDuration();

#endif /* TIMER_FNS_H_ */
