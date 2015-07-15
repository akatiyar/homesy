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

uint8_t open1_close0_invalid2;
uint8_t valid_case;
uint8_t avg_buffer_cnt;
uint8_t isitfirsttime;

float ang_buf[4];
float angle_reading_del1;
float angle_avg;
float angle_reading_del2;
float angle_reading_del3;
float angle_reading_del4;


//  Angle Mess solver
int gdoor_dirCW;
int gdoor_open_angle=0;//290
int gdoor_close_angle=0; //110
int gdoor_snap_angle=0;//120
int ginvalid_angle_entry;

#define RANGE_MIN 20
#define RANGE_MAX 120

//extern int32_t CollectTxit_ImgTempRH();
extern uint8_t g_flag_door_closing_45degree;

void check_doorpos()
{
	float angle_reg = thisSV_6DOF_GB_BASIC.fLPRho;// temp angle variable.. dont change the actual angle readings structure
	
		//------------------------------------------------------
		//If clock wise and the angle variations happen in the cross over region then add 360
		if(gdoor_dirCW)
		{
			if (angle_reg < gdoor_open_angle)// If angle is less than the minimum of the working range
			{
				angle_reg = angle_reg +360;
			}
		}
		else//If anti clock wise and the angle variations happen in the cross over region then add 360
		{
			if(angle_reg< gdoor_close_angle)// If angle is less than the minimum of the working range
			{
				angle_reg = angle_reg +360;
			}
			
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
		if( abs(angle_avg - angle_reg) > 20)
		{
			UART_PRINT("NOISE \n\r");// just ignore and exit this function
		}
		else// If noise free
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
			
			UART_PRINT("ANGLE=%3.2f\n", angle_avg);
			
			
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
				UART_PRINT("INVALID\n");
			}
			//------------------------------------------------------------------------
			
			
			
			//--------Find the SNAP point -------------------
			// Angle conditoion greater or lesser varies with CW or CCW hence the if loop
			if(gdoor_dirCW)//CW case
			{
				//check for the angle crossing and then wait for 45 degrees //CW case
				if(  (angle_avg < gdoor_open_angle) & (open1_close0_invalid2 == 1) )
				{
					valid_case = 1;
					UART_PRINT("OPEN\n");
					LED_Off();
				}
				//-----------------
				//Make it invalid if door got closed before reaching the snap position
				if((angle_avg > gdoor_close_angle) && gdoor_dirCW)
				{
					valid_case = 0;
					UART_PRINT("IVCASE\n");
				}
				//-------------------------
				
				//----***------Conditon check for image snap SNAP
				if( (angle_avg > gdoor_snap_angle) & (open1_close0_invalid2 == 0) & (valid_case == 1 ) & gdoor_dirCW)
				{
					valid_case = 0;
					UART_PRINT("\nS\n");
					g_flag_door_closing_45degree = 1;
				}	
				//---------------------------				
			}
			else//CCW case
			{
				//check for the angle crossing and then wait for 45 degrees //CCW case
				if(  (angle_avg > gdoor_open_angle) & (open1_close0_invalid2 == 1) )
				{
					valid_case = 1;
					UART_PRINT("OPEN\n");
					LED_Off();
				}
				//-----------------
				
				
				//Make it invalid if door got closed before reaching the snap position
				if((angle_avg < gdoor_close_angle) && (!gdoor_dirCW))
				{
					valid_case = 0;
					UART_PRINT("IVCASE\n");
				}
				//-----------------------------
				
				//----***------Conditon check for image snap SNAP
				if( (angle_avg < gdoor_snap_angle) & (open1_close0_invalid2 == 0) & (valid_case == 1 ) & !gdoor_dirCW)
				{
					valid_case = 0;
					UART_PRINT("\nS\n");
					g_flag_door_closing_45degree = 1;
				}
				//------------------------------		
			}
			
		
		}
	}




//clock wise if open angle is less than close angle CCW otherwise
void find_door_direction()
{
	gdoor_dirCW = 0;
	ginvalid_angle_entry = 0;
	
	
	//-------------- FIND door direction CW or CCW --------------------------
	if(abs(gdoor_close_angle - gdoor_open_angle) < RANGE_MIN)
	{// Weird angle readings so ignore it: Send response to app if possible
		ginvalid_angle_entry = 1;		
	}
	else if(abs(gdoor_close_angle - gdoor_open_angle) > RANGE_MAX)
	{ // Valid range but in the cross over region so 
		if(gdoor_close_angle > gdoor_open_angle)// somethign went to the 360 range or 0 range
		{ // close greater meaning it crossed to 359 from 1
			if(((gdoor_open_angle+360)-gdoor_close_angle ) > RANGE_MAX) // invalid angle ranges entered
			{ ginvalid_angle_entry = 1; }
			else // Counter clock wise
			{ gdoor_dirCW = 0;			}
		}
		else
		{// open greater meaning it crossed to 359 from 1
			if(((gdoor_close_angle+360)-gdoor_open_angle ) > RANGE_MAX) // invalid values enetered
			{	ginvalid_angle_entry = 1;		}
			else
			{	gdoor_dirCW = 1;				} // Clock wise yes but in the cross ov er region 
		}
	}
	else
	{// normal CW/CCW case
		if(gdoor_close_angle > gdoor_open_angle)
		{// greater then clock wise
			gdoor_dirCW = 1;	
		}
		else
		{// lesser then counter clock wise
			gdoor_dirCW = 0;	
		}	
	}	

}
