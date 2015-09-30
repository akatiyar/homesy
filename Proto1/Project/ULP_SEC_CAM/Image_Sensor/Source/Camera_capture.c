/*
 * camera_capture.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "app.h"
#include "camera_app.h"

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

#include "flash_files.h"
#include "simplelink.h"
#include "timer_fns.h"
#include "udma_if.h"

//******************************************************************************
//	This function
//		1. makes camera parallel interface to capture one incoming image
//		2. stores it to flash parallelly
//******************************************************************************
int32_t CaptureImage(int32_t lFileHandle)
{
    long lRetVal;
    struct u64_time time_now;
    uint32_t uiImageFile_Offset;

    // Initial values set
    uiImageFile_Offset = 0;
	//g_image_buffer[0] = 0xFFFFFFFF;

    //Command to camera peripheral to capture next incoming image
    MAP_CameraCaptureStart(CAMERA_BASE);
   // HWREG(0x4402609C) |= 1 << 8;

    //Workaround to eliminate the invalid first block bug
    while(1)
    {
    	if(g_flag_blockFull[0])
    	{
    		if((0 == g_image_buffer[0])&&(0 == g_image_buffer[10])&&(0 == g_image_buffer[20]))	//Checking three random positions in the buffer
    		//if(0xFFFFFFFF == g_image_buffer[0])
    		{
				DEBG_PRINT("INV 1st blk\n");
    			//DEBG_PRINT("s%d ", g_readHeader);

				// Write a Block of Image from RAM to Flash
				lRetVal =  sl_FsWrite(lFileHandle, uiImageFile_Offset,
									  (unsigned char *)(g_image_buffer + CAM_DMA_BLOCK_SIZE_IN_BYTES/4),
									  (BLOCK_SIZE_IN_BYTES - CAM_DMA_BLOCK_SIZE_IN_BYTES));
				if (lRetVal < 0)
				{
					lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
					RETURN_ON_ERROR(FILE_WRITE_FAILED);
				}

				// Update Num of bytes written into flash
				uiImageFile_Offset += lRetVal;

				// Indicate that Block is not full
				g_flag_blockFull[g_readHeader] = 0;

				// Change Block# to be read next
				g_readHeader++;
			}
    		break;
    	}
    	if(captureTimeout_Flag)
    	{
    		return MT9D111_IMAGE_CAPTURE_FAILED;
    	}
    }

    while((g_frame_end == 0))
    {
    	//DEBG_PRINT("B");
    	while( g_flag_DataBlockFilled )
    	{
    		//Collect photo click timestamp
    		if(g_collect_timestamp == 1)
        	{
    			//DEBG_PRINT("col2\n");
    			cc_rtc_get(&time_now);
    			g_TimeStamp_PhotoSnap = time_now.secs * 1000 + time_now.nsec / 1000000;
        		g_collect_timestamp = 0;
        	}

    		if(g_flag_blockFull[g_readHeader])
    		{
    			//DEBG_PRINT("s%d ", g_readHeader);

    			// Write a Block of Image from RAM to Flash
				lRetVal =  sl_FsWrite(lFileHandle, uiImageFile_Offset,
									  (unsigned char *)(g_image_buffer + g_readHeader*BLOCK_SIZE_IN_BYTES/4),
									  BLOCK_SIZE_IN_BYTES);
				if (lRetVal < 0)
				{
					lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
					RETURN_ON_ERROR(FILE_WRITE_FAILED);
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
    			//DEBG_PRINT("f");
    		}
    	}
		//DEBG_PRINT("F");
    }

    //When the buffer is not fully filled
    DEBG_PRINT("Image captured from Sensor\n");
    DEBG_PRINT("g_frame_size_in_bytes: %ld\n", g_frame_size_in_bytes+(TOTAL_DMA_ELEMENTS*sizeof(unsigned long)));

    //
    // Write the remaining Image data from RAM to Flash
    //
    lRetVal =  sl_FsWrite(lFileHandle, uiImageFile_Offset,
                      (unsigned char *)(g_image_buffer + g_readHeader*BLOCK_SIZE_IN_BYTES/4),
                      ((g_frame_size_in_bytes % BLOCK_SIZE_IN_BYTES) + (TOTAL_DMA_ELEMENTS*sizeof(unsigned long))));
    if (lRetVal <0)
    {
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        RETURN_ON_ERROR(FILE_WRITE_FAILED);
    }
    uiImageFile_Offset += lRetVal;

    DEBG_PRINT("Image written to Flash\n");
    RELEASE_PRINT("Image size: %ld\n", uiImageFile_Offset);

    //DEBG_PRINT("Image Write No of bytes: %ld\n", uiImageFile_Offset);
    //SlFsFileInfo_t FileInfo;
    //uint32_t ulToken = NULL;
    //sl_FsGetInfo((uint8_t*)JPEG_IMAGE_FILE_NAME,ulToken,&FileInfo);
	//DEBG_PRINT("Size of Image in Flash:%ld\n",FileInfo.FileLen);

    return lRetVal;
}
