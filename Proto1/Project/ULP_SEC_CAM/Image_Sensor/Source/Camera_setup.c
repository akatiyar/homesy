//*****************************************************************************
// camera_app.c
//
// camera application macro & APIs
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
//*****************************************************************************
//
//! \addtogroup camera_app
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "math.h"

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_camera.h"
#include "rom_map.h"
#include "rom.h"
#include "prcm.h"
#include "udma.h"
#include "timer.h"
#include "utils.h"
#include "camera.h"

// Simplelink includes
#include "simplelink.h"

#include "i2cconfig.h"
#include "pinmux.h"
#include "camera_app.h"
#include "mt9d111.h"
#include "app.h"
#include "app_common.h"
#include <accelomtr_magntomtr_fxos8700.h>
#include <include_all.h>	//fxos
#include <temp_rh_sens_si7020.h>
#include "network_related_fns.h"
#include "flash_files.h"

bool g_bCameraOn= true;

extern void DMAConfig();
//*****************************************************************************
// Macros
//*****************************************************************************
#define RETRIES_MAX_MT9D111STANDBY		10


typedef enum pictureRequest{
    NO_PICTURE = 0x00,
    SINGLE_HIGH_RESOLUTION = 0x01,
    STREAM_LOW_RESOLUTION = 0x02
      
}e_pictureRequest;

typedef enum pictureFormat{
    RAW_10BIT = 0,
    ITU_R_BT601,
    YCbCr_4_2_2,
    YCbCr_4_2_0,
    RGB_565,
    RGB_555,
    RGB_444

}e_pictureFormat;

typedef enum pictureResolution{
    QVGA = 0,
    VGA,
    SVGA,
    XGA,
    uXGA

}e_pictureResolution;


#ifdef ENABLE_JPEG
#define FORMAT_YCBCR422   0
#define FORMAT_YCBCR420   1
#define FORMAT_MONOCHROME 2

//unsigned char JPEG_StdQuantTblY[64] =
//{
//    16,  11,  10,  16,  24,  40,  51,  61,
//    12,  12,  14,  19,  26,  58,  60,  55,
//    14,  13,  16,  24,  40,  57,  69,  56,
//    14,  17,  22,  29,  51,  87,  80,  62,
//    18,  22,  37,  56,  68,  109, 103, 77,
//    24,  35,  55,  64,  81,  104, 113, 92,
//    49,  64,  78,  87, 103,  121, 120, 101,
//    72,  92,  95,  98, 112,  100, 103,  99
//};
//
//unsigned char JPEG_StdQuantTblC[64] =
//{
//    17,  18,  24,  47,  99,  99,  99,  99,
//    18,  21,  26,  66,  99,  99,  99,  99,
//    24,  26,  56,  99,  99,  99,  99,  99,
//    47,  66,  99,  99,  99,  99,  99,  99,
//    99,  99,  99,  99,  99,  99,  99,  99,
//    99,  99,  99,  99,  99,  99,  99,  99,
//    99,  99,  99,  99,  99,  99,  99,  99,
//    99,  99,  99,  99,  99,  99,  99,  99
//};
////
//// This table is used for regular-position to zigzagged-position lookup
////  This is Figure A.6 from the ISO/IEC 10918-1 1993 specification
////
//static unsigned char zigzag[64] =
//{
//    0, 1, 5, 6,14,15,27,28,
//    2, 4, 7,13,16,26,29,42,
//    3, 8,12,17,25,30,41,43,
//    9,11,18,24,31,40,44,53,
//    10,19,23,32,39,45,52,54,
//    20,22,33,38,46,51,55,60,
//    21,34,37,47,50,56,59,61,
//    35,36,48,49,57,58,62,63
//};
//
//unsigned int JPEG_StdHuffmanTbl[384] =
//{
//    0x100, 0x101, 0x204, 0x30b, 0x41a, 0x678, 0x7f8, 0x9f6,
//    0xf82, 0xf83, 0x30c, 0x41b, 0x679, 0x8f6, 0xaf6, 0xf84,
//    0xf85, 0xf86, 0xf87, 0xf88, 0x41c, 0x7f9, 0x9f7, 0xbf4,
//    0xf89, 0xf8a, 0xf8b, 0xf8c, 0xf8d, 0xf8e, 0x53a, 0x8f7,
//    0xbf5, 0xf8f, 0xf90, 0xf91, 0xf92, 0xf93, 0xf94, 0xf95,
//    0x53b, 0x9f8, 0xf96, 0xf97, 0xf98, 0xf99, 0xf9a, 0xf9b,
//    0xf9c, 0xf9d, 0x67a, 0xaf7, 0xf9e, 0xf9f, 0xfa0, 0xfa1,
//    0xfa2, 0xfa3, 0xfa4, 0xfa5, 0x67b, 0xbf6, 0xfa6, 0xfa7,
//    0xfa8, 0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad, 0x7fa, 0xbf7,
//    0xfae, 0xfaf, 0xfb0, 0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5,
//    0x8f8, 0xec0, 0xfb6, 0xfb7, 0xfb8, 0xfb9, 0xfba, 0xfbb,
//    0xfbc, 0xfbd, 0x8f9, 0xfbe, 0xfbf, 0xfc0, 0xfc1, 0xfc2,
//    0xfc3, 0xfc4, 0xfc5, 0xfc6, 0x8fa, 0xfc7, 0xfc8, 0xfc9,
//    0xfca, 0xfcb, 0xfcc, 0xfcd, 0xfce, 0xfcf, 0x9f9, 0xfd0,
//    0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0xfd8,
//    0x9fa, 0xfd9, 0xfda, 0xfdb, 0xfdc, 0xfdd, 0xfde, 0xfdf,
//    0xfe0, 0xfe1, 0xaf8, 0xfe2, 0xfe3, 0xfe4, 0xfe5, 0xfe6,
//    0xfe7, 0xfe8, 0xfe9, 0xfea, 0xfeb, 0xfec, 0xfed, 0xfee,
//    0xfef, 0xff0, 0xff1, 0xff2, 0xff3, 0xff4, 0xff5, 0xff6,
//    0xff7, 0xff8, 0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe,
//    0x30a, 0xaf9, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
//    0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7,
//    0x101, 0x204, 0x30a, 0x418, 0x419, 0x538, 0x678, 0x8f4,
//    0x9f6, 0xbf4, 0x30b, 0x539, 0x7f6, 0x8f5, 0xaf6, 0xbf5,
//    0xf88, 0xf89, 0xf8a, 0xf8b, 0x41a, 0x7f7, 0x9f7, 0xbf6,
//    0xec2, 0xf8c, 0xf8d, 0xf8e, 0xf8f, 0xf90, 0x41b, 0x7f8,
//    0x9f8, 0xbf7, 0xf91, 0xf92, 0xf93, 0xf94, 0xf95, 0xf96,
//    0x53a, 0x8f6, 0xf97, 0xf98, 0xf99, 0xf9a, 0xf9b, 0xf9c,
//    0xf9d, 0xf9e, 0x53b, 0x9f9, 0xf9f, 0xfa0, 0xfa1, 0xfa2,
//    0xfa3, 0xfa4, 0xfa5, 0xfa6, 0x679, 0xaf7, 0xfa7, 0xfa8,
//    0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad, 0xfae, 0x67a, 0xaf8,
//    0xfaf, 0xfb0, 0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5, 0xfb6,
//    0x7f9, 0xfb7, 0xfb8, 0xfb9, 0xfba, 0xfbb, 0xfbc, 0xfbd,
//    0xfbe, 0xfbf, 0x8f7, 0xfc0, 0xfc1, 0xfc2, 0xfc3, 0xfc4,
//    0xfc5, 0xfc6, 0xfc7, 0xfc8, 0x8f8, 0xfc9, 0xfca, 0xfcb,
//    0xfcc, 0xfcd, 0xfce, 0xfcf, 0xfd0, 0xfd1, 0x8f9, 0xfd2,
//    0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0xfd8, 0xfd9, 0xfda,
//    0x8fa, 0xfdb, 0xfdc, 0xfdd, 0xfde, 0xfdf, 0xfe0, 0xfe1,
//    0xfe2, 0xfe3, 0xaf9, 0xfe4, 0xfe5, 0xfe6, 0xfe7, 0xfe8,
//    0xfe9, 0xfea, 0xfeb, 0xfec, 0xde0, 0xfed, 0xfee, 0xfef,
//    0xff0, 0xff1, 0xff2, 0xff3, 0xff4, 0xff5, 0xec3, 0xff6,
//    0xff7, 0xff8, 0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe,
//    0x100, 0x9fa, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
//    0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7,
//    0x100, 0x202, 0x203, 0x204, 0x205, 0x206, 0x30e, 0x41e,
//    0x53e, 0x67e, 0x7fe, 0x8fe, 0xfff, 0xfff, 0xfff, 0xfff,
//    0x100, 0x101, 0x102, 0x206, 0x30e, 0x41e, 0x53e, 0x67e,
//    0x7fe, 0x8fe, 0x9fe, 0xafe, 0xfff, 0xfff, 0xfff, 0xfff
//};
#endif

//*****************************************************************************
//	This function
//	1. Enables and configures the camera peripheral of CC3200
//	2. Starts supplying clock to image sensor
//	3. Resets the image sensor
//	4. Configures image sensor registers
//*****************************************************************************
int32_t Config_CameraCapture()
{
	int32_t lRetVal;

	DEBG_PRINT("Cam Config\n");
	CamControllerInit();	// Init parallel camera interface of cc3200
							// since image sensor needs XCLK for
							//its I2C module to work

	UtilsDelay(24/3 + 10);	// Initially, 24 clock cycles needed by MT9D111
							// 10 is margin

	lRetVal = SoftReset_ImageSensor();
	RETURN_ON_ERROR(lRetVal);

	lRetVal = CameraSensorInit();	//Initial configurations
	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//	This function begins the stream of image captures by writing into MT9D111
//	registers through I2C
//******************************************************************************
int32_t Start_CameraCapture()
{
	int32_t lRetVal;

	DEBG_PRINT("\nCam Start:\n");

	lRetVal = StartSensorInJpegMode();
	RETURN_ON_ERROR(lRetVal);

	disableAE();
	RETURN_ON_ERROR(lRetVal);
	disableAWB();
	RETURN_ON_ERROR(lRetVal);
	WriteAllAEnAWBRegs();
	RETURN_ON_ERROR(lRetVal);
	Refresh_mt9d111Firmware();
	RETURN_ON_ERROR(lRetVal);
	//UtilsDelay(80000000/6);

	return lRetVal;
}

//******************************************************************************
//	Sets up the global variables used in image capture and storage
//******************************************************************************
void ImagCapture_Init()
{
    g_block_lastFilled = -1;
    g_position_in_block = 0;
	memset((void*)g_flag_blockFull, 0x00 ,NUM_BLOCKS_IN_IMAGE_BUFFER);
	g_readHeader = 0;
	g_flag_DataBlockFilled = 0;
	//memset((void*)g_image_buffer, 0x00, IMAGE_BUF_SIZE_BYTES);

	//
	// Configure DMA in ping-pong mode
	//
	DMAConfig();

	return;
}

//******************************************************************************
//	Sends commands to MT9D111 restart image capture after waking up from standby
//******************************************************************************
int32_t ReStart_CameraCapture(uint16_t* pImageConfig)
{
	int32_t lRetVal;

	DEBG_PRINT("\nCam Restart:\n");

	lRetVal = RestartSensorInJpegMode(pImageConfig);

	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//	Restart image capture in preview mode
//
// Note : NWP should be ON and g_image buffer is used
//******************************************************************************
int32_t Restart_Camera()
{
	Standby_ImageSensor();
	osi_Sleep(30);
	Wakeup_ImageSensor();

	uint16_t* ImageConfigData = (uint16_t *) &(g_image_buffer[SENSOR_CONFIGS_OFFSET_BUF/sizeof(long)]);
	//ImageConfigData = ImageConfigData +4096;
	// Reading the file, since write erases contents already present
	ReadFile_FromFlash((uint8_t*)ImageConfigData,
						(uint8_t*)FILENAME_SENSORCONFIGS,
						CONTENT_LENGTH_SENSORS_CONFIGS, 0);

	ReStart_CameraCapture(ImageConfigData + (OFFSET_MT9D111/sizeof(uint16_t)));
	return 0;
}

//*****************************************************************************
//	Puts image sensor in standbyand turns off clock supplied to image sensor
//	Pair it with Wakeup_ImageSensor()
//*****************************************************************************
int32_t Standby_ImageSensor()
{
	int32_t lRetVal;
	uint8_t ucTryNum;

	DEBG_PRINT("\nCam Stndby:\n");

	//lRetVal = EnterStandby_mt9d111(HARD_STANDBY);	//Hard Standby is drawing more current
	lRetVal = EnterStandby_mt9d111(SOFT_STANDBY);
	ucTryNum = 1;
	while((lRetVal == MT9D111_FIRMWARE_STATE_ERROR) && (ucTryNum <= RETRIES_MAX_MT9D111STANDBY))
	{
		DEBG_PRINT("\nRetry\n");
		SoftReset_ImageSensor();
		CameraSensorInit();
		Start_CameraCapture();
		lRetVal = EnterStandby_mt9d111(SOFT_STANDBY);
		if(lRetVal == 0)
		{
			DEBG_PRINT("\nSuccess on retry\n");
		}
		else
		{
			ucTryNum++;
			DEBG_PRINT("\nFailure on Retry\n");
		}
	}
	//RETURN_ON_ERROR(lRetVal);

	if(ucTryNum > RETRIES_MAX_MT9D111STANDBY)	//Failed after maximum number of retries
	{
		// Want to do a whole device reset here with SYSOFF pin
		DEBG_PRINT("Standby Failure #@ #@\n");
		lRetVal = MT9D111_STANDBY_FAILED;
	}
	else
	{
		MAP_PRCMPeripheralReset(PRCM_CAMERA);
		MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
		g_bCameraOn = false;
	}

	return lRetVal;
}

//*****************************************************************************
//	Supplies clock to image sensor and wakes it up
//	Pair it with Standby_ImageSensor()
//*****************************************************************************
int32_t Wakeup_ImageSensor()
{
	int32_t lRetVal;

	DEBG_PRINT("\nCam Wake:\n");

	CamControllerInit();

	UtilsDelay(50);		//uTHRA

	//lRetVal = ExitStandby_mt9d111(HARD_STANDBY);	//Hard Standby is drawing more current
	lRetVal = ExitStandby_mt9d111(SOFT_STANDBY);
	RETURN_ON_ERROR(lRetVal);

	g_bCameraOn = true;

	return lRetVal;
}
