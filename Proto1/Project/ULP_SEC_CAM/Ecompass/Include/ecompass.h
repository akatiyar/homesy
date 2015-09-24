/*
 * ecompass.h
 *
 *  Created on: 18-Sep-2015
 *      Author: Chrysolin
 */

#ifndef ECOMPASS_INCLUDE_ECOMPASS_H_
#define ECOMPASS_INCLUDE_ECOMPASS_H_

#include "accelomtr_magntomtr_fxos8700.h"
#include "include_all.h"
#include "flash_files.h"
#include "timer_fns.h"

#define ANGLE_90			0
#define ANGLE_40			1

#define FORTY_DEGREES		40	//degrees
#define NINETY_DEGREES		90	//degrees

#define OPEN				60	//degrees

#define A40_A90_DIFFMAG_MAX	120	//degrees

extern struct SV_6DOF_GB_BASIC thisSV_6DOF_GB_BASIC;
extern struct MQXLiteGlobals mqxglobals;

float ang_buf[4];
float angle_avg;

float gdoor_90deg_angle;//290
float gdoor_40deg_angle; //110
float gdoor_OpenDeg_angle;
float g_angleOffset_to180;
extern uint8_t g_flag_door_closing_45degree;

void check_doorpos();
void Check_MinMax(float_t angle_avg);
float Calculate_DoorOpenThresholdAngle(float_t angle_40, float_t angle_90);
int16_t angleCheck();
int16_t angleCheck_Initializations();
float_t get_angle();
int16_t angleCheck_WithCalibration();
int16_t magnetometer_initialize();
int32_t Get_Calibration_MagSensor();
int32_t CollectAngle(uint8_t ucAngle);

#endif /* ECOMPASS_INCLUDE_ECOMPASS_H_ */
