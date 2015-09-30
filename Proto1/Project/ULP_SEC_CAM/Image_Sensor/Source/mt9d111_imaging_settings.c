/*
 * mt9d111_image_settings.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "mt9d111.h"

//*****************************************************************************
//	Write values to the MT9D111 registers that control exposure and white
//	balance of the image
//*****************************************************************************
long WriteAllAEnAWBRegs()
{
	long lRetVal;

/*	s_RegList StatusRegLst[] = {{0x00, 0x2B, 0x0026},
								{0x00, 0x2C, 0x0030},
								{0x00, 0x2D, 0x002E},
								{0x00, 0x2E, 0x0026},

								{0x01, 0x6E, 0x008A},
								{0x01, 0x6A, 0x00A9},
								{0x01, 0x6B, 0x008A},
								{0x01, 0x6C, 0x008A},
								{0x01, 0x6D, 0x0085},
								{0x01, 0x4E, 0x0020},

								{0x01, 0x60, 0x3923},
								{0x01, 0x61, 0x04E4},
								{0x01, 0x62, 0x91D7},
								{0x01, 0x63, 0x8319},
								{0x01, 0x64, 0x41E4},
								{0x01, 0x65, 0x894B},
								{0x01, 0x66, 0x3F99},

								{0x00, 0x09, 0x0274},
								{0x00, 0x0C, 0x0000}};
*/

	// Fridge
		s_RegList StatusRegLst[] = {
//									{0x00, 0x2B, 0x0024},	//No lines, too dark. tfss-faff768d-e0f2-4aea-8741-dd388459522b-myPic1.jpg
//									{0x00, 0x2C, 0x003A},
//									{0x00, 0x2D, 0x0024},
//									{0x00, 0x2E, 0x0024},

//									{0x00, 0x2B, ((0x0024<<1))},
//									{0x00, 0x2C, ((0x003A<<1))},
//									{0x00, 0x2D, ((0x0024<<1))},
//									{0x00, 0x2E, ((0x0024<<1))},

									{0x00, 0x2B, ((0x0024<<1)|0x0080)},	//Lines, but few. tfss-3c55789c-a5b6-464b-862d-30baaec62370-myPic1.jpg
									{0x00, 0x2C, ((0x003A<<1)|0x0080)},
									{0x00, 0x2D, ((0x0024<<1)|0x0080)},
									{0x00, 0x2E, ((0x0024<<1)|0x0080)},

	//								{0x00, 0x2B, ((0x0024<<1)|0x0180)},	//Lines exist. tfss-9203b65b-f7ea-42b8-b9e8-be5366de68bf-myPic1.jpg
	//								{0x00, 0x2C, ((0x003A<<1)|0x0180)},
	//								{0x00, 0x2D, ((0x0024<<1)|0x0180)},
	//								{0x00, 0x2E, ((0x0024<<1)|0x0180)},

	//								{0x00, 0x2B, (0x0024|0x0180)},	//Lines exist. tfss-cdf82004-4241-425f-96ce-3f68092bebce-myPic1.jpg
	//								{0x00, 0x2C, (0x003A|0x0180)},
	//								{0x00, 0x2D, (0x0024|0x0180)},
	//								{0x00, 0x2E, (0x0024|0x0180)},

	//								{0x00, 0x2B, (0x0024|0x0080)},
	//								{0x00, 0x2C, (0x003A|0x0080)},
	//								{0x00, 0x2D, (0x0024|0x0080)},
	//								{0x00, 0x2E, (0x0024|0x0080)},

	//								{0x00, 0x2B, 0x003F},
	//								{0x00, 0x2C, 0x003F},
	//								{0x00, 0x2D, 0x003F},
	//								{0x00, 0x2E, 0x003F},

	//								{0x00, 0x2B, ((0x0024<<1)|0x0080|0x0200)},
	//								{0x00, 0x2C, ((0x003A<<1)|0x0080|0x0200)},
	//								{0x00, 0x2D, ((0x0024<<1)|0x0080|0x0200)},
	//								{0x00, 0x2E, ((0x0024<<1)|0x0080|0x0200)},

									{0x01, 0x6E, 0x0085},
									{0x01, 0x6A, 0x008D},
									{0x01, 0x6B, 0x008D},
									{0x01, 0x6C, 0x0093},
									{0x01, 0x6D, 0x008d},
									{0x01, 0x4E, 0x0020},

	//								{0x01, 0x6E, 0x0085<<1},
	//								{0x01, 0x6A, 0x008D<<1},
	//								{0x01, 0x6B, 0x008D<<1},
	//								{0x01, 0x6C, 0x0093<<1},
	//								{0x01, 0x6D, 0x008d<<1},
	//								{0x01, 0x4E, 0x0020<<1},

									{0x01, 0x60, 0x291A},
									{0x01, 0x61, 0x04E4},
									{0x01, 0x62, 0xbab5},
									{0x01, 0x63, 0xb001},
									{0x01, 0x64, 0x4089},
									{0x01, 0x65, 0xf17c},
									{0x01, 0x66, 0x3fDA},

									//{0x00, 0x09, 0x03ae},	//122mSec
									//{0x00, 0x09, (0x03ae>>2)},	//30mSec
//									{0x00, 0x09, (0x03ae>>3)},	//15mSec
//									{0x00, 0x09, (0x005E)},	//10mSec
									{0x00, 0x09, (0x0060)},	//10mSec
									{0x00, 0x0C, 0x0000}};

/*
	// Imaging Test Chart on my table
	s_RegList StatusRegLst[] = {{0x00, 0x2B, 0x0022},
								{0x00, 0x2C, 0x002e},
								{0x00, 0x2D, 0x0026},
								{0x00, 0x2E, 0x0022},

								{0x01, 0x6E, 0x008f},
								{0x01, 0x6A, 0x00b3},
								{0x01, 0x6B, 0x008f},
								{0x01, 0x6C, 0x008f},
								{0x01, 0x6D, 0x0085},
								{0x01, 0x4E, 0x0020},

								{0x01, 0x60, 0x3923},
								{0x01, 0x61, 0x04e4},
								{0x01, 0x62, 0xbbeb},
								{0x01, 0x63, 0x8b12},
								{0x01, 0x64, 0x3fe9},
								{0x01, 0x65, 0xad5c},
								{0x01, 0x66, 0x3fb0},

								{0x00, 0x09, 0x044b},
								{0x00, 0x0C, 0x0000}};
*/

/*
	// North table board with L32 On, L16 off
		s_RegList StatusRegLst[] = {
	//								{0x00, 0x2B, 0x0024},	//No lines - tfss-0e8f6d78-479f-4c7b-a185-7b3d8cb7deba-myPic1.jpg
	//								{0x00, 0x2C, 0x0032},
	//								{0x00, 0x2D, 0x0028},
	//								{0x00, 0x2E, 0x0024},

	//								{0x00, 0x2B, ((0x0024<<1)|0x0180)},	//Lines in image - tfss-0c901042-8950-402d-8320-cd371457bc2d-myPic1.jpg
	//								{0x00, 0x2C, ((0x0032<<1)|0x0180)},
	//								{0x00, 0x2D, ((0x0028<<1)|0x0180)},
	//								{0x00, 0x2E, ((0x0024<<1)|0x0180)},

	//								{0x00, 0x2B, (0x0024<<1)},	//No lines in image - tfss-6ac7bb74-facd-42f0-a3a4-68951d09a0a8-myPic1.jpg
	//								{0x00, 0x2C, (0x0032<<1)},
	//								{0x00, 0x2D, (0x0028<<1)},
	//								{0x00, 0x2E, (0x0024<<1)},

	//								{0x00, 0x2B, ((0x0024<<1)|0x0080)},	//Lines(1/3 images taken), but less, - tfss-fed6f19c-3a58-49ff-846e-4eb446a0c06f-myPic1.jpg
	//								{0x00, 0x2C, ((0x0032<<1)|0x0080)},
	//								{0x00, 0x2D, ((0x0028<<1)|0x0080)},
	//								{0x00, 0x2E, ((0x0024<<1)|0x0080)},

	//								{0x00, 0x2B, ((0x0024)|0x0180)},	//No lines(double checked) - tfss-5abb704c-cc72-41a6-8d2d-f82c66c4f006-myPic1.jpg
	//								{0x00, 0x2C, ((0x0032)|0x0180)},
	//								{0x00, 0x2D, ((0x0028)|0x0180)},
	//								{0x00, 0x2E, ((0x0024)|0x0180)},

									{0x01, 0x6E, 0x00aa},
									{0x01, 0x6A, 0x0088},
									{0x01, 0x6B, 0x0088},
									{0x01, 0x6C, 0x0085},
									{0x01, 0x6D, 0x0088},
									{0x01, 0x4E, 0x0020},

									{0x01, 0x60, 0x3923},
									{0x01, 0x61, 0x04e4},
									{0x01, 0x62, 0xbbe8},
									{0x01, 0x63, 0x8711},
									{0x01, 0x64, 0x3ce3},
									{0x01, 0x65, 0xae5d},
									{0x01, 0x66, 0x3faf},

									//{0x00, 0x09, 0x0274},	//Auto Value
									{0x00, 0x09, 0x004E},	//10msec
									//{0x00, 0x09, 0x005D},	//12msec
									//{0x00, 0x09, 0x005D<<1},	//20msec
									{0x00, 0x0C, 0x0000}};
*/

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	return lRetVal;
}

//*****************************************************************************
//	Read values from the MT9D111 registers that control exposure and white
//	balance of the image
//*****************************************************************************
long ReadAllAEnAWBRegs()
{
	long lRetVal;

	lRetVal = AnalogGainReg_Read();
	RETURN_ON_ERROR(lRetVal);
	lRetVal = DigitalGainRegs_Read();
	RETURN_ON_ERROR(lRetVal);
	lRetVal = CCMRegs_Read();
	RETURN_ON_ERROR(lRetVal);
	lRetVal = ShutterRegs_Read();
	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}

//*****************************************************************************
//	Enable Auto White Balance driver
//*****************************************************************************
long enableAWB()
{
	long lRetVal;
	uint16_t usRegVal;

	s_RegList StatusRegLst[] = {
								{1, 0xC6, 0xA102}, {1, 0xC8, 0x0004},
								};

	lRetVal = RegLstWrite(StatusRegLst, 1);
	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);

	StatusRegLst[1].usValue |= usRegVal;

	lRetVal = RegLstWrite(StatusRegLst, 2);

	return lRetVal;
}

//*****************************************************************************
//	Disable Auto White Balance driver
//*****************************************************************************
long disableAWB()
{
	long lRetVal;
	uint16_t usRegVal;

	s_RegList StatusRegLst[] = {
								{1, 0xC6, 0xA102}, {1, 0xC8, 0x0004},
								};

	lRetVal = RegLstWrite(StatusRegLst, 1);
	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);

	StatusRegLst[1].usValue = usRegVal & ~(StatusRegLst[1].usValue);

	lRetVal = RegLstWrite(StatusRegLst, 2);

	return lRetVal;
}

//*****************************************************************************
//	Enable Auto Exposure driver
//*****************************************************************************
long enableAE()
{
	long lRetVal;
	uint16_t usRegVal;

	s_RegList StatusRegLst[] = {
								{1, 0xC6, 0xA102}, {1, 0xC8, 0x0001},
								};

	lRetVal = RegLstWrite(StatusRegLst, 1);
	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);

	StatusRegLst[1].usValue |= usRegVal;

	lRetVal = RegLstWrite(StatusRegLst, 2);

	return lRetVal;
}

//*****************************************************************************
//	Disable Auto Exposure driver
//*****************************************************************************
long disableAE()
{
	long lRetVal;
	uint16_t usRegVal;

	s_RegList StatusRegLst[] = {
								{1, 0xC6, 0xA102}, {1, 0xC8, 0x0001},
								};

	lRetVal = RegLstWrite(StatusRegLst, 1);
	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);

	StatusRegLst[1].usValue = usRegVal & ~(StatusRegLst[1].usValue);

	//lRetVal = RegLstWrite(StatusRegLst, 2);
	lRetVal = RegLstWrite(&StatusRegLst[1], 1);

	return lRetVal;
}

//*****************************************************************************
//	Read color correction matrix registers
//*****************************************************************************
long CCMRegs_Read()
{
	long lRetVal;
	uint16_t usRegVal;
	int i;

	DEBG_PRINT("CCM Regs(0x60-0x67):\n");
	s_RegList StatusRegLst[] = {{0x01, 0x60, 0xBADD},
								{0x01, 0x61, 0xBADD},
								{0x01, 0x62, 0xBADD},
								{0x01, 0x63, 0xBADD},
								{0x01, 0x64, 0xBADD},
								{0x01, 0x65, 0xBADD},
								{0x01, 0x66, 0xBADD}};

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read digital gain registers
//*****************************************************************************
long DigitalGainRegs_Read()
{
	long lRetVal;
	uint16_t usRegVal;
	int i;

	DEBG_PRINT("DGain Regs(0x6A-0x6E,0x4E):\n");
	s_RegList StatusRegLst[] = {{0x01, 0x6A, 0xBADD},
								{0x01, 0x6B, 0xBADD},
								{0x01, 0x6C, 0xBADD},
								{0x01, 0x6D, 0xBADD},
								{0x01, 0x6E, 0xBADD},
								{0x01, 0x4E, 0xBADD}};

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read shutter width register - related to integration time
//*****************************************************************************
long ShutterRegs_Read()
{
	long lRetVal;
	uint16_t usRegVal;
	int i;
	s_RegList StatusRegLst[] = {{0x00, 0x09, 0xBADD},
									{0x00, 0x0C, 0xBADD}};

	DEBG_PRINT("Shutter Width, Delay Regs:\n");

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read analog gain registers
//*****************************************************************************
long AnalogGainReg_Read()
{
	long lRetVal;
	uint16_t usRegVal;
	int i;
	s_RegList StatusRegLst[] = {{0x00, 0x2B, 0xBADD},
									{0x00, 0x2C, 0xBADD},
									{0x00, 0x2D, 0xBADD},
									{0x00, 0x2E, 0xBADD}};

	DEBG_PRINT("AGain Regs:\n");

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
		//DEBG_PRINT("Register Val: %x\n\r", StatusRegLst[i].usValue);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read registers that decide the pixel clock
//*****************************************************************************
long PCLK_Rate_read()
{
	long lRetVal;
	uint16_t usRegVal;
	int i;


	s_RegList StatusRegLst[] = {{0x02, 0x0E, 0xBADD},
								{0x02, 0x0F, 0xBADD},
								{0x02, 0x02, 0xBADD},
								{0x02, 0x0D, 0xBADD}};

	DEBG_PRINT("O/P Clk Reg:\n");

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
		//DEBG_PRINT("Register Val: %x\n\r", StatusRegLst[i].usValue);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read a collection of registers that decide the image configuration
//
//	param	RegValues - pointer to where the register values have to be stored
//*****************************************************************************
int32_t ReadImageConfigReg(uint16_t *RegValues)
{
	long lRetVal;
	int i;

	s_RegList StatusRegLst[] = {{0x00, 0x2B, 0xBADD},
								{0x00, 0x2C, 0xBADD},
								{0x00, 0x2D, 0xBADD},
								{0x00, 0x2E, 0xBADD},
								{0x00, 0x09, 0xBADD},
								{0x00, 0x0C, 0xBADD},
								{0x01, 0x4E, 0xBADD},
								{0x01, 0x60, 0xBADD},
								{0x01, 0x61, 0xBADD},
								{0x01, 0x62, 0xBADD},
								{0x01, 0x63, 0xBADD},
								{0x01, 0x64, 0xBADD},
								{0x01, 0x65, 0xBADD},
								{0x01, 0x6E, 0xBADD},
								{0x01, 0x6A, 0xBADD},
								{0x01, 0x6B, 0xBADD},
								{0x01, 0x6C, 0xBADD},
								{0x01, 0x6D, 0xBADD},

								{0x01, 0x30, 0xBADD},
								{0x01, 0x31, 0xBADD},
								{0x01, 0x32, 0xBADD},
								{0x01, 0x35, 0xBADD},
								{0x01, 0x36, 0xBADD},
								{0x01, 0x3B, 0xBADD},
								{0x01, 0x3C, 0xBADD},
								{0x01, 0x66, 0xBADD},

								};

	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&StatusRegLst[i], RegValues);
		DEBG_PRINT("RegVal %x:%x\n",StatusRegLst[i].ucRegAddr , *RegValues);
		RegValues++;
	}
	return lRetVal;
}

//******************************************************************************
//	Read all configuration register
//	Function is used for debug purposes
//******************************************************************************
int32_t Read_AllRegisters()
{
	uint16_t i;
	uint16_t regVal;

	uint16_t variable_list[] = {0xA115, 0x2003, 0xA002, 0xA361, 0xAB04, 0xA104};

	s_RegList reg_list[] = {

				{0, 0x09, 0xBADD},
				{0, 0x0A, 0xBADD},
				{0, 0x0B, 0xBADD},
				{0, 0x0C, 0xBADD},

				{0, 0x65, 0xBADD},
				{0, 0x66, 0xBADD},
				{0, 0x67, 0xBADD},

				{2, 0x00, 0xBADD},
				{2, 0x02, 0xBADD},
				{2, 0x03, 0xBADD},
				{2, 0x04, 0xBADD},

				/*{0, 0x33, 0xBADD},
				{0, 0x38, 0xBADD},
				{2, 0x80, 0xBADD}, // LENS_CORRECTION_CONTROL
			    {2, 0x81, 0xBADD}, // ZONE_BOUNDS_X1_X2
			    {2, 0x82, 0xBADD}, // ZONE_BOUNDS_X0_X3
			    {2, 0x83, 0xBADD}, // ZONE_BOUNDS_X4_X5
			    {2, 0x84, 0xBADD}, // ZONE_BOUNDS_Y1_Y2
			    {2, 0x85, 0xBADD}, // ZONE_BOUNDS_Y0_Y3
			    {2, 0x86, 0xBADD}, // ZONE_BOUNDS_Y4_Y5
			    {2, 0x87, 0xBADD}, // CENTER_OFFSET
			    {2, 0x88, 0xBADD}, // FX_RED
			    {2, 0x89, 0xBADD}, // FX_GREEN
			    {2, 0x8A, 0xBADD}, // FX_BLUE
			    {2, 0x8B, 0xBADD}, // FY_RED
			    {2, 0x8C, 0xBADD}, // FY_GREEN
			    {2, 0x8D, 0xBADD}, // FY_BLUE
			    {2, 0x8E, 0xBADD}, // DF_DX_RED
			    {2, 0x8F, 0xBADD}, // DF_DX_GREEN
			    {2, 0x90, 0xBADD}, // DF_DX_BLUE
			    {2, 0x91, 0xBADD}, // DF_DY_RED
			    {2, 0x92, 0xBADD}, // DF_DY_GREEN
			    {2, 0x93, 0xBADD}, // DF_DY_BLUE
			    {2, 0x94, 0xBADD}, // SECOND_DERIV_ZONE_0_RED
			    {2, 0x95, 0xBADD}, // SECOND_DERIV_ZONE_0_GREEN
			    {2, 0x96, 0xBADD}, // SECOND_DERIV_ZONE_0_BLUE
			    {2, 0x97, 0xBADD}, // SECOND_DERIV_ZONE_1_RED
			    {2, 0x98, 0xBADD}, // SECOND_DERIV_ZONE_1_GREEN
			    {2, 0x99, 0xBADD}, // SECOND_DERIV_ZONE_1_BLUE
			    {2, 0x9A, 0xBADD}, // SECOND_DERIV_ZONE_2_RED
			    {2, 0x9B, 0xBADD}, // SECOND_DERIV_ZONE_2_GREEN
			    {2, 0x9C, 0xBADD}, // SECOND_DERIV_ZONE_2_BLUE
			    {2, 0x9D, 0xBADD}, // SECOND_DERIV_ZONE_3_RED
			    {2, 0x9E, 0xBADD}, // SECOND_DERIV_ZONE_3_GREEN
			    {2, 0x9F, 0xBADD}, // SECOND_DERIV_ZONE_3_BLUE
			    {2, 0xA0, 0xBADD}, // SECOND_DERIV_ZONE_4_RED
			    {2, 0xA1, 0xBADD}, // SECOND_DERIV_ZONE_4_GREEN
			    {2, 0xA2, 0xBADD}, // SECOND_DERIV_ZONE_4_BLUE
			    {2, 0xA3, 0xBADD}, // SECOND_DERIV_ZONE_5_RED
			    {2, 0xA4, 0xBADD}, // SECOND_DERIV_ZONE_5_GREEN
			    {2, 0xA5, 0xBADD}, // SECOND_DERIV_ZONE_5_BLUE
			    {2, 0xA6, 0xBADD}, // SECOND_DERIV_ZONE_6_RED
			    {2, 0xA7, 0xBADD}, // SECOND_DERIV_ZONE_6_GREEN
			    {2, 0xA8, 0xBADD}, // SECOND_DERIV_ZONE_6_BLUE
			    {2, 0xA9, 0xBADD}, // SECOND_DERIV_ZONE_7_RED
			    {2, 0xAA, 0xBADD}, // SECOND_DERIV_ZONE_7_GREEN
			    {2, 0xAB, 0xBADD}, // SECOND_DERIV_ZONE_7_BLUE
			    {2, 0xAC, 0xBADD}, // X2_FACTORS
			    {2, 0xAD, 0xBADD}, // GLOBAL_OFFSET_FXY_FUNCTION
			    {2, 0xAE, 0xBADD}, // K_FACTOR_IN_K_FX_FY
			    {1, 0x08, 0xBADD}, // COLOR_PIPELINE_CONTROL
			    {1, 0x1F, 0xBADD}, // RESERVED_SOC1_1F
			    {1, 0x51, 0xBADD}, // RESERVED_SOC1_51
			    {0, 0x33, 0xBADD}, // RESERVED_CORE_33
			    {0, 0x38, 0xBADD}, // RESERVED_CORE_38
				{1, 0x1F, 0xBADD}, // RESERVED_SOC1_1F
				{1, 0x08, 0xBADD}, // COLOR_PIPELINE_CONTROL
				{1, 0x08, 0xBADD}, // COLOR_PIPELINE_CONTROL
				{1, 0x08, 0xBADD}, // COLOR_PIPELINE_CONTROL
				{1, 0x36, 0xBADD}, // APERTURE_PARAMETERS

			    {0, 0x66, 0xBADD},
				{0, 0x67, 0xBADD},
				{0, 0x65, 0xBADD},*/
			};

		for (i = 0; i < (sizeof(reg_list)/sizeof(s_RegList)); i++)
		{
			Register_Read(&reg_list[i], &regVal);
			DEBG_PRINT("\n0x%x:0x%x", reg_list[i].ucRegAddr, regVal);
		}

		for (i=0; i< (sizeof(variable_list)/sizeof(uint16_t)); i++)
		{
			Variable_Read(variable_list[i], &regVal);
			DEBG_PRINT("\n0x%x:0x%x", variable_list[i], regVal);
		}

		return 0;
}

//*****************************************************************************
//	Sets the shutter width - related to integration time
//
//	param	ShutterWidth
//*****************************************************************************
int32_t SetShutterWidth(uint16_t ShutterWidth)
{
	return Reg_Write(0x00,0x09,ShutterWidth);
}

//*****************************************************************************
//	Sets the digital gain registers
//
//	param	G1Gain - Gain of green 1 pixels
//	param	BGain - Gain of blue pixels
//	param	RGain - Gain of red pixels
//	param	G2Gain - Gain of green 2 pixels
//
//	All these params can take values 1 or 3 only.
//	Param = 1 multiplies gain by 2
//			3 multiplies gain by 4
//*****************************************************************************
int32_t SetAnalogGain(uint8_t G1Gain,uint8_t BGain,uint8_t RGain,uint8_t G2Gain)
{
	int32_t lRetVal;
	uint16_t tempVal=0;

	lRetVal = Reg_Read(0x00,0x2B,&tempVal);
	tempVal = tempVal & ~ANALOG_GAIN_BITS;
	tempVal = tempVal | (G1Gain << 7);
	lRetVal = Reg_Write(0x00,0x2B,tempVal);

	lRetVal = Reg_Read(0x00,0x2C,&tempVal);
	tempVal = tempVal & ~ANALOG_GAIN_BITS;
	tempVal = tempVal | (BGain <<7) ;
	lRetVal = Reg_Write(0x00,0x2C,tempVal);

	lRetVal = Reg_Read(0x00,0x2D,&tempVal);
	tempVal = tempVal & ~ANALOG_GAIN_BITS;
	tempVal = tempVal | (RGain<<7);
	lRetVal = Reg_Write(0x00,0x2D,tempVal);

	lRetVal = Reg_Read(0x00,0x2E,&tempVal);
	tempVal = tempVal & ~ANALOG_GAIN_BITS;
	tempVal = tempVal | (G2Gain<<7);
	lRetVal = Reg_Write(0x00,0x2E,tempVal);

	return lRetVal;
}

//*****************************************************************************
//	Sets the digital gain registers
//
//	param	G1Gain - Gain of green 1 pixels
//	param	BGain - Gain of blue pixels
//	param	RGain - Gain of red pixels
//	param	G2Gain - Gain of green 2 pixels
//
//	All these params can take values 1 or 3 or 7 only.
//	Param = 1 multiplies gain by 2
//			3 multiplies gain by 4
//			7 multiplies gain by 8
//*****************************************************************************
int32_t SetDigitalGain(uint8_t G1Gain,uint8_t BGain,uint8_t RGain,uint8_t G2Gain)
{
	int32_t lRetVal;
	uint16_t tempVal=0;

	lRetVal = Reg_Read(0x00,0x2B,&tempVal);
	tempVal = tempVal & ~DIGITAL_GAIN_BITS;
	tempVal = tempVal | (G1Gain << 9);
	lRetVal = Reg_Write(0x00,0x2B,tempVal);

	lRetVal = Reg_Read(0x00,0x2C,&tempVal);
	tempVal = tempVal & ~DIGITAL_GAIN_BITS;
	tempVal = tempVal | (BGain <<9) ;
	lRetVal = Reg_Write(0x00,0x2C,tempVal);

	lRetVal = Reg_Read(0x00,0x2D,&tempVal);
	tempVal = tempVal & ~DIGITAL_GAIN_BITS;
	tempVal = tempVal | (RGain<<9);
	lRetVal = Reg_Write(0x00,0x2D,tempVal);

	lRetVal = Reg_Read(0x00,0x2E,&tempVal);
	tempVal = tempVal & ~DIGITAL_GAIN_BITS;
	tempVal = tempVal | (G2Gain<<9);
	lRetVal = Reg_Write(0x00,0x2E,tempVal);

	return lRetVal;
}

//*****************************************************************************
//	Sets the initial gain
//
//	param	ValChange - value by which initial gain should be increased /
//						decreased
//						LSB weight = 0.03125
//						Max init gain (7-bit): 128
//						The increment/ decrement value should be within the
//							range. Otherwise, overflow will happen
//	param	IsInc - tells whether to increase gain or decrease gain
//*****************************************************************************
int32_t SetInitialGain(uint8_t ValChange, bool IsInc)
{
	int32_t lRetVal;
	uint16_t CurGain=0,NewGain=0,tmpIGain=0;
	uint8_t Reg_Addr = 0x2B;
	uint8_t Cnt=0;

	for (Cnt=0;Cnt<4;Cnt++)
	{
		lRetVal = Reg_Read(0x00,(Reg_Addr+Cnt),&CurGain);

		NewGain = CurGain & ~INITIAL_GAIN_BITS;		//new gain initialised 0
		tmpIGain = CurGain &  INITIAL_GAIN_BITS;	//previous initial gain

		if(IsInc)
		{
			tmpIGain = tmpIGain + ValChange;
		}
		else
		{
			tmpIGain = tmpIGain - ValChange;
		}

		NewGain = NewGain | (tmpIGain & INITIAL_GAIN_BITS);
		lRetVal = Reg_Write(0x00,(Reg_Addr+Cnt),NewGain);

		RETURN_ON_ERROR(lRetVal);
	}

	return lRetVal;
}

//*****************************************************************************
//	Read the 4 gain registers (G1, R, B, G2)
//
//	Pointer where the values of these registers are stored
//*****************************************************************************
int32_t ReadGainReg(uint16_t* Gains)
{
	uint8_t Reg_Addr = 0x2B;
	uint8_t Cnt=0;
	int32_t lRetVal;

	for (Cnt=0;Cnt<4;Cnt++)
	{
		lRetVal = Reg_Read(0x00,(Reg_Addr+Cnt),Gains + Cnt);
		DEBG_PRINT("Reg %x : %x\n",(Reg_Addr+Cnt),*(Gains + Cnt));
		RETURN_ON_ERROR(lRetVal);
	}
	return lRetVal;
}
