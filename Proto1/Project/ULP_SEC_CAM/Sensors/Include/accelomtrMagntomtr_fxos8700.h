////////////////////////////////////////////////////////////////////////////////
 /*	accelomtrMagntomtr_fxos8700.h
 *	Company Name : Soliton Technologies
 *	Description  : Accelerometer + Magnetometer chip APIs
 *	Author		 : CS
 *	Date		 :
 *	Version		 : 1.0
 *
 */
 ///////////////////////////////////////////////////////////////////////////////

#ifndef FXOS8700_H_
#define FXOS8700_H_

#include "i2c_app.h"
#include "math.h"
#include "app.h"


//#define MOVING_AVG_FLTR_L				10
#define MOVING_AVG_FLTR_L				4
#define L_AVG_INIT_MAGN					50
//#define NO_OF_PAST_VALS					100 // Set this such that:
//											// angle(n) = 40
#define DOOR_ANGLE_TO_DETECT			40	// In degrees
#define MAGNMTR_NOISEFLOOR				5	// In degrees
											// Experimentally obtained value
#define DOOR_ANGLE_HIGHER				(DOOR_ANGLE_TO_DETECT+MAGNMTR_NOISEFLOOR)
											//In Degrees
//******************************************************************************
//                      MACRO DEFINITIONS
//******************************************************************************
#define FXOS8700_I2C_ADDRESS 			0x1E

#define FAST_READ 						0
#define LENGTH_OUTPUT_DATA 				(FAST_READ?3:6)
#define FXOS8700_LENGTH_OUTPUT_DATA 	LENGTH_OUTPUT_DATA

#define ACCEL_OUTPUT_DATA_REG 			0x01
#define MAGNT_OUTPUT_DATA_REG 			0x33

#define STATUS_REG 						0x00
#define ACCEL_THRSLD_REG				0X17
#define DEBOUNCE_THRSLD_REG				0X18
#define CTRL_REG1 						0x2A
#define CTRL_REG2 						0x2B
#define CTRL_REG3 						0x2C
#define CTRL_REG4 						0x2D
#define CTRL_REG5 						0x2E
#define M_CTRL_REG1 					0x5B
#define M_CTRL_REG2 					0x5C
#define M_CTRL_REG3						0x5D
#define XYZ_DATA_CFG_REG		 		0x0E
#define MOFF_X_MSB_REG					0x3F
#define MOFF_X_LSB_REG					0x40
#define MOFF_Y_MSB_REG					0x41
#define MOFF_Y_LSB_REG					0x42
#define MOFF_Z_MSB_REG					0x43
#define MOFF_Z_LSB_REG					0x44

//Accel Motion Detection Registers' Addresses
#define A_FFMT_CFG_REG					0x15
#define A_FFMT_THRS_REG					0x17
#define A_FFMT_DEBOUNCE_CNT_REG			0x18
#define A_FFMT_SRC_REG					0x16

#define FS								2 		// Valid values:2,4,8
#define SENSITIVITY_1FS_FACTOR			(0.000122)
#define SENSITIVITY_ACCEL				SENSITIVITY_1FS_FACTOR * FS

#define ACCEL_THRESHOLD_STEP_VAL		(0.063)
#define MASK_BIT0TO6					(0b01111111)

#define ACCEL_THRESHOLD_UPPER_LIMIT		(FS)
#define ACCEL_THRESHOLD_LOWER_LIMIT		(-FS)

#define NO_OF_AXIS						3
#define X_AXIS							0
#define Y_AXIS							1
#define Z_AXIS							2

//	These macros should be fixed according on orientation of the chip and PCB
#define PARALLEL_AXIS					X_AXIS	//Axis that is Horizontal, and
												//parallel to the door
#define PERPENDICULAR_AXIS				Z_AXIS	//Axis that is Horizontal, and
												//perpendicular to the door

//#define PI								(3.141592654F)

#define SENSITIVITY_MAG					.1		//uT per LSB

#define MODE_READ_ACCEL_MAGN_DATA		1
#define MODE_ACCEL_INTERRUPT			2

//*****************************************************************************
// 									STRUCTURES
//*****************************************************************************
struct reg_data_pair
{
	uint8_t ucRegAddr;
	uint8_t ucConfigVal;
	uint8_t ucMask;
};

//******************************************************************************
//
//  This function fetches current accelerometer readings of all the 3 axis from
//	FXOS8700 by calling i2cReadRegister API
//
//	\param[out] pucAccelData - pointer to array to which accelerometer data is
//								stored
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define getAccelData(pucAccelData) i2cReadRegisters(FXOS8700_I2C_ADDRESS,\
									ACCEL_OUTPUT_DATA_REG,\
									LENGTH_OUTPUT_DATA,\
									pucAccelData)

//******************************************************************************
//
//  This function writes the value of a single register, particularly for a
//	status register, in FXOS8700
//
//	\param[in]	ucStatusRegAddr - Address of a configuration register, held in a
//									macro
//	\param[out]	pucStatusVal - Status value as read from the register address
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define readStatusReg(ucStatusRegAddr, pucStatusVal)\
								i2cReadRegisters(FXOS8700_I2C_ADDRESS,\
									ucStatusRegAddr,\
									LENGTH_IS_ONE,\
									pucStatusVal)

//******************************************************************************
//
//  This function writes a value to a single register, particularly for
//	configuring the device, in FXOS8700
//
//	\param[in]	ucConfigRegAddr - Address of a configuration register, held in a
//									macro
//	\param[in]	ucConfigVal -	Configuration Value to be written to the
//									register, can be a macro or variable
//
//	\return 0: Success or <0: Failure
//
//******************************************************************************
#define setConfigReg(ucConfigRegAddr, ucConfigVal)\
								i2cWriteRegisters(FXOS8700_I2C_ADDRESS,\
									ucConfigRegAddr,\
									LENGTH_IS_ONE,\
									&ucConfigVal)



//*****************************************************************************
// 								API PROTOTYPES
//*****************************************************************************
int32_t verifyAccelMagnSensor();
void configureFXOS8700(uint8_t ucAppMode);
void getDoorDirection(float_t* pfDegrees);
void getAccelerationMagnitude(float_t* pfAccel);
void getMagnFlux_3axis(float_t* pfMagnFlux_3axis);
int32_t getMagntData(uint8_t* pucMagntData);
int32_t getAccelMagntData(uint8_t* pucAccelMagntData);

void setMotionDetectionThreshold(float accelThreshold);
void updateConfigReg(uint8_t ucConfigRegAddr,
						uint8_t ucConfigVal,
						uint8_t ucBitMask);

void FXOS8700CQ_Init (void);
void FXOS8700CQ_Mag_Calibration (void);
void writeMagntCalibrationValue(int16_t* psCalibOffsetVals);

void clearAccelMotionIntrpt();

int32_t standby_accelMagn_fxos8700();
#endif /* FXOS8700_H_ */
