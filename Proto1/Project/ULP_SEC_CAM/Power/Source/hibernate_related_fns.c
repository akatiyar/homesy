
#include "common.h"
#include "app.h"
#include "prcm.h"

#include "lightSens_isl29035.h"
#include "accelomtrMagntomtr_fxos8700.h"

#include "hibernate_related_fns.h"

void HIBernate()
{
    //
    // Setup Wake Source
    //
    MAP_PRCMHibernateWakeupSourceEnable(PRCM_HIB_GPIO2);
    MAP_PRCMHibernateWakeUpGPIOSelect(PRCM_HIB_GPIO2, PRCM_HIB_FALL_EDGE);

    //
    //	Configure or clear interrupts of wake-up trigger sensors
    //
    sensorsTriggerSetup();
    UART_PRINT("Cleared sensors\n\r");

    DBG_PRINT("HIB: Entering HIBernate...\n\r"
        		"***OPEN DOOR TO CAPTURE IMAGE***\n\r");
    MAP_UtilsDelay(80000);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}


void sensorsTriggerSetup()
{
	//		UART_PRINT("\n\rLIGHT SENSOR:\n\r");
	//		verifyISL29035();
	//		configureISL29035(0);
	//		getLightsensor_intrptStatus(); //To clear the interrupt
	//		UART_PRINT("Configured Light Sensor for wake up\n\r");

	verifyAccelMagnSensor();
	clearAccelMotionIntrpt();
	configureFXOS8700(MODE_ACCEL_INTERRUPT);
	UART_PRINT("Configured Accelerometer for wake up\n\r");
}
