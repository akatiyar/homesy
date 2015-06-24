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

//#define DOORCLOSE_DEG  355
//#define IMAGEPOS_DEG   (350-5)
//#define OPEN_DEG	330

//	PCB4
//#define DOORCLOSE_DEG	361
//#define IMAGEPOS_DEG	359
//#define OPEN_DEG		345

//	PCB5 at Krishna's Fridge
#define IMAGEPOS_DEG	120
#define DOORCLOSE_DEG	(IMAGEPOS_DEG - 5)
#define OPEN_DEG		290

#define ANG_RANGE		0
#define ANG_RANGE_VAL	180

float ang_buf[4];
float angle_reading_del1;
float angle_avg;
float angle_reading_del2;
float angle_reading_del3;
float angle_reading_del4;

//extern int32_t CollectTxit_ImgTempRH();
extern uint8_t g_flag_door_closing_45degree;

void check_doorpos()
{

		if(thisSV_6DOF_GB_BASIC.fLPRho<ANG_RANGE_VAL)
		{
			#ifdef ANG_RANGE == 0
			thisSV_6DOF_GB_BASIC.fLPRho = thisSV_6DOF_GB_BASIC.fLPRho +360;
			#endif

		}

		if(!isitfirsttime)
		{
			avg_buffer_cnt = 0;
			ang_buf[0] = thisSV_6DOF_GB_BASIC.fLPRho;
			ang_buf[1] = thisSV_6DOF_GB_BASIC.fLPRho;
			ang_buf[2]= thisSV_6DOF_GB_BASIC.fLPRho;
			ang_buf[3] = thisSV_6DOF_GB_BASIC.fLPRho;
			angle_avg = thisSV_6DOF_GB_BASIC.fLPRho;
			isitfirsttime = 1;
		}
//		if(   abs(angle_avg - thisSV_6DOF_GB_BASIC.fLPRho) > 20 )
//		{
//			UART_PRINT("SPIKE \n\r");
//			ang_buf[0] = thisSV_6DOF_GB_BASIC.fLPRho;
//			ang_buf[1] = thisSV_6DOF_GB_BASIC.fLPRho;
//			ang_buf[2]= thisSV_6DOF_GB_BASIC.fLPRho;
//			ang_buf[3] = thisSV_6DOF_GB_BASIC.fLPRho;
//			angle_avg = thisSV_6DOF_GB_BASIC.fLPRho;
//		}
//		else
		{
			ang_buf[avg_buffer_cnt] = thisSV_6DOF_GB_BASIC.fLPRho;
			angle_avg = (ang_buf[0] + ang_buf[1] + ang_buf[2] + ang_buf[3])/4;

			if(avg_buffer_cnt==3)
			{
				avg_buffer_cnt = 0;
			}
			else
			{
				avg_buffer_cnt++;
			}
		}
		angle_reading_del4 = angle_reading_del3;
		angle_reading_del3 = angle_reading_del2;
		angle_reading_del2 = angle_reading_del1;
		angle_reading_del1 = angle_avg;

		UART_PRINT("ANGLE=%3.2f\n", angle_avg);

		//Find door closing or opening based ont he angle reading change
		if( (angle_reading_del1 > angle_reading_del2) && (angle_reading_del2 > angle_reading_del3) && (angle_reading_del3 >angle_reading_del4) )
		{
			open1_close0_invalid2 = 0;
			UART_PRINT("Close\n");
		}
		else if((angle_reading_del1 < angle_reading_del2) && (angle_reading_del2 < angle_reading_del3) && (angle_reading_del3 < angle_reading_del4) )
		{
			open1_close0_invalid2 = 1;
		}
		else
		{
			open1_close0_invalid2 = 2;
			UART_PRINT("INVALID\n");
		}
		//---------------------------

		//check for the angle crossing and then wait for 45 degrees
		if(  (thisSV_6DOF_GB_BASIC.fLPRho < OPEN_DEG) & (open1_close0_invalid2 == 1) )
		{
			valid_case = 1;
			UART_PRINT("OPEN\n");
			LED_Off();
		}
		else if(thisSV_6DOF_GB_BASIC.fLPRho>DOORCLOSE_DEG)
		{
			valid_case = 0;
			UART_PRINT("IVCASE\n");
		}
		//------------------------------

		//Conditon check for image snap
		if( (thisSV_6DOF_GB_BASIC.fLPRho > IMAGEPOS_DEG) & (open1_close0_invalid2 == 0) & (valid_case == 1 ) )
		{
			valid_case = 0;
			UART_PRINT("\nS\n");
			g_flag_door_closing_45degree = 1;
		}
		//------------------------------
}






