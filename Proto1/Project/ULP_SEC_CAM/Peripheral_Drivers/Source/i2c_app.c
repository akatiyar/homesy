////////////////////////////////////////////////////////////////////////////////
 /*	i2c_app.c
 *	Company Name : Soliton Technologies
 *	Description  : I2C Read and Write interface APIs for the application
 *	Date		 :
 *	Version		 :
 *
 *
 *
 *
 */
 ///////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>
#include "i2c_if.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "i2c.h"
#include "rom_map.h"
#include "prcm.h"
#include "i2c_app.h"
//******************************************************************************
//                      		MACRO DEFINITIONS
//******************************************************************************
#define ONE_STOP_BIT 			1
#define ZERO_STOP_BIT 			0

#define SYS_CLK                 80000000
//******************************************************************************
//
//  This function reads values of one or more registers in FXOS8700, ISL29035
//	or Si7020 through I2C interface
//
//	\param[in]	ucDevI2CAddr - address of the slave I2C device - the 7 bits
//								should be right aligned
//	\param[in]	ucRegAddr - 8-bit address of the register (first register,
//								in case of burst read) to be read
//	\param[in]	ucLen - number of registers to be read starting from ucRegAddr
//	\param[out]	pucRegReadValues - pointer to array to which values read will be
//								stored
//
//	\return FAILURE if function fails
//			SUCCESS if function finishes execution successfully
//
//******************************************************************************
int32_t i2cReadRegisters(uint8_t ucDevI2CAddr,
								uint8_t ucRegAddr,
								uint8_t ucLen,
								uint8_t* pucRegReadValues)
{
	int32_t lRetVal;

	if(MAP_I2CMasterErr(I2CA0_BASE) != I2C_MASTER_ERR_NONE)
	{
		DEBG_PRINT("I2C Master Error: %x Read()",MAP_I2CMasterErr(I2CA0_BASE));
		//return FAILURE;
		//LOOP_FOREVER();
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_PRCMPeripheralClkDisable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_UtilsDelay(1);
	    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,false);	//100k only. Change if 400k is also to be used
	}

	lRetVal = I2C_IF_Write(ucDevI2CAddr,
							&ucRegAddr,
							LENGTH_IS_ONE,
							ZERO_STOP_BIT);
	PRINT_ON_ERROR(lRetVal);

	if(lRetVal == SUCCESS)
	{
		lRetVal = I2C_IF_Read(ucDevI2CAddr,
								pucRegReadValues,
								ucLen);
	}
	PRINT_ON_ERROR(lRetVal);
	MAP_UtilsDelay((5*80)/6);	//5uSec

	return lRetVal;
}


//******************************************************************************
//
//  This function writes values to one or more registers in FXOS8700, ISL29035
//	or Si7020 through I2C interface
//
//	\param[in]	ucDevI2CAddr - address of the slave I2C device - the 7 bits
//								should be right aligned
//	\param[in]	ucRegAddr - 8-bit address of the register (first register,
//								in case of burst write) to be written
//	\param[in]	ucLen - number of registers to be written starting from
//								ucRegAddr
//	\param[in]	pucRegWriteValues - pointer to array to which holds the data to
//								be written
//
//	\return FAILURE if function fails
//			SUCCESS if function finishes execution successfully
//
//******************************************************************************
int32_t i2cWriteRegisters(uint8_t ucDevI2CAddr,
										uint8_t ucRegAddr,
										uint8_t ucLen,
										uint8_t* pucRegWriteValues)
{
	int32_t lRetVal;
	uint8_t ucWriteArray[ucLen+1];
	uint8_t i;

	if(MAP_I2CMasterErr(I2CA0_BASE) != I2C_MASTER_ERR_NONE)
	{
		DEBG_PRINT("I2C Master Error: %x Write()",MAP_I2CMasterErr(I2CA0_BASE));
		//return FAILURE;
		//LOOP_FOREVER();
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_PRCMPeripheralClkDisable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_UtilsDelay(1);
	    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,false);	//100k only. Change if 400k is also to be used
	}

	//
	//Arrange the register address and write data in a single array
	//
	ucWriteArray[0] = ucRegAddr;
	for(i = 1; i <= ucLen; i++)
	{
		ucWriteArray[i] = pucRegWriteValues[i-1];
	}

	lRetVal = I2C_IF_Write(ucDevI2CAddr,
							ucWriteArray,
							ucLen+1,
							ONE_STOP_BIT);

	PRINT_ON_ERROR(lRetVal);

	MAP_UtilsDelay((5*80)/6);	//5uSec
	return lRetVal;
}


//******************************************************************************
//
//  This function read values from Si7020 through I2C interface where the
//	register address/command is 2 bytes
//
//	\param[in]	ucDevI2CAddr - address of the slave I2C device - the 7 bits
//								should be right aligned
//	\param[in]	pucRegAddr - pointer to array containing 2 byte register address
//								command from where data has to be read
//	\param[in]	ucLen - number of registers to be read starting
//	\param[in]	pucRegWriteValues - pointer to array to which holds the data to
//								be written
//
//	\return FAILURE if function fails
//			SUCCESS if function finishes execution successfully
//
//******************************************************************************
int32_t i2cReadRegistersTwoBytes(uint8_t ucDevI2CAddr,
								uint8_t* pucRegAddr,
								uint8_t ucLen,
								uint8_t* pucRegReadValues)
{
	int32_t lRetVal;

	if(MAP_I2CMasterErr(I2CA0_BASE) != I2C_MASTER_ERR_NONE)
	{
		DEBG_PRINT("I2C Master Error: %x ReadTwo()",MAP_I2CMasterErr(I2CA0_BASE));
		//return FAILURE;
		//LOOP_FOREVER();
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_PRCMPeripheralClkDisable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_UtilsDelay(1);
	    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	    MAP_PRCMPeripheralReset(PRCM_I2CA0);
	    MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,false);	//100k only. Change if 400k is also to be used
	}

	lRetVal = I2C_IF_Write(ucDevI2CAddr,
							pucRegAddr,
							LENGTH_IS_TWO,
							//LENGTH_IS_ONE,
							ZERO_STOP_BIT);
	PRINT_ON_ERROR(lRetVal);

	if(lRetVal == SUCCESS)
	{
		lRetVal = I2C_IF_Read(ucDevI2CAddr,
								pucRegReadValues,
								ucLen);
	}
	PRINT_ON_ERROR(lRetVal);
	MAP_UtilsDelay((5*80)/6);	//5uSec

	return lRetVal;
}
