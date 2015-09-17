//*****************************************************************************
// camera_app.h
//
// camera application macro & API's prototypes
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

#ifndef __CAMERA_APP_H__
#define __CAMERA_APP_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "app.h"

#define UART_COMMAND_IMG_CAPTURE		('x')
#define LOWER_TO_UPPER_CASE				(32)

#define DISABLE							(0)
#define ENABLE							(1)
#define SL_VERSION_LENGTH				(11)

#define IMAGE_BUF_SIZE_BYTES			81920	//80K //Image buffer
//#define IMAGE_BUF_SIZE_BYTES			61440	//60K //Image buffer
//#define IMAGE_BUF_SIZE_BYTES			40960	//40K //Image buffer

#define PREVIEW_IMAGE_MAXSZ				40960	//40K

#define ONE_KB                      	(1024)

#ifdef ENABLE_JPEG
    // Capture is stopped at 'CAM_INT_FE' anyway 
    #ifdef	HD_FRAME
		#define PIXELS_IN_X_AXIS        (1280)
        #define PIXELS_IN_Y_AXIS        (720)
		//#define PIXELS_IN_X_AXIS        (640)	//DEBUG
        //#define PIXELS_IN_Y_AXIS        (480)	//

		//#define NUM_OF_1KB_BUFFERS      150
		//#define NUM_OF_1KB_BUFFERS      120
		//#define MAX_IMAGE_SIZE_BYTES	(150*ONE_KB)		//Expected Val

#elif XGA_FRAME
        #define PIXELS_IN_X_AXIS        (1024)
        #define PIXELS_IN_Y_AXIS        (768)

        #define NUM_OF_1KB_BUFFERS      120
    #elif VGA_FRAME
        #define PIXELS_IN_X_AXIS        (640)
        #define PIXELS_IN_Y_AXIS        (480)

        #define NUM_OF_1KB_BUFFERS      80
    #elif QVGA_FRAME
        #define PIXELS_IN_X_AXIS        (240)
        #define PIXELS_IN_Y_AXIS        (320)

        #define NUM_OF_1KB_BUFFERS      50
    #endif
#else   
    #ifdef QVGA_FRAME
        #define PIXELS_IN_X_AXIS        (240)
        #define PIXELS_IN_Y_AXIS        (256)

        #define NUM_OF_1KB_BUFFERS      120
    #endif
#endif  

#define BYTES_PER_PIXEL             	(2)       // RGB 565
#define FRAME_SIZE_IN_BYTES         \
(PIXELS_IN_X_AXIS * PIXELS_IN_Y_AXIS * BYTES_PER_PIXEL)

#define NUM_OF_4B_CHUNKS            	((IMAGE_BUF_SIZE_BYTES)/(sizeof(unsigned long)))
#define NUM_OF_1KB_CHUNKS           	(IMAGE_BUF_SIZE_BYTES/ONE_KB)
#define NUM_OF_4B_CHUNKS_IN_1KB     	(ONE_KB/(sizeof(unsigned long)))


#define MAX_EMAIL_ID_LENGTH         	34
#define SMTP_BUF_LEN                	1024

// Defines for g_Image buffer utilization

#define PREVIEWIMAGE_MARGINBUFFER_SZ		5120
#define SENSOR_CONFIGS_OFFSET_BUF			PREVIEWIMAGE_MARGINBUFFER_SZ + PREVIEW_IMAGE_MAXSZ
#define SENSOR_CONFIGS_SZ					8192
#define USER_CONFIGS_OFFSET_BUF				SENSOR_CONFIGS_OFFSET_BUF + SENSOR_CONFIGS_SZ
#define USER_CONFIGS_SZ						8192

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif


#define CAM_BT_CORRECT_EN   0x00001000

typedef enum opcd{
    START_CAPTURE = 1,
    STOP_CAPTURE,
    IMG_FMT,
    IMG_SIZE,
    EXIT
}e_opcode;

typedef struct cmd_struct{
    long    opcode;
    char    email_id[MAX_EMAIL_ID_LENGTH];
}s_cmd_struct;


//******************************************************************************
// Externs
//******************************************************************************
//extern unsigned long g_image_buffer[NUM_OF_4B_CHUNKS];
extern unsigned long g_image_buffer[(IMAGE_BUF_SIZE_BYTES/sizeof(unsigned long))];
//******************************************************************************
// APIs
//******************************************************************************
long CaptureAndStore_Image();
long CaptureinRAM_StoreAfterCapture_Image();
long CaptureinRAM();
void CamControllerInit();
void StartCamera();
int32_t CaptureandSavePreviewImage();
int32_t Restart_Camera();
int32_t Standby_ImageSensor();
int32_t Wakeup_ImageSensor();
int32_t createAndWrite_ImageHeaderFile();
int32_t create_JpegImageFile();
int32_t Write_JPEGHeader();
int32_t Config_And_Start_CameraCapture();
int32_t Start_CameraCapture();
int32_t Config_CameraCapture();
int32_t ReStart_CameraCapture(uint16_t* pImageConfig);

int32_t toggle_standby();//Tag:Remove later

void ImagCapture_Init();
int32_t CaptureImage(int32_t lFileHandle);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CAMERA_APP_H__ */

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
