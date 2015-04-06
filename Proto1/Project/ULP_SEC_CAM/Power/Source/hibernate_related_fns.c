
#include "common.h"
#include "app.h"
#include "prcm.h"

#include "lightSens_isl29035.h"

static void sensorsTriggerSetup();



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
    UART_PRINT("\n\rCleared sensors\n\r");

    DBG_PRINT("\n\rHIB: Entering HIBernate..."
        		"	\n\r Open up the door to capture the image\n\r");
    MAP_UtilsDelay(80000);

    //
    // Enter HIBernate mode
    //
    MAP_PRCMHibernateEnter();
}


static void sensorsTriggerSetup()
{
//	getLightsensor_intrptStatus();
	clearAccelMotionIntrpt();
}
