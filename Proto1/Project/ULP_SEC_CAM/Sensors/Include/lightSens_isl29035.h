////////////////////////////////////////////////////////////////////////////////
 /*	fxos8700.h
 *	Company Name : Soliton Technologies
 *	Description  : Accelerometer + Magnetometer chip APIs
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////

#ifndef ISL29035_H_
#define ISL29035_H_

#include "i2c_app.h"
#include "math.h"
#include "app.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define ISL29035_I2C_ADDRESS 			0x44

// device register map
#define CMD_1_REG						0x00
#define CMD_2_REG						0x01
// data to read
#define DATA_LSB_REG					0x02
#define DATA_MSB_REG					0x03
//Threshold lower and upper
#define INT_LT_LSB_REG					0x04
#define INT_LT_MSB_REG					0x05
#define INT_HT_LSB_REG					0x06
#define INT_HT_MSB_REG					0x07

#define DEVICE_ID_REG 					0x0F
#define DEVICE_ID_MASK					0x38 //xx_ _ _xxx
//****************************************************
//Command register functions
//*****************************************************

//COMMAND 1
	// 1,2,4,8,16 kindoff filter waits for 16 successive samples above interrupt level
	#define INT_PERSIST						0x00

	//bit 2 of command 1.. read this register after interupt arise
	#define INT_STATUS_MSK					0x04


	//Operating mode bits 7 6 5
	//*******************************************
	//A0 - ambient light always check
	//00 - Power down
	//E0 - IR always check
	#define OP_MODE_ALS_ALWAYSON			0xA0
	#define OP_MODE_PWDN					0x00
	#define OP_MODE_IR_ALWAYSON				0xE0
	//********************************************
//COMMAND 2
	// full scale range 00-1, 01-4, 10-16, 11-64 K  1K seems to be good enough default
	#define	FS_RANGE_1K						0x00
//	#define	FS_RANGE_4K						0x01
//	#define	FS_RANGE_16K					0x10
//	#define	FS_RANGE_64K					0x11


	// ADC resolution 16,12,8,4   .. only 16 can give flicker removal
	#define ADC_RES_16						0x00
//	#define ADC_RES_12						0x01
//	#define ADC_RES_8						0x10
//	#define ADC_RES_4						0x11

//****************************************************************
//calculation for Lux

#ifdef 	FS_RANGE_1K
	#define LUX_RANGE 1000
#elif  	FS_RANGE_4K
	#define LUX_RANGE 4000
#elif	FS_RANGE_16K
	#define LUX_RANGE 16000
#elif 	FS_RANGE_64K
	#define LUX_RANGE 64000
#else
	#define LUX_RANGE 1000
#endif

#ifdef 	ADC_RES_16
	#define ADC_RANGE 65536
#elif  	ADC_RES_12
	#define ADC_RANGE 4096
#elif	ADC_RES_8
	#define ADC_RANGE 256
#elif 	ADC_RES_4
	#define ADC_RANGE 16
#else
	#define ADC_RANGE 65536
#endif

//******************************************************************************
// 								API PROTOTYPES
//******************************************************************************
uint16_t getLightsensor_data(void);
void configureISL29035(uint8_t ucAppMode);
uint16_t verifyISL29035(void);
uint16_t getLightsensor_intrptStatus(void);
void setThreshold_lightsensor(uint16_t upper_threshold,
								uint16_t lower_threshold);

#endif
