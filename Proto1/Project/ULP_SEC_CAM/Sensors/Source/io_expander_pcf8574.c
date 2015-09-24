/*
 * IOExpander_pcf8574.c
 *
 *  Created on: 30-Jul-2015
 *      Author: Chrysolin
 */
#include <io_expander_pcf8574.h>
#include "app.h"
#include "i2c.h"
#include "i2c_if.h"
#include "i2c_app.h"

//******************************************************************************
//	Reads the port data of PCF8574
//******************************************************************************
int32_t ReadPorts_pcf8574(uint8_t* pucData)
{
	int32_t lRetVal;

	if(MAP_I2CMasterErr(I2CA0_BASE) != I2C_MASTER_ERR_NONE)
	{
		DBG_PRINT("I2C Master Error: %x Write()",MAP_I2CMasterErr(I2CA0_BASE));
		//return FAILURE;
		//LOOP_FOREVER();
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_PRCMPeripheralClkDisable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_UtilsDelay(1);
	    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,false);	//100k only. Change if 400k is also to be used
	}

	//Only one data register and no cofig registers. Therefore, send only device
	//I2C address and read the port data.
	lRetVal = I2C_IF_Read(PCF8574_I2C_ADDRESS, pucData, LENGTH_IS_ONE);
	PRINT_ON_ERROR(lRetVal);

	return lRetVal;
}
