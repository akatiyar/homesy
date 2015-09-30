/*
 * algorithm_basic.c
 *
 *  Created on: 02-Jul-2015
 *      Author: Prakash
 */

#include <accelomtr_magntomtr_fxos8700.h>
#include <ecompass.h>
#include <include_all.h>
#include "app.h"
#include "math.h"
#include "timer_fns.h"
#include "app_common.h"

uint8_t valid_case;	// If this variable is set, snap-angle match is valid. The
					// variable is cleared during initialization and set when
					// door open is detected
uint8_t avg_buffer_cnt;
uint8_t isitfirsttime;
uint8_t print_count;

//******************************************************************************
//	This function checks if door-open or door-at-snap-position condition is met
//
//	Output global variable: g_flag_door_closing_45degree is set if snap position
//						is detected
//
//	Input global variable: This function uses thisSV_6DOF_GB_BASIC.fLPRho.
//						So it should be called after the getAngle function which
//						puts the current angle in thisSV_6DOF_GB_BASIC.fLPRho
//******************************************************************************
void check_doorpos()
{
	float angle_reg = 0;

	//--------To prevent 180 degree crossover for small tilts of device--------
	//Why not @have thisSV_6DOF_GB_BASIC.fLPPhi passed as a parameter to this fn??
	if(thisSV_6DOF_GB_BASIC.fLPPhi<0)
	{
		//temp angle variable.. dont change the actual angle readings structure
		angle_reg = thisSV_6DOF_GB_BASIC.fLPRho +180;
		if(angle_reg>360)
		{
			angle_reg = angle_reg-360;
		}
	}
	else
	{
		angle_reg = thisSV_6DOF_GB_BASIC.fLPRho;
	}
	//------------------------------------

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
	// Average the angle readings.. remove this incase if it is not relevant in
	//the future
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

	//----Buffer pointer circular------
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

	//Check if the angle is min(max). This is for ground data.
	Check_MinMax(angle_avg);

	//------check for the angle crossing for fridge opening------
	//if(  (angle_avg < (gdoor_90deg_angle+3)) && (angle_avg > (gdoor_90deg_angle-3)) && (valid_case == 0) )
	if( (angle_avg < (gdoor_OpenDeg_angle+3)) && (angle_avg > (gdoor_OpenDeg_angle-3)) && (valid_case == 0) )
	{
		valid_case = 1;
		RELEASE_PRINT("O  %3.2f\n", angle_avg);
		cc_rtc_get(&g_Struct_TimeStamp_OpenAngle);
		g_ucReasonForFailure = OPEN_NOTCLOSED;
		//LED_Blink_2(.25,.25,BLINK_FOREVER);
	}
	//-----------------

	//------Conditon check for image SNAP------
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
