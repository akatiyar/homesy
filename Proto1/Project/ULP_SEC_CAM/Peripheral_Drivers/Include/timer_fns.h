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

volatile uint32_t v_TimerOverflows;

int32_t InitializeTimer();
int32_t StartTimer();
int32_t StopTimer();
int32_t GetTimeDuration();

#endif /* TIMER_FNS_H_ */
