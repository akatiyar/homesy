/*
 * Camera_capture_preview_image.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "camera_app.h"
#include "flash_files.h"

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

#include "simplelink.h"

int32_t CaptureandSavePreviewImage()
{
	SlFsFileInfo_t fileInfo;
	long lFileHandle;
	unsigned long ulToken = 0;
	long lRetVal;

	uint8_t* databuf = (uint8_t*)g_image_buffer;
	sl_FsGetInfo((_u8*)JPEG_HEADER_FILE_NAME, ulToken, &fileInfo);
	lRetVal = ReadFile_FromFlash((uint8_t*)g_image_buffer, JPEG_HEADER_FILE_NAME, fileInfo.FileLen, 0);

	databuf[159] = 0x01;
	databuf[160] = 0xE0;
	databuf[161] = 0x02;
	databuf[162] = 0x80;
    //
    // Open the file for Write Operation
    //
    lRetVal = sl_FsOpen((unsigned char *)JPEG_PREVIEW_IMAGE_FILE_NAME,
                        FS_MODE_OPEN_WRITE,
                        &ulToken,
                        &lFileHandle);
    //
    // Error handling if File Operation fails
    //
    if(lRetVal < 0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        RETURN_ON_ERROR(FILE_OPEN_WRITE_FAILED);
    }

    lRetVal = sl_FsWrite(lFileHandle, 0,(uint8_t*) g_image_buffer,  fileInfo.FileLen+1);

	memset((void*)g_image_buffer, 0x00, PREVIEW_IMAGE_MAXSZ);

	ImagCapture_Init();
    //
    // Perform Image Capture
    //
    MAP_CameraCaptureStart(CAMERA_BASE);

    while((g_frame_end == 0) && (g_frame_size_in_bytes < PREVIEW_IMAGE_MAXSZ));

    MAP_CameraCaptureStop(CAMERA_BASE, true);

    //
    // Write the Image Buffer
    //
    lRetVal =  sl_FsWrite(lFileHandle,  fileInfo.FileLen+1,
                      (unsigned char *)g_image_buffer, g_frame_size_in_bytes);
    //
    // Error handling if file operation fails
    //
    if (lRetVal <0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        RETURN_ON_ERROR(FILE_WRITE_FAILED);
    }
    //
    // Close the file post writing the image
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    RETURN_ON_ERROR(lRetVal);

    return SUCCESS;
}
