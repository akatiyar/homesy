
#ifndef HIBERNATE_RELATED_FNS_H_
#define HIBERNATE_RELATED_FNS_H_

//Includes needed to call fns declared in this file
#include "prcm.h"

//Defines used as parameter at function call
#define ENABLE_TIMER_WAKESOURCE			PRCM_HIB_SLOW_CLK_CTR
#define DISABLE_TIMER_WAKESOURCE		0
#define ENABLE_GPIO_WAKESOURCE			PRCM_HIB_GPIO2
#define DISABLE_GPIO_WAKESOURCE			0

#define HIGH_LEVEL						PRCM_HIB_HIGH_LEVEL
#define LOW_LEVEL						PRCM_HIB_LOW_LEVEL
#define FALL_EDGE						PRCM_HIB_FALL_EDGE
#define RISE_EDGE						PRCM_HIB_RISE_EDGE

#define WAKEON_LIGHT_ON					1
#define WAKEON_LIGHT_OFF				2

void HIBernate(uint32_t ucWakeSources,
				uint32_t ucGPIOInterruptType,
				uint8_t ucGPIOWakeCondition,
				float_t fHibIntervalInMinutes);
void sensorsTriggerSetup();


#endif /* HIBERNATE_RELATED_FNS_H_ */
