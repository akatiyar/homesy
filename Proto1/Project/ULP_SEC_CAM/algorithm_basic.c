/*
 * algorithm_basic.c
 *
 *  Created on: 02-Jul-2015
 *      Author: Prakash
 */



//Prakash all globals
#include "FXOS8700 Frescale Lib/include_all.h"
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


//extern int32_t CollectTxit_ImgTempRH();
extern uint8_t g_flag_door_closing_45degree;

uint8_t valid_case;
uint8_t avg_buffer_cnt;
uint8_t isitfirsttime;
uint8_t print_count;

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

	if(print_count==20)
	{
	UART_PRINT("ANGLE=%3.2f\n", angle_reg);
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
	if(  (angle_avg < (gdoor_90deg_angle+3)) && (angle_avg > (gdoor_90deg_angle-3)) && (valid_case == 0) )
	{
		valid_case = 1;
		UART_PRINT("O \n");
		UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
		LED_Off();
	}
	//-----------------

	//----***------Conditon check for image snap SNAP
	if( (angle_avg < (gdoor_40deg_angle+3)) &&  (angle_avg > (gdoor_40deg_angle-3)) && (valid_case == 1 ))
	{
		valid_case = 0;
		UART_PRINT("S<>ANGLE=%3.2f \n", thisSV_6DOF_GB_BASIC.fLPRho);
		g_flag_door_closing_45degree = 1;
	}
	//---------------------------
}
//--------------------------------------------------------------------------


