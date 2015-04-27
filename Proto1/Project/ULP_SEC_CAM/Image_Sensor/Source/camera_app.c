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

// Common interface includes
#include "udma_if.h"

#include "i2cconfig.h"
#include "pinmux.h"
#include "camera_app.h"
#include "mt9d111.h"

#include "app.h"

#include "network_related_fns.h"
#include "timer_fns.h"
#include "math.h"
//*****************************************************************************
// Macros
//*****************************************************************************

//#define USER_FILE_NAME          "www/images/testimage.jpg"
//#define USER_FILE_NAME          "www/123.txt"
#define TOTAL_DMA_ELEMENTS      64

//#define BUFFER_SIZE_IN_BYTES	71960	//70K
//#define BUFFER_SIZE_IN_BYTES	72192
//#define BUFFER_SIZE_IN_BYTES	25600
#define BLOCK_SIZE_IN_BYTES		4096	//4K
#define NUM_BLOCKS_IN_IMAGE_BUFFER			(BUFFER_SIZE_IN_BYTES / BLOCK_SIZE_IN_BYTES)
#define LAST_BLOCK_IN_BUFFER	(NUM_BLOCKS_IN_IMAGE_BUFFER-1)

#define DMA_TRANSFERS_TOFILL_BUFFER	( BUFFER_SIZE_IN_BYTES/(TOTAL_DMA_ELEMENTS*sizeof(unsigned long)))
#define DMA_TRANSFERS_TOFILL_BLOCK	( BLOCK_SIZE_IN_BYTES/(TOTAL_DMA_ELEMENTS*sizeof(unsigned long)))
//*****************************************************************************
//                      GLOBAL VARIABLES
//*****************************************************************************

unsigned long g_frame_size_in_bytes;

static unsigned long *p_buffer = NULL;
static unsigned char g_dma_txn_done;
volatile unsigned char g_frame_end;
static unsigned long g_total_dma_intrpts;

volatile long g_block_lastFilled; //Holds the block number that was filled latest by Cam
								 // can hold values -1(initialize) or between 0 and NUM_OB_BLOCKS_IN_IMAGE_BUFFER
volatile long g_position_in_block;

volatile long g_readHeader; //The block number that is to be read next and
							//stored in Flash

extern volatile unsigned char g_CaptureImage;

volatile uint8_t g_flag_blockFull[NUM_BLOCKS_IN_IMAGE_BUFFER];
						//Flag is set if the block has been written by camera,
						//but not stored in flash yet
volatile uint8_t g_flag_DataBlockFilled;
						//To denote that atleast one data block is filled


unsigned long  g_ulPingPacketsRecv = 0; //Number of Ping Packets received 

#ifdef ENABLE_JPEG
char g_header[SMTP_BUF_LEN] = {'\0'};
unsigned long g_header_length;


#endif 

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

unsigned char JPEG_StdQuantTblY[64] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68,  109, 103, 77,
    24,  35,  55,  64,  81,  104, 113, 92,
    49,  64,  78,  87, 103,  121, 120, 101,
    72,  92,  95,  98, 112,  100, 103,  99
};

unsigned char JPEG_StdQuantTblC[64] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};
//
// This table is used for regular-position to zigzagged-position lookup
//  This is Figure A.6 from the ISO/IEC 10918-1 1993 specification 
//
static unsigned char zigzag[64] =
{
    0, 1, 5, 6,14,15,27,28,
    2, 4, 7,13,16,26,29,42,
    3, 8,12,17,25,30,41,43,
    9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63
};

unsigned int JPEG_StdHuffmanTbl[384] =
{
    0x100, 0x101, 0x204, 0x30b, 0x41a, 0x678, 0x7f8, 0x9f6,
    0xf82, 0xf83, 0x30c, 0x41b, 0x679, 0x8f6, 0xaf6, 0xf84,
    0xf85, 0xf86, 0xf87, 0xf88, 0x41c, 0x7f9, 0x9f7, 0xbf4,
    0xf89, 0xf8a, 0xf8b, 0xf8c, 0xf8d, 0xf8e, 0x53a, 0x8f7,
    0xbf5, 0xf8f, 0xf90, 0xf91, 0xf92, 0xf93, 0xf94, 0xf95,
    0x53b, 0x9f8, 0xf96, 0xf97, 0xf98, 0xf99, 0xf9a, 0xf9b,
    0xf9c, 0xf9d, 0x67a, 0xaf7, 0xf9e, 0xf9f, 0xfa0, 0xfa1,
    0xfa2, 0xfa3, 0xfa4, 0xfa5, 0x67b, 0xbf6, 0xfa6, 0xfa7,
    0xfa8, 0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad, 0x7fa, 0xbf7,
    0xfae, 0xfaf, 0xfb0, 0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5,
    0x8f8, 0xec0, 0xfb6, 0xfb7, 0xfb8, 0xfb9, 0xfba, 0xfbb,
    0xfbc, 0xfbd, 0x8f9, 0xfbe, 0xfbf, 0xfc0, 0xfc1, 0xfc2,
    0xfc3, 0xfc4, 0xfc5, 0xfc6, 0x8fa, 0xfc7, 0xfc8, 0xfc9,
    0xfca, 0xfcb, 0xfcc, 0xfcd, 0xfce, 0xfcf, 0x9f9, 0xfd0,
    0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0xfd8,
    0x9fa, 0xfd9, 0xfda, 0xfdb, 0xfdc, 0xfdd, 0xfde, 0xfdf,
    0xfe0, 0xfe1, 0xaf8, 0xfe2, 0xfe3, 0xfe4, 0xfe5, 0xfe6,
    0xfe7, 0xfe8, 0xfe9, 0xfea, 0xfeb, 0xfec, 0xfed, 0xfee,
    0xfef, 0xff0, 0xff1, 0xff2, 0xff3, 0xff4, 0xff5, 0xff6,
    0xff7, 0xff8, 0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe,
    0x30a, 0xaf9, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
    0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7,
    0x101, 0x204, 0x30a, 0x418, 0x419, 0x538, 0x678, 0x8f4,
    0x9f6, 0xbf4, 0x30b, 0x539, 0x7f6, 0x8f5, 0xaf6, 0xbf5,
    0xf88, 0xf89, 0xf8a, 0xf8b, 0x41a, 0x7f7, 0x9f7, 0xbf6,
    0xec2, 0xf8c, 0xf8d, 0xf8e, 0xf8f, 0xf90, 0x41b, 0x7f8,
    0x9f8, 0xbf7, 0xf91, 0xf92, 0xf93, 0xf94, 0xf95, 0xf96,
    0x53a, 0x8f6, 0xf97, 0xf98, 0xf99, 0xf9a, 0xf9b, 0xf9c,
    0xf9d, 0xf9e, 0x53b, 0x9f9, 0xf9f, 0xfa0, 0xfa1, 0xfa2,
    0xfa3, 0xfa4, 0xfa5, 0xfa6, 0x679, 0xaf7, 0xfa7, 0xfa8,
    0xfa9, 0xfaa, 0xfab, 0xfac, 0xfad, 0xfae, 0x67a, 0xaf8,
    0xfaf, 0xfb0, 0xfb1, 0xfb2, 0xfb3, 0xfb4, 0xfb5, 0xfb6,
    0x7f9, 0xfb7, 0xfb8, 0xfb9, 0xfba, 0xfbb, 0xfbc, 0xfbd,
    0xfbe, 0xfbf, 0x8f7, 0xfc0, 0xfc1, 0xfc2, 0xfc3, 0xfc4,
    0xfc5, 0xfc6, 0xfc7, 0xfc8, 0x8f8, 0xfc9, 0xfca, 0xfcb,
    0xfcc, 0xfcd, 0xfce, 0xfcf, 0xfd0, 0xfd1, 0x8f9, 0xfd2,
    0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7, 0xfd8, 0xfd9, 0xfda,
    0x8fa, 0xfdb, 0xfdc, 0xfdd, 0xfde, 0xfdf, 0xfe0, 0xfe1,
    0xfe2, 0xfe3, 0xaf9, 0xfe4, 0xfe5, 0xfe6, 0xfe7, 0xfe8,
    0xfe9, 0xfea, 0xfeb, 0xfec, 0xde0, 0xfed, 0xfee, 0xfef,
    0xff0, 0xff1, 0xff2, 0xff3, 0xff4, 0xff5, 0xec3, 0xff6,
    0xff7, 0xff8, 0xff9, 0xffa, 0xffb, 0xffc, 0xffd, 0xffe,
    0x100, 0x9fa, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff,
    0xfd0, 0xfd1, 0xfd2, 0xfd3, 0xfd4, 0xfd5, 0xfd6, 0xfd7,
    0x100, 0x202, 0x203, 0x204, 0x205, 0x206, 0x30e, 0x41e,
    0x53e, 0x67e, 0x7fe, 0x8fe, 0xfff, 0xfff, 0xfff, 0xfff,
    0x100, 0x101, 0x102, 0x206, 0x30e, 0x41e, 0x53e, 0x67e,
    0x7fe, 0x8fe, 0x9fe, 0xafe, 0xfff, 0xfff, 0xfff, 0xfff
};
#endif 
/****************************************************************************/
/*                      LOCAL FUNCTION PROTOTYPES                           */
/****************************************************************************/
long InitCameraComponents();
//static void CamControllerInit();
void CamControllerInit();
static void CameraIntHandler();
static void DMAConfig();


#ifdef ENABLE_JPEG
static int CreateJpegHeader(char *header, int width, int height,
                            int format, int restart_int, int qscale);
static int DefineRestartIntervalMarker(char *pbuf, int ri);
static int DefineHuffmanTableMarkerAC
                               (char *pbuf, unsigned int *htable, int class_id);
static int DefineHuffmanTableMarkerDC
                               (char *pbuf, unsigned int *htable, int class_id);
static int DefineQuantizationTableMarker 
                                  (unsigned char *pbuf, int qscale, int format);
static int ScanHeaderMarker(char *pbuf, int format);
static int FrameHeaderMarker(char *pbuf, int width, int height, int format);
static int JfifApp0Marker(char *pbuf);
#endif 


//*****************************************************************************
//
//!     CaptureAndStore_Image
//!     Configures DMA and starts the Capture. Post Capture writes to SFLASH 
//!    
//!    \param                      None  
//!     \return                     0 - Success
//!                                   Negative - Failure
//!                               
//
//*****************************************************************************
long CaptureAndStore_Image()
{
    long lFileHandle;
    unsigned long ulToken = NULL;
    long lRetVal;
    unsigned char *p_header;

    uint32_t uiImageFile_Offset = 0;

    lRetVal = sl_Start(0, 0, 0);
   	ASSERT_ON_ERROR(lRetVal);

/*
	sl_FsDel((unsigned char *)USER_FILE_NAME, ulToken);
    //
    // Error handling if File Operation fails
    //
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    }
*/

    //
    // NVMEM File Open to write to SFLASH
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,//0x00212001,
//    					FS_MODE_OPEN_CREATE(65260,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
//    					FS_MODE_OPEN_CREATE(65,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
//    					FS_MODE_OPEN_CREATE(120535,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						FS_MODE_OPEN_CREATE(IMAGE_BUF_SIZE,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
    	UART_PRINT("File Open Error: %i", lRetVal);
    	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    }
    //
    // Close the created file
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    ASSERT_ON_ERROR(lRetVal);

/*
    SlFsFileInfo_t fileInfo;
    sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);
*/

    //
    // Open the file for Write Operation
    //
    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
                        FS_MODE_OPEN_WRITE,
                        &ulToken,
                        &lFileHandle);
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    }

    //
    // Create JPEG Header
    //
    memset(g_header, '\0', sizeof(g_header));
    g_header_length = CreateJpegHeader((char *)&g_header[0],
    									PIXELS_IN_X_AXIS,PIXELS_IN_Y_AXIS,
    									0, 0x0020, 9);
    InitializeTimer();
    StartTimer();
    //
    // Write the Header 
    //
    lRetVal = sl_FsWrite(lFileHandle, 0, g_header, g_header_length - 1);
    StopTimer();
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    }
    uiImageFile_Offset += lRetVal;
	UART_PRINT("Image Write No of bytes: %ld\n", lRetVal);

	float_t fHeaderWriteDuration;
	GetTimeDuration(&fHeaderWriteDuration);
	UART_PRINT("\n\rHeader Flash write Duration: %f\n\r", fHeaderWriteDuration);

	//
	// Initialize camera controller
	//
	CamControllerInit();

	//
	// Configure DMA in ping-pong mode
	//
	DMAConfig();

	memset(g_flag_blockFull, 0x00 ,NUM_BLOCKS_IN_IMAGE_BUFFER);
	g_readHeader = 0;
	g_flag_DataBlockFilled = 0;

	//
    // Perform Image Capture
    //
    UART_PRINT("sB");
    InitializeTimer();
    StartTimer();
    MAP_CameraCaptureStart(CAMERA_BASE);

    while((g_frame_end == 0))
    {
    	//UART_PRINT("B");
    	while( g_flag_DataBlockFilled )
    	{
    		if(g_flag_blockFull[g_readHeader])
    		{
    			UART_PRINT("s%d ", g_readHeader);

    			// Write a Block of Image from RAM to Flash
				lRetVal =  sl_FsWrite(lFileHandle, uiImageFile_Offset,
									  (unsigned char *)(g_image_buffer + g_readHeader*BLOCK_SIZE_IN_BYTES/4),
									  BLOCK_SIZE_IN_BYTES);
				if (lRetVal <0)
				{
					lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
					ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
				}

				// Update Num of bytes written into flash
				uiImageFile_Offset += lRetVal;

				// Indicate that Block is not full
				g_flag_blockFull[g_readHeader] = 0;

				// Change Block# to be read nexr
				g_readHeader++;
				g_readHeader %= NUM_BLOCKS_IN_IMAGE_BUFFER;
    		}
    		else
    		{
    			g_flag_DataBlockFilled = 0;
    			UART_PRINT("f");
    		}
    	}
		//UART_PRINT("F");
    }

    MAP_CameraCaptureStop(CAMERA_BASE, true);
    StopTimer();
    UART_PRINT("pA");

    float_t fPicCaptureDuration;
    GetTimeDuration(&fPicCaptureDuration);
    UART_PRINT("\n\rImage Capture Duration: %f\n\r", fPicCaptureDuration);

    UART_PRINT("\n\rDONE: Image Capture from Sensor\n\r");
    UART_PRINT("Image size: %ld\n", g_frame_size_in_bytes);

    //
    // Write the remaining Image data from RAM to Flash
    //
    lRetVal =  sl_FsWrite(lFileHandle, uiImageFile_Offset,
                      (unsigned char *)(g_image_buffer + g_readHeader*BLOCK_SIZE_IN_BYTES/4),
                      (g_frame_size_in_bytes % BLOCK_SIZE_IN_BYTES));
    if (lRetVal <0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    }
    uiImageFile_Offset += lRetVal;
    UART_PRINT("Image Write No of bytes: %ld\n", uiImageFile_Offset);

    // Close the file post writing the image
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    ASSERT_ON_ERROR(lRetVal);

/*
    sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);

    lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
    					FS_MODE_OPEN_READ,
    					&ulToken,
    					&lFileHandle);
	 if(lRetVal < 0)
	 {
		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	 }
	 lRetVal = sl_FsRead(lFileHandle, 0, (unsigned char*)g_image_buffer , 500);
	 if (lRetVal < 0)
	 {
		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	 }
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);
*/

    lRetVal = sl_Stop(0xFFFF);
    //lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	UART_PRINT("DONE: Image Write to Flash\n\r");

	//RegStatusRead();
	//AnalogGainReg_Read();

//	MAP_PRCMPeripheralReset(PRCM_CAMERA);
//    MAP_PRCMPeripheralClkDisable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
//
//    MAP_PRCMPeripheralReset(PRCM_UDMA);
//    MAP_PRCMPeripheralClkDisable(PRCM_UDMA,PRCM_RUN_MODE_CLK);


    return SUCCESS;
}
//*****************************************************************************
//
//!     DMA Config
//!     Initialize the DMA and Setup the DMA transfer
//!    
//!    \param                      None  
//!     \return                     None                  
//
//*****************************************************************************
static void DMAConfig()
{
    memset(g_image_buffer,0xF80F,sizeof(g_image_buffer));
    p_buffer = &g_image_buffer[0];

    //
    // Setup ping-pong transfer
    //
    DMASetupTransfer(UDMA_CH22_CAMERA,UDMA_MODE_PINGPONG,TOTAL_DMA_ELEMENTS,
                     UDMA_SIZE_32,
                     UDMA_ARB_8,(void *)CAM_BUFFER_ADDR, UDMA_SRC_INC_32,
                     (void *)p_buffer, UDMA_DST_INC_32);
    //
    //  Pong Buffer
    // 
    p_buffer += TOTAL_DMA_ELEMENTS;
    DMASetupTransfer(UDMA_CH22_CAMERA|UDMA_ALT_SELECT,UDMA_MODE_PINGPONG,
                     TOTAL_DMA_ELEMENTS,
                     UDMA_SIZE_32, UDMA_ARB_8,(void *)CAM_BUFFER_ADDR,
                     UDMA_SRC_INC_32, (void *)p_buffer, UDMA_DST_INC_32);
    //
    //  Ping Buffer
    // 
    p_buffer += TOTAL_DMA_ELEMENTS;

    g_dma_txn_done = 0;
    g_frame_size_in_bytes = 0;
    g_frame_end = 0;
    g_total_dma_intrpts = 0;

    g_block_lastFilled = -1;

    //
    // Clear any pending interrupt
    //
    HWREG(0x4402609C) |= 1 << 8;

    //
    // DMA Interrupt unmask from apps config
    //
    HWREG(0x44026094) |= 1 << 8;
}

//*****************************************************************************
//
//!     Camera Controller Initialisation
//!
//!    \param                      None
//!     \return                     None
//!
//
//*****************************************************************************
void CamControllerInit()
{
    MAP_PRCMPeripheralClkEnable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_CAMERA);

#ifndef ENABLE_JPEG
    // Configure Camera clock from ARCM
    // CamClkIn = ((240)/((1+1)+(1+1))) = 60 MHz
    HWREG(0x44025000) = 0x0101;
#else
    HWREG(0x44025000) = 0x0000;
#endif

    MAP_CameraReset(CAMERA_BASE);

#ifndef ENABLE_JPEG
    MAP_CameraParamsConfig(CAMERA_BASE, CAM_HS_POL_HI, CAM_VS_POL_HI,
                       CAM_ORDERCAM_SWAP|CAM_NOBT_SYNCHRO);
#else
    MAP_CameraParamsConfig(CAMERA_BASE, CAM_HS_POL_HI,CAM_VS_POL_HI,
                       CAM_NOBT_SYNCHRO|CAM_IF_SYNCHRO|CAM_BT_CORRECT_EN);
#endif

    MAP_CameraIntRegister(CAMERA_BASE, CameraIntHandler);

#ifndef ENABLE_JPEG
    MAP_CameraXClkConfig(CAMERA_BASE, 60000000,3750000);
#else
    //MAP_CameraXClkConfig(CAMERA_BASE, 120000000,24000000);
    MAP_CameraXClkConfig(CAMERA_BASE, 120000000, 8000000);
//    MAP_CameraXClkConfig(CAMERA_BASE, 120000000, 6000000);
#endif

    MAP_CameraThresholdSet(CAMERA_BASE, 8);
    MAP_CameraIntEnable(CAMERA_BASE, CAM_INT_FE);
    MAP_CameraDMAEnable(CAMERA_BASE);
}


//*****************************************************************************
//
//!     Camera Interrupt Handler
//!    
//!    \param                      None  
//!     \return                     None                              
//
//*****************************************************************************
static void CameraIntHandler()
{
	//UART_PRINT("!");
    if(g_total_dma_intrpts > 1 && MAP_CameraIntStatus(CAMERA_BASE) & CAM_INT_FE)
    {
        MAP_CameraIntClear(CAMERA_BASE, CAM_INT_FE);
        g_frame_end = 1;
        MAP_CameraCaptureStop(CAMERA_BASE, true);
        //UART_PRINT("++++++");
    }

    if(HWREG(0x440260A4) & (1<<8))
    {
    	//UART_PRINT(".");
    	// Camera DMA Done clear
        HWREG(0x4402609C) |= 1 << 8;

        g_total_dma_intrpts++;

        g_frame_size_in_bytes += (TOTAL_DMA_ELEMENTS*sizeof(unsigned long));
        if(g_frame_size_in_bytes < FRAME_SIZE_IN_BYTES && 
                                    g_frame_size_in_bytes < IMAGE_BUF_SIZE)
        {
            if(g_dma_txn_done == 0)
            {
            	DMASetupTransfer(UDMA_CH22_CAMERA,UDMA_MODE_PINGPONG,
                                 TOTAL_DMA_ELEMENTS,UDMA_SIZE_32,
                                 UDMA_ARB_8,(void *)CAM_BUFFER_ADDR, 
                                 UDMA_SRC_INC_32,
                                 (void *)p_buffer, UDMA_DST_INC_32);
            	p_buffer += TOTAL_DMA_ELEMENTS;
            	g_dma_txn_done = 1;
                //UART_PRINT("I");
            }
            else
            {
            	DMASetupTransfer(UDMA_CH22_CAMERA|UDMA_ALT_SELECT,
                                 UDMA_MODE_PINGPONG,TOTAL_DMA_ELEMENTS,
                                 UDMA_SIZE_32, UDMA_ARB_8,
                                 (void *)CAM_BUFFER_ADDR,
                                 UDMA_SRC_INC_32, (void *)p_buffer,
                                 UDMA_DST_INC_32);
                p_buffer += TOTAL_DMA_ELEMENTS;
                g_dma_txn_done = 0;
                //UART_PRINT("G");
            }

        	g_position_in_block++;

        	//
        	// If buffer end is reached, change write pointer to point top of buffer
        	//
        	if( g_position_in_block == (DMA_TRANSFERS_TOFILL_BLOCK - 2) )
        	{
        		if(g_block_lastFilled == (LAST_BLOCK_IN_BUFFER - 1 ))
				{
					//UART_PRINT("FllBuff:%d ",(p_buffer - g_image_buffer));
					p_buffer = g_image_buffer;
				}
        	}

        	//
        	// See if end of block is reached
        	//
            if( g_position_in_block == DMA_TRANSFERS_TOFILL_BLOCK )
            {
            	// reset counter
            	g_position_in_block = 0;

            	// Update block#
            	g_block_lastFilled++;
            	UART_PRINT("%d ", g_block_lastFilled);

            	// Set Block full flag
            	g_flag_blockFull[g_block_lastFilled] = 1;
            	g_flag_DataBlockFilled = 1;

            	// If the last block is filled, reset block# to -1
            	if( g_block_lastFilled == LAST_BLOCK_IN_BUFFER )
            	{
            		g_block_lastFilled = -1;
            	}
            }
      }
        else
        {
            // Disable DMA 
            MAP_UtilsDelay(20000);
            MAP_uDMAChannelDisable(UDMA_CH22_CAMERA);
            HWREG(0x44026090) |= 1 << 8;
            g_frame_end = 1;
            UART_PRINT(",,,,,");
        }
    }
}

//*****************************************************************************
//
//!     JfifApp0Marker 
//!    
//!    \param                      Pointer to the output buffer  
//!     \return                     Length of the Marker or error code                        
//
//*****************************************************************************

#ifdef ENABLE_JPEG
static int JfifApp0Marker(char *pbuf)
{
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    *pbuf++= 0xFF;                  // APP0 marker 
    *pbuf++= 0xE0;
    *pbuf++= 0x00;                  // length 
    *pbuf++= 0x10;
    *pbuf++= 0x4A;                  // JFIF identifier 
    *pbuf++= 0x46;
    *pbuf++= 0x49;
    *pbuf++= 0x46;
    *pbuf++= 0x00;
    *pbuf++= 0x01;                  // version 
    *pbuf++= 0x02;
    *pbuf++= 0x00;                  // units 
    *pbuf++= 0x00;                  // X density 
    *pbuf++= 0x01;
    *pbuf++= 0x00;                  // Y density 
    *pbuf++= 0x01;
    *pbuf++= 0x00;                  // X thumbnail 
    *pbuf++= 0x00;                  // Y thumbnail 
    return 18;
}


//*****************************************************************************
//
//!    FrameHeaderMarker
//!    
//!    \param1                      pointer to the output buffer  
//!    \param2                      width   
//!    \param3                      height 
//!    \param4                      format
//!
//!     \return                       Length of the header marker or error code                      
//
//*****************************************************************************
static int FrameHeaderMarker(char *pbuf, int width, int height, int format)
{
    int length;
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer");
        LOOP_FOREVER();
    }
    if (format == FORMAT_MONOCHROME)
        length = 11;
    else
        length = 17;

    *pbuf++= 0xFF;                      // start of frame: baseline DCT 
    *pbuf++= 0xC0;
    *pbuf++= length>>8;                 // length field 
    *pbuf++= length&0xFF;
    *pbuf++= 0x08;                      // sample precision 
    *pbuf++= height>>8;                 // number of lines 
    *pbuf++= height&0xFF;
    *pbuf++= width>>8;                  // number of samples per line 
    *pbuf++= width&0xFF;

    if (format == FORMAT_MONOCHROME)    // monochrome 
    {
        *pbuf++= 0x01;              // number of image components in frame 
        *pbuf++= 0x00;              // component identifier: Y 
        *pbuf++= 0x11;              // horizontal | vertical sampling factor: Y 
        *pbuf++= 0x00;              // quantization table selector: Y 
    }
    else if (format == FORMAT_YCBCR422) // YCbCr422
    {
        *pbuf++= 0x03;        // number of image components in frame 
        *pbuf++= 0x00;        // component identifier: Y 
        *pbuf++= 0x21;        // horizontal | vertical sampling factor: Y 
        *pbuf++= 0x00;        // quantization table selector: Y 
        *pbuf++= 0x01;        // component identifier: Cb 
        *pbuf++= 0x11;        // horizontal | vertical sampling factor: Cb 
        *pbuf++= 0x01;        // quantization table selector: Cb 
        *pbuf++= 0x02;        // component identifier: Cr 
        *pbuf++= 0x11;        // horizontal | vertical sampling factor: Cr 
        *pbuf++= 0x01;        // quantization table selector: Cr 
    }
    else                                // YCbCr420 
    {
        *pbuf++= 0x03;         // number of image components in frame 
        *pbuf++= 0x00;         // component identifier: Y 
        *pbuf++= 0x22;         // horizontal | vertical sampling factor: Y 
        *pbuf++= 0x00;         // quantization table selector: Y 
        *pbuf++= 0x01;         // component identifier: Cb 
        *pbuf++= 0x11;         // horizontal | vertical sampling factor: Cb 
        *pbuf++= 0x01;         // quantization table selector: Cb 
        *pbuf++= 0x02;         // component identifier: Cr 
        *pbuf++= 0x11;         // horizontal | vertical sampling factor: Cr 
        *pbuf++= 0x01;        // quantization table selector: Cr 
    }

    return (length+2);
}


//*****************************************************************************
//
//!     ScanHeaderMarker
//!
//!    \param1                     pointer to output buffer  
//!    \param2                     Format 
//!
//!     \return                     Length or error code                             
//
//*****************************************************************************
static int ScanHeaderMarker(char *pbuf, int format)
{
    int length;
    
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    if (format == FORMAT_MONOCHROME)
        length = 8;
    else
        length = 12;

    *pbuf++= 0xFF;                  // start of scan 
    *pbuf++= 0xDA;
    *pbuf++= length>>8;             // length field 
    *pbuf++= length&0xFF;
    if (format == FORMAT_MONOCHROME)// monochrome 
    {
        *pbuf++= 0x01;              // number of image components in scan 
        *pbuf++= 0x00;              // scan component selector: Y 
        *pbuf++= 0x00;              // DC | AC huffman table selector: Y 
    }
    else                            // YCbCr
    {
        *pbuf++= 0x03;              // number of image components in scan 
        *pbuf++= 0x00;              // scan component selector: Y 
        *pbuf++= 0x00;              // DC | AC huffman table selector: Y 
        *pbuf++= 0x01;              // scan component selector: Cb 
        *pbuf++= 0x11;              // DC | AC huffman table selector: Cb 
        *pbuf++= 0x02;              // scan component selector: Cr 
        *pbuf++= 0x11;              // DC | AC huffman table selector: Cr 
    }

    *pbuf++= 0x00;         // Ss: start of predictor selector 
    *pbuf++= 0x3F;         // Se: end of spectral selector 
    *pbuf++= 0x00;         // Ah | Al: successive approximation bit position 

    return (length+2);
}


//*****************************************************************************
//
//!     DefineQuantizationTableMarker 
//!      Calculate and write the quantisation tables
//! qscale is the customised scaling factor  see MT9D131 developer guide page 78 
//!    
//!    \param1                      pointer to the output buffer  
//!    \param2                      Quantization Scale  
//!    \param3                      Format 
//!
//!     \return                      Length of the Marker or error code                      
//
//*****************************************************************************
static int DefineQuantizationTableMarker (unsigned char *pbuf, int qscale, int format)
{
    int i, length, temp;
    // temp array to store scaled zigzagged quant entries 
    unsigned char newtbl[64];

    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer");
        LOOP_FOREVER();
    }

    if (format == FORMAT_MONOCHROME)    // monochrome 
        length  =  67;
    else
        length  =  132;

    *pbuf++  =  0xFF;                   // define quantization table marker 
    *pbuf++  =  0xDB;
    *pbuf++  =  length>>8;              // length field 
    *pbuf++  =  length&0xFF;
     // quantization table precision | identifier for luminance 
    *pbuf++  =  0;                     

    // calculate scaled zigzagged luminance quantisation table entries 
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblY[i] * qscale + 16) / 32;
        // limit the values to the valid range 
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting luminance quant table to the output buffer 
    for (i=0; i<64; i++)
        *pbuf++ = newtbl[i];

    // if format is monochrome we're finished, 
    // otherwise continue on, to do chrominance quant table 
    if (format == FORMAT_MONOCHROME)
        return (length+2);

    *pbuf++ = 1;   // quantization table precision | identifier for chrominance 

    // calculate scaled zigzagged chrominance quantisation table entries 
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblC[i] * qscale + 16) / 32;
        // limit the values to the valid range 
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting chrominance quant table to the output buffer 
    for (i=0; i<64; i++)
        *pbuf++ = newtbl[i];

    return (length+2);
}


//*****************************************************************************
//
//!     DefineHuffmanTableMarkerDC 
//!    
//!    \param1                      pointer to Marker buffer  
//!    \param2                      Huffman table  
//!    \param3                      Class Identifier 
//!  
//!     \return                      Length of the marker or error code                            
//
//*****************************************************************************
static int DefineHuffmanTableMarkerDC(char *pbuf, unsigned int *htable, 
                                                                int class_id)
{
    int i, l, count;
    int length;
    char *plength;

    if((pbuf == NULL) || (htable == NULL))
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    *pbuf++= 0xFF;                  // define huffman table marker 
    *pbuf++= 0xC4;
    plength = pbuf;                 // place holder for length field 
    *pbuf++;
    *pbuf++;
    *pbuf++= class_id;              // huffman table class | identifier 

    for (l = 0; l < 16; l++)
    {
        count = 0;
        for (i = 0; i < 12; i++)
        {
            if ((htable[i] >> 8) == l)
                count++;
        }
        *pbuf++= count;             // number of huffman codes of length l+1 
    }

    length = 19;
    for (l = 0; l < 16; l++)
    {
        for (i = 0; i < 12; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                *pbuf++= i;         // HUFFVAL with huffman codes of length l+1 
                length++;
            }
        }
    }

    *plength++= length>>8;          // length field 
    *plength = length&0xFF;

    return (length + 2);
}


//*****************************************************************************
//
//!     DefineHuffmanTableMarkerAC 
//!     1. Establishes connection w/ AP//
//!     2. Initializes the camera sub-components//! GPIO Enable & Configuration
//!     3. Listens and processes the image capture requests from user-applications
//!    
//!    \param1                      pointer to Marker buffer  
//!    \param2                      Huffman table  
//!    \param3                      Class Identifier 
//!
//!     \return                      Length of the Marker or error code
//!                               
//
//*****************************************************************************
static int DefineHuffmanTableMarkerAC(char *pbuf, unsigned int *htable, 
                                                                int class_id)
{
    int i, l, a, b, count;
    char *plength;
    int length;

    if((pbuf == NULL) || (htable == NULL))
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    *pbuf++= 0xFF;                      // define huffman table marker 
    *pbuf++= 0xC4;
    plength = pbuf;                     // place holder for length field 
    *pbuf++;
    *pbuf++;
    *pbuf++= class_id;                  // huffman table class | identifier 

    for (l = 0; l < 16; l++)
    {
        count = 0;
        for (i = 0; i < 162; i++)
        {
            if ((htable[i] >> 8) == l)
                count++;
        }

        *pbuf++= count;          // number of huffman codes of length l+1 
    }

    length = 19;
    for (l = 0; l < 16; l++)
    {
        // check EOB: 0|0 
        if ((htable[160] >> 8) == l)
        {
            *pbuf++= 0;         // HUFFVAL with huffman codes of length l+1 
            length++;
        }

        // check HUFFVAL: 0|1 to E|A 
        for (i = 0; i < 150; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                a = i/10;
                b = i%10;
                // HUFFVAL with huffman codes of length l+1
                *pbuf++= (a<<4)|(b+1);   
                length++;
            }
        }

        // check ZRL: F|0 
        if ((htable[161] >> 8) == l)
        {
        // HUFFVAL with huffman codes of length l+1 
            *pbuf++= 0xF0;              
            length++;
        }

        // check HUFFVAL: F|1 to F|A 
        for (i = 150; i < 160; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                a = i/10;
                b = i%10;
                 // HUFFVAL with huffman codes of length l+1 
                *pbuf++= (a<<4)|(b+1); 
                length++;
            }
        }
    }

    *plength++= length>>8;              // length field 
    *plength = length&0xFF;
    return (length + 2);
}


//*****************************************************************************
//
//!     DefineRestartIntervalMarker
//!    
//!    \param1                      pointer to Marker buffer  
//!    \param2                      return interval
//!
//!     \return                      Length or error code                                
//
//*****************************************************************************
static int DefineRestartIntervalMarker(char *pbuf, int ri)
{
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    *pbuf++= 0xFF;                  // define restart interval marker 
    *pbuf++= 0xDD;
    *pbuf++= 0x00;                  // length 
    *pbuf++= 0x04;
    *pbuf++= ri >> 8;               // restart interval 
    *pbuf++= ri & 0xFF;
    return 6;
}
//*****************************************************************************
//
//!     CreateJpegHeader
//!     Create JPEG Header in JFIF format
//!    
//!    \param1                     header - pointer to JPEG header buffer  
//!    \param2                     width - image width 
//!    \param3                     height - image height  
//!    \param4                     format - color format 
//!                                 (0 = YCbCr422, 1 = YCbCr420, 2 = monochrome)  
//!    \param5                     restart_int - restart marker interval  
//!    \param6                     qscale - quantization table scaling factor
//!
//!     \return               length of JPEG header (bytes) or error code -1                             
//
//*****************************************************************************

static int CreateJpegHeader(char *header, int width, int height,
                            int format, int restart_int, int qscale)
{
    char *pbuf = header;
    int length;
    if(header == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    // SOI 
    *pbuf++= 0xFF;
    *pbuf++= 0xD8;
    length = 2;

    // JFIF APP0 
    length += JfifApp0Marker(pbuf);

    // Quantization Tables 
    pbuf = header + length;
    length += DefineQuantizationTableMarker
                                       ((unsigned char *)pbuf, qscale, format);

    // Frame Header 
    pbuf = header + length;
    length += FrameHeaderMarker(pbuf, width, height, format);

    // Huffman Table DC 0 for Luma 
    pbuf = header + length;
    length += DefineHuffmanTableMarkerDC(pbuf, &JPEG_StdHuffmanTbl[352], 0x00);

    // Huffman Table AC 0 for Luma 
    pbuf = header + length;
    length += DefineHuffmanTableMarkerAC(pbuf, &JPEG_StdHuffmanTbl[0], 0x10);

    if (format != FORMAT_MONOCHROME)// YCbCr
    {
        // Huffman Table DC 1 for Chroma 
        pbuf = header + length;
        length += DefineHuffmanTableMarkerDC
                                        (pbuf, &JPEG_StdHuffmanTbl[368], 0x01);

        // Huffman Table AC 1 for Chroma 
        pbuf = header + length;
        length += DefineHuffmanTableMarkerAC
                                        (pbuf, &JPEG_StdHuffmanTbl[176], 0x11);
    }

    // Restart Interval 
    if (restart_int > 0)
    {
        pbuf = header + length;
        length += DefineRestartIntervalMarker(pbuf, restart_int);
    }

    // Scan Header 
    pbuf = header + length;
    length += ScanHeaderMarker(pbuf, format);

    return length;
}
#endif 

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
