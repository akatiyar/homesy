/*
 * algorithm_basic.c
 *
 *  Created on: 02-Jul-2015
 *      Author: Prakash
 */



//Prakash all globals
#include "include_all.h"
#include "app.h"
#include "math.h"
#include "timer_fns.h"
#include "accelomtrMagntomtr_fxos8700.h"
#include "app_common.h"



extern struct SV_6DOF_GB_BASIC thisSV_6DOF_GB_BASIC;

void check_doorpos();

float ang_buf[4];
float angle_avg;

float gdoor_90deg_angle;//290
float gdoor_40deg_angle; //110
float gdoor_OpenDeg_angle;


//extern int32_t CollectTxit_ImgTempRH();
extern uint8_t g_flag_door_closing_45degree;

uint8_t valid_case;
uint8_t avg_buffer_cnt;
uint8_t isitfirsttime;
uint8_t print_count;

extern struct MQXLiteGlobals mqxglobals;

void check_doorpos()
{
	float angle_reg = 0;

	if(thisSV_6DOF_GB_BASIC.fLPPhi<0)
	{
		angle_reg = thisSV_6DOF_GB_BASIC.fLPRho +180;
		if(angle_reg>360)
		{
			angle_reg = angle_reg-360;
		}
	}
	else
	{
		angle_reg = thisSV_6DOF_GB_BASIC.fLPRho;// temp angle variable.. dont change the actual angle readings structure
	}

	if(angle_reg < g_fMinAngle)
	{
		g_fMinAngle = angle_reg;
		cc_rtc_get(&g_Struct_TimeStamp_MinAngle);
		//UART_PRINT("%d milli sec\n", g_TimeStamp_minAngle);
	}
	if(angle_reg > g_fMaxAngle)
	{
		g_fMaxAngle = angle_reg;
		cc_rtc_get(&g_Struct_TimeStamp_MaxAngle);
		//UART_PRINT("%d milli sec\n", g_TimeStamp_maxAngle);
	}

	if(print_count==20)
	{
	//UART_PRINT("ANGLE=%3.2f\n", angle_reg);
	UART_PRINT("%3.2f\n", angle_reg);
	//UART_PRINT("%3.2f  %3.2f\n", angle_reg, thisSV_6DOF_GB_BASIC.fLPRho);
//	UART_PRINT("phi = %3.2f, theta = %3.2f, psi = %3.2f, rho = %3.2f, chi = %3.2f\n",
//			thisSV_6DOF_GB_BASIC.fLPPhi, thisSV_6DOF_GB_BASIC.fLPThe, thisSV_6DOF_GB_BASIC.fLPPsi,
//			thisSV_6DOF_GB_BASIC.fLPRho, thisSV_6DOF_GB_BASIC.fLPChi);
	print_count=0;

	}
	print_count++;

	//--------- Do only once -------------
	if(!isitfirsttime)
	{
		avg_buffer_cnt = 0;
		ang_buf[0] = angle_reg;
		ang_buf[1] = angle_reg;
		ang_buf[2]= angle_reg;
		ang_buf[3] = angle_reg;
		angle_avg = angle_reg;
		isitfirsttime = 1;

	}
	//------------------------------------

	//--------------Averaging : May not be required----------------------
	// Average the angle readings.. remove this incase if it is not relevant int he future
	ang_buf[avg_buffer_cnt] = angle_reg; // store the latest angle reading to the latest buffer
	angle_avg = (ang_buf[0] + ang_buf[1] + ang_buf[2] + ang_buf[3])/4;// average the buffer

	//--Buffer pointer circular
	if(avg_buffer_cnt==3)
	{ avg_buffer_cnt = 0; 		}
	else
	{ avg_buffer_cnt++; 		}
	//----------------------------------

	//check for the angle crossing for fridge opening //handle -5 crossing zero
	//if(  (angle_avg < (gdoor_90deg_angle+3)) && (angle_avg > (gdoor_90deg_angle-3)) && (valid_case == 0) )
	if(  (angle_avg < (gdoor_OpenDeg_angle+3)) && (angle_avg > (gdoor_OpenDeg_angle-3)) && (valid_case == 0) )
	{
		valid_case = 1;
		UART_PRINT("O \n");
		UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
		LED_Off();
		g_ucReasonForFailure = OPEN_NOTCLOSED;
		LED_Blink_2(.25,.25,BLINK_FOREVER);
	}
	//-----------------

	//----***------Conditon check for image snap SNAP
	if( (angle_avg < (gdoor_40deg_angle+3)) &&  (angle_avg > (gdoor_40deg_angle-3)) )
	{
		if ( valid_case == 1 )
		{
			valid_case = 0;
			UART_PRINT("S<>ANGLE=%3.2f \n", thisSV_6DOF_GB_BASIC.fLPRho);
			g_flag_door_closing_45degree = 1;
			LED_On();
		}
		else
		{
			g_ucReasonForFailure = NOTOPEN_CLOSED;
		}
	}
	//---------------------------
}
//--------------------------------------------------------------------------

#define FORTY_DEGREES		40	//degrees
#define NINETY_DEGREES		90	//degrees

#define OPEN				60	//degrees

#define A40_A90_DIFFMAG_MAX	120	//degrees

//Determines the Open angle by Interpolation from Angle90 and Angle40
float_t Calculate_DoorOpenThresholdAngle(float_t angle_40, float_t angle_90)
{
	float_t angle_openThreshold, angle_40_temp = 0, angle_90_temp = 0;
	float_t offset;

	UART_PRINT("Interpolation fn\n");
	UART_PRINT("40:%f 90:%f\n",angle_40, angle_90);
	if(abs(angle_40-angle_90) < A40_A90_DIFFMAG_MAX)	//No 360 degree crossover
	{
		UART_PRINT("No 360degree crossover\n");
		//Same interpolation formula works for both angle_90>angle_40 and
		//angle_90<angle_40
		angle_openThreshold = angle_40 + ((OPEN - FORTY_DEGREES) * ((angle_40 - angle_90)/(FORTY_DEGREES - NINETY_DEGREES)));
	}
	else		//i.e if there is a 360 degree crossover
	{
		UART_PRINT("360degree crossover\n");
		if(angle_40 > angle_90) //i.e if angle_40 in Q4 (300s) and angle_90 in Q1 (<90)
		{
			UART_PRINT("case1\n");
			//Calculate offset as the degrees between angle_40 and 360, plus a
			//margin to tip angle_40 over to Q1
			offset = (360 - angle_40 + 1);
			UART_PRINT("Offset: %f\n", offset);

			//Offset the angles so that crossover is eliminated
			angle_40_temp = angle_40 + offset; //should be 361 here. this can be removed
			UART_PRINT("40(=361):%f\n", angle_40_temp);
			angle_40_temp -= 360;	//should be 1 here. . this can be removed
			UART_PRINT("40(=1):%f\n", angle_40_temp);
			angle_90_temp += angle_90 + offset;
			UART_PRINT("90(+offset):%f\n", angle_90_temp);

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));
			UART_PRINT("Open:%f\n", angle_openThreshold);

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
			UART_PRINT("Open(-offset):%f\n", angle_openThreshold);

		}
		if(angle_90 > angle_40) //i.e if angle_90 in Q4 (300s) and angle_40 in Q1 (<90)
		{
			UART_PRINT("case2\n");
			//Calculate offset as the degrees between angle_90 and 360, plus a
			//margin(1) to tip angle_90 over to Q1
			offset = (360 - angle_90 + 1);
			UART_PRINT("Offset: %f\n", offset);

			//Offset the angles so that crossover is eliminated
			angle_90_temp = angle_90 + offset; //should be 361 here. this can be removed
			UART_PRINT("90(=361):%f\n", angle_90_temp);
			angle_90_temp -= 360;	//should be 1 here. . this can be removed
			UART_PRINT("90(=1):%f\n", angle_90_temp);
			angle_40_temp = angle_40 + offset;
			UART_PRINT("40(+offset):%f\n", angle_40_temp);

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));
			UART_PRINT("Open:%f\n", angle_openThreshold);

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
			UART_PRINT("Open(-offset):%f\n", angle_openThreshold);
		}
	}

	return angle_openThreshold;
}
