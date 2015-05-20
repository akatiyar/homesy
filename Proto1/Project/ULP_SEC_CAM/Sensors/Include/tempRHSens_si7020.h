////////////////////////////////////////////////////////////////////////////////
 /*	tempRHSens_si7020.h
 *	Company Name : Soliton Technologies
 *	Description  : Temp + RH sensor chip APIs
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////

#ifndef SI7020_H_
#define SI7020_H_

#include "i2c_app.h"
#include "math.h"
#include "app.h"

//******************************************************************************
//                      		MACRO DEFINITIONS
//******************************************************************************
#define SI7020_I2C_ADDRESS		0x40

//
//	Si7020 Commands
//
#define RESET_CMD				0xFE
#define MEASURE_RH_CMD			0xE5
#define MEASURE_TEMP_CMD		0xE3
#define READ_TEMP_CMD			0xE0
#define READ_CONFIGREG_CMD		0xE7
#define WRITE_CONFIGREG_CMD		0xE6
#define FETCH_ID_CMD_BYTE1		0xFC
#define FETCH_ID_CMD_BYTE2		0xC9

#define RESOLUTION_MASK			0b10000001
#define HEATER_ENABLE_MASK		0b00000100
#define HEATER_ENABLE			HEATER_ENABLE_MASK

//
//	Resolution Options macros
//
#define RH12BIT_TEMP14BIT		0b00000000	// Default. Choose this for highest
											// resolution
#define RH8BIT_TEMP12BIT		0b00000001	// Choose this for lowest conversion
											// time
#define RH10BIT_TEMP13BIT		0b10000000
#define RH11BIT_TEMP11BIT		0b10000001

//******************************************************************************
//
//	Calling this function does a software reset of SI7020
//
//	\param none
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define softResetTempRHSensor() i2cWriteRegisters(SI7020_I2C_ADDRESS,\
													RESET_CMD,\
													0, \
													NULL)

//******************************************************************************
// 								API PROTOTYPES
//******************************************************************************
void verifyTempRHSensor();
void configureTempRHSensor();
void getTempRH(float_t* pfTemp, float_t* pfRH);


#endif /* SI7020_H_ */
