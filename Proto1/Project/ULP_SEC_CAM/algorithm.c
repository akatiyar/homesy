/*
 * algorithm
 *
 *  Created on: 16-Jun-2015
 *      Author: Chrysolin
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
void find_door_direction();

float ang_buf[4];
float angle_reading_del1;
float angle_avg;
float angle_reading_del2;
float angle_reading_del3;
float angle_reading_del4;

//  Angle Mess solver
uint8_t gdoor_dirCW;
float gdoor_90deg_angle;//290
float gdoor_40deg_angle; //110
float gdoor_snap_angle;//120
uint8_t ginvalid_angle_entry;
float angle_min;


#define RANGE_MIN 20
#define RANGE_MAX 120

//extern int32_t CollectTxit_ImgTempRH();
extern uint8_t g_flag_door_closing_45degree;

uint8_t open1_close0_invalid2;
uint8_t valid_case;
uint8_t avg_buffer_cnt;
uint8_t isitfirsttime;

void check_doorpos()
{

	//gdoor_90deg_angle = 20;//290
	//gdoor_40deg_angle = 100;//120

	//Find the door direction first.. this should be moved to an init function only once to be done
	find_door_direction();
	//---------------------------------------------

	float angle_reg = thisSV_6DOF_GB_BASIC.fLPRho;// temp angle variable.. dont change the actual angle readings structure
	UART_PRINT("ANGLE=%3.2f\n", angle_reg);

	//------------------------------------------------------
	// If the measured angle is less than the minimum angle then add 360 to it considering 360 cross over
		if (angle_reg < angle_min)// If angle is less than the minimum of the working range
		{
			angle_reg = angle_reg +360;
		}

	//--------------------------------------------------------------

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



	//--------------code where the snap point is found----------------------
	//vv Remove Noise from getting in to the buffer
//	if( abs(angle_avg - angle_reg) > 100)
//	{
//		UART_PRINT("NOISE \n\r");// just ignore and exit this function
//	}
//	else// If noise free
	{
		// Average the angle readings.. remove this incase if it is not relevant int he future
		ang_buf[avg_buffer_cnt] = angle_reg; // store the latest angle reading to the latest buffer
		angle_avg = (ang_buf[0] + ang_buf[1] + ang_buf[2] + ang_buf[3])/4;// average the buffer

		//--Buffer pointer circular
		if(avg_buffer_cnt==3)
		{ avg_buffer_cnt = 0; 		}
		else
		{ avg_buffer_cnt++; 		}
		//----------------------------------

		//---  Keep 3 delayed values of the averaged angle reading to check increase or decrease of angle with time
		angle_reading_del4 = angle_reading_del3;
		angle_reading_del3 = angle_reading_del2;
		angle_reading_del2 = angle_reading_del1;
		angle_reading_del1 = angle_avg;
		//-------------------------------------------------------------------

		//UART_PRINT("ANGLE=%3.2f\n", angle_avg);


		//--------  Detect closing or opening of fridge -------------------------
		//Find door closing or opening based on the angle reading change
		//increasing CW->closing  CCW-> opening
		if( (angle_reading_del1 > angle_reading_del2) && (angle_reading_del2 > angle_reading_del3) && (angle_reading_del3 >angle_reading_del4) )
		{//increasing
			open1_close0_invalid2 = !gdoor_dirCW;
		}
		//decreasing CW-> opening CCW-> closing
		else if((angle_reading_del1 < angle_reading_del2) && (angle_reading_del2 < angle_reading_del3) && (angle_reading_del3 < angle_reading_del4) )
		{//decreasing
				open1_close0_invalid2 = gdoor_dirCW;
		}
		else// invalid
		{
			open1_close0_invalid2 = 2;  // no constant increase or decrease so invalid
			//UART_PRINT("INVALID\n");
		}
		//------------------------------------------------------------------------


		//--------Find the SNAP point -------------------
		// Angle conditoion greater or lesser varies with CW or CCW hence the if loop
		if(gdoor_dirCW)//CW case
		{
			//check for the angle crossing and then wait for 45 degrees //CW case
			if(  (angle_avg < gdoor_90deg_angle) && (angle_avg > (gdoor_90deg_angle-10)) && (valid_case == 0)/* & (open1_close0_invalid2 == 1)*/ )
			{
				valid_case = 1;
				UART_PRINT("O \n");
				UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
				LED_Off();
			}
			//-----------------

			//----***------Conditon check for image snap SNAP
			if( (angle_avg > gdoor_40deg_angle) &&  (angle_avg < (gdoor_40deg_angle+10)) &&/* (open1_close0_invalid2 == 0) &*/ (valid_case == 1 ) && gdoor_dirCW )
			{
				valid_case = 0;
				UART_PRINT("\nS\n");
				UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
				g_flag_door_closing_45degree = 1;
			}
			//---------------------------
		}
		else//CCW case
		{
			//check for the angle crossing and then wait for 45 degrees //CCW case
			if(  (angle_avg > gdoor_90deg_angle) && (angle_avg < (gdoor_90deg_angle+10)) && (valid_case == 0) /* & (open1_close0_invalid2 == 1) */)
			{
				valid_case = 1;
				UART_PRINT("O \n");
				UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
				LED_Off();
			}
			//-----------------

			//----***------Conditon check for image snap SNAP //handle -10 Tag
			if( (angle_avg < gdoor_40deg_angle) &&  (angle_avg > (gdoor_40deg_angle-10)) && /*(open1_close0_invalid2 == 0) &*/ (valid_case == 1 ) && !gdoor_dirCW)
			{
				valid_case = 0;
				UART_PRINT("\nS\n");
				UART_PRINT("ANGLE=%3.2f\n", thisSV_6DOF_GB_BASIC.fLPRho);
				g_flag_door_closing_45degree = 1;
			}
			//------------------------------
		}
	}
}
//--------------------------------------------------------------------------
//-------------------------------------------------------------------------





//--------------------------  DOOR DIRECTION ---------------------------------------------------
//------------------------------------------------------

//clock wise if open angle is less than close angle CCW otherwise
void find_door_direction()
{
	gdoor_dirCW = 0;
	ginvalid_angle_entry = 0;
	angle_min = 180;

	//-------------- FIND door direction CW or CCW --------------------------
	// Assuming 90deg and 40deg cannot have an angle difference of RANG_MAX. If there is then it is a 360 cross over
	// for example 350 to 80 degree will give a difference of 270 which implies there is a zero cross over or an error
	// In the above case 360 + 80 is 440 and 440 -350 should not cross RANGE_MAX if corssed then wrong entry.
	// This applies viceversa on CW and CCW
	if(abs(gdoor_40deg_angle - gdoor_90deg_angle) < RANGE_MIN)
	{// Weird angle readings so ignore it: Send response to app if possible
		ginvalid_angle_entry = 1;

	}
	else if(abs(gdoor_40deg_angle - gdoor_90deg_angle) > RANGE_MAX)
	{ // Valid range but in the cross over region so
		if(gdoor_40deg_angle > gdoor_90deg_angle)// somethign went to the 360 range or 0 range
		{ // close greater meaning it crossed to 359 from 1
			if( ( (gdoor_90deg_angle+360)-gdoor_40deg_angle ) > RANGE_MAX  ) // invalid angle ranges entered
			{ ginvalid_angle_entry = 1; }
			else // Counter clock wise
			{ gdoor_dirCW = 0;
			gdoor_90deg_angle = gdoor_90deg_angle + 360;}
		}
		else
		{// open greater meaning it crossed to 359 from 1
			if(  (  (gdoor_40deg_angle+360)-gdoor_90deg_angle ) > RANGE_MAX) // invalid values enetered
			{	ginvalid_angle_entry = 1;		}
			else
			{	gdoor_dirCW = 1;
			gdoor_40deg_angle = gdoor_40deg_angle + 360;} // Clock wise yes but in the cross ov er region
		}
	}
	else
	{// normal CW/CCW case
		if(gdoor_40deg_angle > gdoor_90deg_angle)
		{// greater then clock wise
			gdoor_dirCW = 1;
		}
		else
		{// lesser then counter clock wise
			gdoor_dirCW = 0;
		}
	}

	//---------------------------------
	if(gdoor_dirCW) //Find a minum angle below which if the actual reafdign goes then add 360 because it iscrossing 360
	{ //clockwise then 90deg is minimum
		angle_min = gdoor_90deg_angle;
	}
	else
	{//anticlockwise then 40deg is minimum
		angle_min = gdoor_40deg_angle;
	}
	//------------------------------------

	//---------------------------------
	//Handle subtraction not to go below zero
	if(angle_min>RANGE_MIN)
	{
		angle_min = angle_min - RANGE_MIN;
	}
	//---------------------------------
}
//-----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------






