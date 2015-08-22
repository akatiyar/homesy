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
void Check_MinMax(float_t angle_avg);

float ang_buf[4];
float angle_avg;

float gdoor_90deg_angle;//290
float gdoor_40deg_angle; //110
float gdoor_OpenDeg_angle;
float g_angleOffset_to180;


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
	if( (abs(angle_avg-angle_reg)) > 300)	//Crossover from 0 to 360 or 360 to 0
	{
//		DEBG_PRINT("Crossover\n");
		angle_avg = angle_reg;
		ang_buf[0] = angle_reg;
		ang_buf[1] = angle_reg;
		ang_buf[2]= angle_reg;
		ang_buf[3] = angle_reg;
	}
	else	//The usual case
	{
		ang_buf[avg_buffer_cnt] = angle_reg; // store the latest angle reading to the latest buffer
		angle_avg = (ang_buf[0] + ang_buf[1] + ang_buf[2] + ang_buf[3])/4;// average the buffer
	}

	//--Buffer pointer circular
	if(avg_buffer_cnt==3)
	{ avg_buffer_cnt = 0; 		}
	else
	{ avg_buffer_cnt++; 		}
	//----------------------------------

	print_count++;
	if(print_count==20)
	//if(print_count==5)
	{
		//DEBG_PRINT("ANGLE=%3.2f\n", angle_reg);
		RELEASE_PRINT("%3.2f\n", angle_avg);
		//DEBG_PRINT("%3.2f  %3.2f\n", angle_reg, thisSV_6DOF_GB_BASIC.fLPRho);

		/*//----------------------Print all angle related values------------------
		int i,j;
		DEBG_PRINT("Magnetic fld:\n");
		DEBG_PRINT("%3.2f  %3.2f  %3.2f\n", thisMag.fBc[X], thisMag.fBc[Y], thisMag.fBc[Z]);

		DEBG_PRINT("Acceleration:\n");
		DEBG_PRINT("%3.2f  %3.2f  %3.2f\n", thisAccel.fGp[X], thisAccel.fGp[Y], thisAccel.fGp[Z]);

		DEBG_PRINT("Rotation Matrix:\n");
		for(i=0;i<3;i++)
		{
			for(j=0;j<3;j++)
				DEBG_PRINT("%3.2f   ", thisSV_6DOF_GB_BASIC.fR[i][j]);
			DEBG_PRINT("\n");
		}
		DEBG_PRINT("phi = %3.2f, theta = %3.2f, psi = %3.2f, rho = %3.2f, chi = %3.2f, del = %3.2f\n",
				thisSV_6DOF_GB_BASIC.fLPPhi, thisSV_6DOF_GB_BASIC.fLPThe, thisSV_6DOF_GB_BASIC.fLPPsi,
				thisSV_6DOF_GB_BASIC.fLPRho, thisSV_6DOF_GB_BASIC.fLPChi, thisSV_6DOF_GB_BASIC.fLPDelta);*/
		//----------------------
		print_count=0;
	}

	Check_MinMax(angle_avg);

	//check for the angle crossing for fridge opening //handle -5 crossing zero
	//if(  (angle_avg < (gdoor_90deg_angle+3)) && (angle_avg > (gdoor_90deg_angle-3)) && (valid_case == 0) )
	if( (angle_avg < (gdoor_OpenDeg_angle+3)) && (angle_avg > (gdoor_OpenDeg_angle-3)) && (valid_case == 0) )
	{
		valid_case = 1;
		RELEASE_PRINT("O  %3.2f\n", angle_avg);
		cc_rtc_get(&g_Struct_TimeStamp_OpenAngle);
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
			RELEASE_PRINT("S  %3.2f\n", angle_avg);
			cc_rtc_get(&g_Struct_TimeStamp_SnapAngle);
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

	if(abs(angle_40-angle_90) < A40_A90_DIFFMAG_MAX)	//No 360 degree crossover
	{
		//Same interpolation formula works for both angle_90>angle_40 and
		//angle_90<angle_40
		angle_openThreshold = angle_40 + ((OPEN - FORTY_DEGREES) * ((angle_40 - angle_90)/(FORTY_DEGREES - NINETY_DEGREES)));
	}
	else		//i.e if there is a 360 degree crossover
	{
		if(angle_40 > angle_90) //i.e if angle_40 in Q4 (300s) and angle_90 in Q1 (<90)
		{
			//Calculate offset as the degrees between angle_40 and 360, plus a
			//margin to tip angle_40 over to Q1
			offset = (360 - angle_40 + 1);

			//Offset the angles so that crossover is eliminated
			angle_40_temp = angle_40 + offset; //should be 361 here. this can be removed
			angle_40_temp -= 360;	//should be 1 here. . this can be removed
			angle_90_temp += angle_90 + offset;

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
		}
		if(angle_90 > angle_40) //i.e if angle_90 in Q4 (300s) and angle_40 in Q1 (<90)
		{
			//Calculate offset as the degrees between angle_90 and 360, plus a
			//margin(1) to tip angle_90 over to Q1
			offset = (360 - angle_90 + 1);

			//Offset the angles so that crossover is eliminated
			angle_90_temp = angle_90 + offset; //should be 361 here. this can be removed
			angle_90_temp -= 360;	//should be 1 here. . this can be removed
			angle_40_temp = angle_40 + offset;

			//Calculate the OpenAngle using the interpolation equation
			angle_openThreshold = angle_40_temp + ((OPEN - FORTY_DEGREES) * ((angle_40_temp - angle_90_temp)/(FORTY_DEGREES - NINETY_DEGREES)));

			//Undo the offset
			angle_openThreshold -= offset;
			if(angle_openThreshold < 0)
			{
				angle_openThreshold += 360;
			}
		}
	}

	DEBG_PRINT("60 degree: %f\n",angle_openThreshold);

	return angle_openThreshold;
}

//******************************************************************************
// Checks if the angle passed as parameter is min/max. If it is min/max it is
//	stored in min/max angle global variable and the current timestamp is also
//	stored in the corresponding timestamp variable
//******************************************************************************
void Check_MinMax(float_t angle_avg)
{
	float angle_reg_afterOffset = 0;

	//Offset to get angles in Q2 and Q3
	angle_reg_afterOffset = angle_avg + g_angleOffset_to180;
	if(angle_reg_afterOffset > 360)
	{
		angle_reg_afterOffset -= 360;
	}
	else if(angle_reg_afterOffset < 0)
	{
		angle_reg_afterOffset += 360;
	}

	// Invalid case. Angle ramping up from 0 to 360 or down from 360 to 0
//	if((angle_reg_afterOffset > 240)||(angle_reg_afterOffset < 120))
//	{
//		return;
//	}

	if(angle_reg_afterOffset < g_fMinAngle)
	{
		g_fMinAngle = angle_reg_afterOffset;
		//g_RawMinAngle = angle_avg;
//		DEBG_PRINT("N");
		//DEBG_PRINT("MIN: %d\n", g_fMinAngle);
		cc_rtc_get(&g_Struct_TimeStamp_MinAngle);
	}
	if (angle_reg_afterOffset > g_fMaxAngle)
	{
		g_fMaxAngle = angle_reg_afterOffset;
		//g_RawMaxAngle = angle_avg;
//		DEBG_PRINT("X");
		//DEBG_PRINT("MAX: %d\n", g_fMaxAngle);
		cc_rtc_get(&g_Struct_TimeStamp_MaxAngle);
	}

	if(angle_avg < g_RawMinAngle)
	{
		g_RawMinAngle = angle_avg;
	}
	if(angle_avg > g_RawMaxAngle)
	{
		g_RawMaxAngle = angle_avg;
	}

	return;
}

//******************************************************************************
//	Calculates the true min/max door angles from the offset_to180 min/max angles
//
//STEPS:
//	(1)reversing offset and handling out-of-range angle(negative and >360)
//	(2)swappping min and max incase Angle90<Angle40
//Swaps timestamps also if door angles are swapped
//******************************************************************************
void Calculate_TrueMinMaxAngles()
{
	float temp_swap_variable;
	long long temp_swap_variable_uint64;

	DEBG_PRINT("TrueMinMaxAngles()\n");
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Reverse offset
	g_fMinAngle -= g_angleOffset_to180;
	g_fMaxAngle -= g_angleOffset_to180;
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Handle <0 and >360 cases
	if(g_fMaxAngle < 0)
	{
		g_fMaxAngle += 360;
	}
	else if(g_fMaxAngle > 360)
	{
		g_fMaxAngle -= 360;
	}
	if(g_fMinAngle < 0)
	{
		g_fMinAngle += 360;
	}
	else if(g_fMinAngle > 360)
	{
		g_fMinAngle -= 360;
	}
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	//Swap the min and max angles if Angle90<Angle40
	if (((gdoor_90deg_angle < gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) < A40_A90_DIFFMAG_MAX))
		|| ((gdoor_90deg_angle > gdoor_40deg_angle) && (abs(gdoor_90deg_angle-gdoor_40deg_angle) > A40_A90_DIFFMAG_MAX)))
	{
		temp_swap_variable = g_fMinAngle;
		g_fMinAngle = g_fMaxAngle;
		g_fMaxAngle = temp_swap_variable;

		temp_swap_variable_uint64 = g_TimeStamp_MaxAngle;
		g_TimeStamp_MaxAngle = g_TimeStamp_MinAngle;
		g_TimeStamp_MinAngle = temp_swap_variable_uint64;
	}
	DEBG_PRINT("%d, %d\n", g_fMinAngle, g_fMaxAngle);

	return;
}
