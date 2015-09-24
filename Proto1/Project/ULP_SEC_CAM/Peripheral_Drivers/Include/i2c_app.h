////////////////////////////////////////////////////////////////////////////////
 /*	i2c_app.h
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

#ifndef __I2C_APP_H__
#define __I2C_APP_H__

#include "app.h"

#define I2C_APP_MODE 			I2C_MASTER_MODE_STD //	Used by higher level API
#define LENGTH_IS_ONE 			1	//	Used by both same-level and higher level
									//	API
#define LENGTH_IS_TWO 			2
#define LENGTH_IS_FOUR			4
#define LENGTH_IS_SIX			6

int32_t i2cReadRegisters(uint8_t ucDevI2CAddr,
							uint8_t ucRegAddr,
							uint8_t ucLen,
							uint8_t* pucRegReadValues);

int32_t i2cWriteRegisters(uint8_t ucDevI2CAddr,
							uint8_t ucRegAddr,
							uint8_t ucLen,
							uint8_t* pucRegWriteValues);

int32_t i2cReadRegistersTwoBytes(uint8_t ucDevI2CAddr,
									uint8_t* pucRegAddr,
									uint8_t ucLen,
									uint8_t* pucRegReadValues);

#endif
