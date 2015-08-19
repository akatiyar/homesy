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
	ang_buf[avg_buffer_cnt] = angle_reg; // store the latest angle reading to the latest buffer
	angle_avg = (ang_buf[0] + ang_buf[1] + ang_buf[2] + ang_buf[3])/4;// average the buffer

	//--Buffer pointer circular
	if(avg_buffer_cnt==3)
	{ avg_buffer_cnt = 0; 		}
	else
	{ avg_buffer_cnt++; 		}
	//----------------------------------

	print_count++;
	if(print_count==20)
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

void Check_MinMax(float_t angle_avg)
{
	float angle_reg_afterOffset = 0;

	//*****************Min and max angles and their timestamps
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
	//DEBG_PRINT("%3.2f, Offset: %3.2f\n", angle_avg, angle_reg_afterOffset);
	//if(angle_reg < g_fMinAngle)
	if(angle_reg_afterOffset < g_fMinAngle)
	{
		g_fMinAngle = angle_reg_afterOffset;
		DEBG_PRINT("N");
		//DEBG_PRINT("MIN: %d\n", g_fMinAngle);
		//DEBG_PRINT("MIN: %d sec, %d nsec\n", g_Struct_TimeStamp_MinAngle.secs, g_Struct_TimeStamp_MinAngle.nsec);
		cc_rtc_get(&g_Struct_TimeStamp_MinAngle);
		//DEBG_PRINT("MIN: %d sec, %d nsec\n", g_Struct_TimeStamp_MinAngle.secs, g_Struct_TimeStamp_MinAngle.nsec);
		//DEBG_PRINT("%d milli sec\n", g_TimeStamp_minAngle);
	}
	//if(angle_reg > g_fMaxAngle)
	if (angle_reg_afterOffset > g_fMaxAngle)
	{
		g_fMaxAngle = angle_reg_afterOffset;
		DEBG_PRINT("X");
		//DEBG_PRINT("MAX: %d\n", g_fMaxAngle);
		//DEBG_PRINT("MAX: %d sec, %d nsec\n", g_Struct_TimeStamp_MaxAngle.secs, g_Struct_TimeStamp_MaxAngle.nsec);
		cc_rtc_get(&g_Struct_TimeStamp_MaxAngle);
		//DEBG_PRINT("MAX: %d sec, %d nsec\n", g_Struct_TimeStamp_MaxAngle.secs, g_Struct_TimeStamp_MaxAngle.nsec);
		//DEBG_PRINT("%d milli sec\n", g_TimeStamp_maxAngle);
	}
	//DEBG_PRINT("MIN: %d, MAX: %d\n", g_fMinAngle, g_fMaxAngle);

	return;
}
