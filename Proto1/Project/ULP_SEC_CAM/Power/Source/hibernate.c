#include "stdbool.h"

#include "common.h"

#include "app.h"
#include "app_common.h"
#include <accelomtr_magntomtr_fxos8700.h>
#include <hibernate.h>
#include <light_sens_isl29035.h>

#define SEC_PER_MINUTE					60
#define RTCCLKS_PER_SEC					32768

extern int16_t IsLightOff(uint16_t usThresholdLux);

//******************************************************************************
//	This function puts CC3200 in hibernate
//
//	param[in]	ucWakeSources - choice of source for wake up from hibernate. Can
//						be one of the following macros or the or of the two
//							ENABLE_TIMER_WAKESOURCE
//							ENABLE_GPIO_WAKESOURCE
//	param[in]	ucGPIOInterruptType-valid for GPIO wake up only
//						Type of GPIO Signal taken as wakeup signal.
//						Can take one of the following Macros as its value
//							PRCM_HIB_LOW_LEVEL
//							PRCM_HIB_HIGH_LEVEL
//							PRCM_HIB_FALL_EDGE
//							PRCM_HIB_RISE_EDGE
//	param[in]	ucGPIOWakeCondition	-Can use WAKEON_LIGHT_ON or WAKEON_LIGHT_OFF
//									-not used presently
//	param[in]	fHibIntervalInMinutes-Hibernation interval before the Slow Clock
//						counter wakes the device up fom Hibernate
//
//	Description:
//		1. Disable the wakeup sources (CC3200 timer, CC3200 GPIO)
//		2. Enable the wakeup source(s) (CC3200 timer and/or CC3200 GPIO)
//		3. Sets Hibernate interval incase time based interrupt is enabled
//		4. Sets up type of wake trigger, if GPIO WakeSource is enabled
//		5. Prepares peripheral devices to give interrupt
//		6. Goes into hibernate
//
//	NOTE: The device
//******************************************************************************
void HIBernate(uint32_t ulWakeSources,
				uint32_t ulGPIOInterruptType,
				uint8_t ucGPIOWakeCondition,
				float_t fHibIntervalInMinutes)
{
	RELEASE_PRINT("HIBernating\n");

	//
    // Enable Wake Source (CC3200 timer and/or CC3200 GPIO)
    //
	MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_GPIO2|PRCM_HIB_SLOW_CLK_CTR);
	MAP_PRCMHibernateWakeupSourceEnable(ulWakeSources);

	//
	// Set-up Wake Source (CC3200 timer and/or CC3200 GPIO)
	//
	if(ulWakeSources&PRCM_HIB_SLOW_CLK_CTR)
	{
		MAP_PRCMHibernateIntervalSet(fHibIntervalInMinutes*((float_t)SEC_PER_MINUTE)*((float_t)RTCCLKS_PER_SEC));
		MAP_UtilsDelay(80000);
	}
	if(ulWakeSources&PRCM_HIB_GPIO2)
	{
		MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, ulGPIOInterruptType);
		MAP_UtilsDelay(80000);

		//
		//	Configure wake-up trigger sensors
		//
		//If light is off now, trigger should be given on light on
		if(IsLightOff(LUX_THRESHOLD))
		{
			configureISL29035(0, LUX_THRESHOLD, LIGHTON_TRIGGER);
			RELEASE_PRINT("***OPEN DOOR TO CAPTURE IMAGE***\n");
			UtilsDelay(800000);	//Delay for UART peripheral to finish printing
			sensorsTriggerSetup();
		}
		else
		//If light is on now, trigger should be given on light off
		{
			configureISL29035(0, LUX_THRESHOLD, LIGHTOFF_TRIGGER);
			RELEASE_PRINT("***CLOSE DOOR TO WAKE DEVICE***\n");
			UtilsDelay(800000); //Delay for UART peripheral to finish printing
			sensorsTriggerSetup();
		}
#ifdef IOEXPANDER_PRESENT
		ClearInterrupt_IOExpander();
#endif
	}

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}

//******************************************************************************
//	Simply clears the interrupt
//******************************************************************************
void sensorsTriggerSetup()
{
	//
	// If Light sensor is to be used
	//
	getLightsensor_intrptStatus(); //To clear the interrupt

#if 0
	//
	// If Accelerometer trigger is to be used
	//
	//	verifyAccelMagnSensor();
	//	clearAccelMotionIntrpt();
	//	configureFXOS8700(MODE_ACCEL_INTERRUPT);
	//	DEBG_PRINT("Configured Accelerometer for wake up\n\r");
#endif
}

#if 0
{
    //
    // Setup Wake Source
    //
	if(ucEnableSlowClkCntrWake)
	{
		//MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2|PRCM_HIB_SLOW_CLK_CTR);
		MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
		MAP_PRCMHibernateIntervalSet(fHibIntervalInMinutes*SEC_PER_MINUTE*RTCCLKS_PER_SEC);
	}
	else
	{
		MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_SLOW_CLK_CTR);
				//Otherwise old setting of RTC timer Wake trigger will be
				//retained
		//MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
	}

	if(ucEnableGPIOWake)
	{
		MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
		MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, ucGPIOInterruptType);

		//
		//	Configure or clear interrupts of wake-up trigger sensors
		//
		sensorsTriggerSetup();
		DEBG_PRINT("Cleared sensors\n\r");
	}
	else
	{
		MAP_PRCMHibernateWakeupSourceDisable(PRCM_HIB_GPIO2);
	}

    DBG_PRINT("HIB: Entering HIBernate...\n\r"
        		"***OPEN DOOR TO CAPTURE IMAGE***\n\r");
    MAP_UtilsDelay(80000);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}
#endif
