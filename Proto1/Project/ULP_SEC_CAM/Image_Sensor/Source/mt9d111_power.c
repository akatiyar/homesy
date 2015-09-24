/*
 * mt9d111_power.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "mt9d111.h"

//******************************************************************************
//	This function puts MT9D111 in standby
//
//	param ucMethod - can either be HARD_STANDBY or SOFT_STANDBY
//	return SUCCESS or failure value
//
//	Sequence followed as per MT9D111 developer's guide
//******************************************************************************
int32_t EnterStandby_mt9d111(uint8_t ucMethod)
{
	int32_t lRetVal;

	s_RegList stndby_cmds_list[] = {
			{1, 0xC6, 0xA103},{1, 0xC8, 0x0001},	//Conext A/Preview; seq.cmd = 1(preview cmd)
			{1, 0xC6, 0xA104},{111, 0xC8, 0x0003},	//Wait till in A; seq.state = 3(preview state)
			{100, 0x00, 0x0064},//100ms delay

			{1, 0xC6, 0xA103},{1, 0xC8, 0x0003},	//Standby firmware; seq.cmd = 3(standby cmd)
			{100, 0x00, 0x01F4},//500ms delay
			//{100, 0x00, 0x0064},//100ms delay - doesnt always work
			{1, 0xC6, 0xA104},{111, 0xC8, 0x0009},	//Wait till stanby; seq.state = 9(standby state)

			{0, 0x65, 0xA000},  	//Bypass PLL
			//{0, 0x65, 0xE000},  	// Power DOWN PLL. Added additionally. Doesnt seem to have an effect

			//{1, 0x0A, 0x0488},	//I/O pad input clamp during standby
			{1, 0x0A, 0x0080},		//I/O pad input clamp during standby
			{0, 0x0D, 0x0040},		//high-impedance outputs when in standby state

			//Configure direction of GPIO pads as Output
			{1, 0xC6, 0x9078},{1, 0xC8, 0x0000},	//Pins 11:8
			{1, 0xC6, 0x9079},{1, 0xC8, 0x0000},	//Pins 7:0
			//Make the state of GPIO pins as 0
			{1, 0xC6, 0x9070},{1, 0xC8, 0x0000},	//Pins 11:8
			{1, 0xC6, 0x9071},{1, 0xC8, 0x0000},	//Pins 7:0
									};
	//s_RegList stndby_cmds = {0, 0x0D, 0x0044};	//Sensor standby - use for soft standby

	s_RegList stndby_cmds[] = 	{
								{0, 0x0D, 0x0044},
								{100, 0, 0x01F4},
								};	//Sensor standby - use for soft standby

	lRetVal = RegLstWrite(stndby_cmds_list, (sizeof(stndby_cmds_list)/sizeof(s_RegList)));
	RETURN_ON_ERROR(lRetVal);

	if(ucMethod == SOFT_STANDBY)
	{
		//lRetVal = RegLstWrite(&stndby_cmds,1);
		lRetVal = RegLstWrite(&stndby_cmds[0],2);
		RETURN_ON_ERROR(lRetVal);
	}
	else if(ucMethod == HARD_STANDBY)
	{
		MAP_GPIOPinWrite(GPIOA0_BASE, GPIO_PIN_7, GPIO_PIN_7);	//Sensor standby - use for hard standby
	}

	MT9D111Delay(SYSTEM_CLOCK/MT9D111_CLKIN_MIN * 10 + 100);
									//Wait before turning off Clock to Cam (XCLK)
									//Calcs in doc

	return lRetVal;
}

//******************************************************************************
//	This function takes MT9D111 out of standby
//
//	param ucMethod - can either be HARD_STANDBY or SOFT_STANDBY
//	return SUCCESS or failure value
//
//	Sequence followed as per MT9D111 developer's guide
//******************************************************************************
int32_t ExitStandby_mt9d111(uint8_t ucMethod)
{
	int32_t lRetVal;

	s_RegList stndby_cmds = {0, 0x0D, 0x0040};	//Sensor wake - soft

	s_RegList stndby_cmds_list[] =	{
				{1, 0xC6, 0xA103},{1, 0xC8, 0x0001},	//Conext A/Preview; seq.cmd = 1(preview cmd)
				{100, 0, 0x0064	},						//100ms delay
				{1, 0xC6, 0xA104},{111, 0xC8, 0x0003},	//Wait till in A; seq.state = 3(preview state)
				//{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 }  // SEQ_MODE Will turn off AE, AWB
									};

	MT9D111Delay(SYSTEM_CLOCK/MT9D111_CLKIN_MIN * 24 + 100);
									//Wait after turning ON Clock to Cam (XCLK)
									//Calcs in doc

	if(ucMethod == SOFT_STANDBY)
	{
		lRetVal = RegLstWrite(&stndby_cmds,1);
		RETURN_ON_ERROR(lRetVal);
	}
	else if(ucMethod == HARD_STANDBY)
	{
		MAP_GPIOPinWrite(GPIOA0_BASE, GPIO_PIN_7, 0);
	}

	lRetVal = RegLstWrite(stndby_cmds_list,
							(sizeof(stndby_cmds_list)/sizeof(s_RegList)));
	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}
