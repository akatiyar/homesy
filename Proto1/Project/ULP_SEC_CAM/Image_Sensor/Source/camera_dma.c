/*
 * camera_dma.c
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

static unsigned long *p_buffer = NULL;
static unsigned char g_dma_txn_done;
static unsigned long g_total_dma_intrpts;

/****************************************************************************/
/*                      LOCAL FUNCTION PROTOTYPES                           */
/****************************************************************************/
void CamControllerInit();
static void CameraIntHandler();

//*****************************************************************************
//
//!     DMA Config
//!     Initialize the DMA and Setup the DMA transfer
//!
//!    \param                      None
//!     \return                     None
//
//*****************************************************************************
void DMAConfig()
{
    p_buffer = &g_image_buffer[0];

    //
    // Setup ping-pong transfer
    //
    UDMASetupTransfer(UDMA_CH22_CAMERA,UDMA_MODE_PINGPONG,TOTAL_DMA_ELEMENTS,
                     UDMA_SIZE_32,
                     UDMA_ARB_8,(void *)CAM_BUFFER_ADDR,
                     //UDMA_SRC_INC_32,
                     UDMA_SRC_INC_NONE,
                     (void *)p_buffer, UDMA_DST_INC_32);
    //
    //  Pong Buffer
    //
    p_buffer += TOTAL_DMA_ELEMENTS;
    UDMASetupTransfer(UDMA_CH22_CAMERA|UDMA_ALT_SELECT,UDMA_MODE_PINGPONG,
                     TOTAL_DMA_ELEMENTS,
                     UDMA_SIZE_32, UDMA_ARB_8,(void *)CAM_BUFFER_ADDR,
                     //UDMA_SRC_INC_32,
                     UDMA_SRC_INC_NONE,
                     (void *)p_buffer, UDMA_DST_INC_32);
    //
    //  Ping Buffer
    //
    p_buffer += TOTAL_DMA_ELEMENTS;

    g_dma_txn_done = 0;
    g_frame_size_in_bytes = 0;
    g_frame_end = 0;
    g_total_dma_intrpts = 0;
    g_collect_timestamp = 0;

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
    //MAP_CameraXClkConfig(CAMERA_BASE, 120000000, 6000000);
#endif

    MAP_CameraThresholdSet(CAMERA_BASE, 8);
    //MAP_CameraThresholdSet(CAMERA_BASE, 64);
    MAP_CameraIntEnable(CAMERA_BASE, CAM_INT_FE | CAM_INT_DMA);
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
	//DEBG_PRINT("!");

    if(g_total_dma_intrpts > 1 && CameraIntStatus(CAMERA_BASE) & CAM_INT_FE)
    {
    	//DEBG_PRINT("?");
        MAP_CameraIntClear(CAMERA_BASE, CAM_INT_FE);
        g_frame_end = 1;
        g_collect_timestamp = 1;
        //DEBG_PRINT("col1a\n");
        MAP_CameraCaptureStop(CAMERA_BASE, true);
        //DEBG_PRINT("++++++");
        //JPEGDataLength_read();
    }
    //else if(HWREG(0x440260A0) & (1<<8))
    //if(HWREG(0x440260A4) & (1<<8))
    if(CameraIntStatus(CAMERA_BASE)& CAM_INT_DMA)
    {
    	//DEBG_PRINT(".");
    	// Camera DMA Done clear
    	CameraIntClear(CAMERA_BASE,CAM_INT_DMA);	//HWREG(0x4402609C) |= 1 << 8;

        g_total_dma_intrpts++;

        g_frame_size_in_bytes += (TOTAL_DMA_ELEMENTS*sizeof(unsigned long));
        if(g_frame_size_in_bytes < FRAME_SIZE_IN_BYTES &&
        		(g_frame_size_in_bytes < (JPEG_IMAGE_MAX_FILESIZE-2*(TOTAL_DMA_ELEMENTS*sizeof(unsigned long)))))
        {
            if(g_dma_txn_done == 0)
            {
//            	if(g_image_buffer[10]==0x0FF80FF8)
//            		while(1);
            	UDMASetupTransfer(UDMA_CH22_CAMERA,UDMA_MODE_PINGPONG,
                                 TOTAL_DMA_ELEMENTS,UDMA_SIZE_32,
                                 UDMA_ARB_8,(void *)CAM_BUFFER_ADDR,
                                 //UDMA_SRC_INC_32,
                                 UDMA_SRC_INC_NONE,
                                 (void *)p_buffer, UDMA_DST_INC_32);
            	p_buffer += TOTAL_DMA_ELEMENTS;
            	g_dma_txn_done = 1;
                //DEBG_PRINT("I");
            }
            else
            {
            	UDMASetupTransfer(UDMA_CH22_CAMERA|UDMA_ALT_SELECT,
                                 UDMA_MODE_PINGPONG,TOTAL_DMA_ELEMENTS,
                                 UDMA_SIZE_32, UDMA_ARB_8,
                                 (void *)CAM_BUFFER_ADDR,
                                 //UDMA_SRC_INC_32,
                                 UDMA_SRC_INC_NONE,
                                 (void *)p_buffer,
                                 UDMA_DST_INC_32);
                p_buffer += TOTAL_DMA_ELEMENTS;
                g_dma_txn_done = 0;
                //DEBG_PRINT("G");
            }

            /*//
            //	For Block Size = 256Bytes. Overwrite happening
            //
            // Update block#
			g_block_lastFilled++;
			DEBG_PRINT("%d ", g_block_lastFilled);

			// Set Block full flag
			g_flag_blockFull[g_block_lastFilled] = 1;
			g_flag_DataBlockFilled = 1;

        	// If the last block is filled, reset block# to -1
        	if( g_block_lastFilled == LAST_BLOCK_IN_BUFFER )
        	{
        		g_block_lastFilled = -1;
        	}

        	// If buffer end is reached, change write pointer to point top of buffer
        	if(g_block_lastFilled == (LAST_BLOCK_IN_BUFFER - 2 ))
			{
				//DEBG_PRINT("FllBuff:%d ",(p_buffer - g_image_buffer));
				p_buffer = g_image_buffer;
			}*/

        	//
			//	For Block Size = 4MBytes
			//
        	g_position_in_block++;

        	//
        	// If buffer end is reached, change write pointer to point top of buffer
        	//
        	if( g_position_in_block == (DMA_TRANSFERS_TOFILL_BLOCK - 2) )
        	{
        		if(g_block_lastFilled == (LAST_BLOCK_IN_BUFFER - 1 ))
				{
					//DEBG_PRINT("FllBuff:%d ",(p_buffer - g_image_buffer));
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
            	//DEBG_PRINT("%d ", g_block_lastFilled);

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
        	DEBG_PRINT("?");
            // Disable DMA
            MAP_UtilsDelay(20000);
            MAP_uDMAChannelDisable(UDMA_CH22_CAMERA);
            CameraIntDisable(CAMERA_BASE,CAM_INT_DMA);             //HWREG(0x44026090) |= 1 << 8;
            g_frame_end = 1;
            g_collect_timestamp = 1;
            //DEBG_PRINT("col1b\n");
            //DEBG_PRINT(",,,,, %x , %x", HWREG(0x440260A0),MAP_CameraIntStatus(CAMERA_BASE));
        }
    }
}
