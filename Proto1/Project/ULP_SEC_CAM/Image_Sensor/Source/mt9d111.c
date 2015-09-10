//*****************************************************************************
//  MT9D111.c
//
// Micron MT9D111 camera sensor driver
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
//! \addtogroup mt9d111
//! @{
//
//*****************************************************************************
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "mt9d111.h"
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "i2c.h"
#include "i2cconfig.h"
#include "common.h"

#include "app.h"

#include "flash_files.h"

#include "osi.h"

#define RET_OK                  0
#define RET_ERROR               -1
#define SENSOR_PAGE_REG         0xF0
#define CAM_I2C_SLAVE_ADDR      ((0xBA >> 1))
//#define CAM_I2C_SLAVE_ADDR      ((0x90 >> 1))
#define MARGIN_NUMCLKS			10
#define CHIP_VERSION			(0x1519)

#define MT9D111_CLKIN_MIN		6000000

typedef struct MT9D111RegLst
{
    unsigned char ucPageAddr;
    unsigned char ucRegAddr;
    unsigned short usValue;
} s_RegList;

#define SIZE_REGLIST		sizeof(struct MT9D111RegLst)
#define OFFSET_PAGE_ADDR	0
#define OFFSET_REG_ADDR		sizeof(s_RegList.ucPageAddr)
#define OFFSET_VALUE		(sizeof(s_RegList.ucPageAddr) + sizeof(s_RegList.ucRegAddr))

#ifndef ENABLE_JPEG
static const s_RegList preview_on_cmd_list[]= {
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0001    },  // SEQ_CMD, Do Preview
    {1, 0xC6, 0xA104    },  // SEQ_CMD
    {111, 0xC8, 0x0003  },  // SEQ_CMD, Do Preview
    {1, 0xC6, 0xA103    },  // SEQ_CMD-refresh
    {1, 0xC8, 0x0005    },  // SEQ_CMD-refresh
    {1, 0xC6, 0xA103    },  // SEQ_CMD-refresh
    {1, 0xC8, 0x0006    },  // SEQ_CMD-refresh
    {1, 0xC6, 0xA104    },  // SEQ_CMD
    {111, 0xC8, 0x0003  },  // SEQ_CMD, Do Preview
    {100, 0x00, 0x01E0  },  // Delay = 500ms
};

static  const s_RegList freq_setup_cmd_List[]= {
    {1, 0xC6, 0x276D    },  // MODE_FIFO_CONF1_A
    {1, 0xC8, 0xE4E2    },  // MODE_FIFO_CONF1_A
    {1, 0xC6, 0xA76F    },  // MODE_FIFO_CONF2_A
    {1, 0xC8, 0x00E8    },  // MODE_FIFO_CONF2_A
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD (Refresh)
   // Set maximum integration time to get a minimum of 15 fps at 45MHz
    {1, 0xC6, 0xA20E    },  // AE_MAX_INDEX
    {1, 0xC8, 0x0004},      // AE_MAX_INDEX
    {1, 0xC6, 0xA102    },  // SEQ_MODE
    {1, 0xC8, 0x0001    },  // SEQ_MODE
    {1, 0xC6, 0xA102    },  // SEQ_MODE
    {1, 0xC8, 0x0005    },  // SEQ_MODE
   // Set minimum integration time to get a maximum of 15 fps at 45MHz
    {1, 0xC6, 0xA20D    },  // AE_MAX_INDEX
    {1, 0xC8, 0x0004    },  // AE_MAX_INDEX
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD (Refresh)
};

static  const s_RegList image_size_240_320_preview_cmds_list[]=
{
    {0, 0x07, 0x00FE    },  // HORZ_BLANK_A
    {0, 0x08, 0x02A0    },  // VERT_BLANK_A
    {0, 0x20, 0x0303    },  // READ_MODE_B (Image flip settings)
    {0, 0x21, 0x8400    },  // READ_MODE_A (1ADC)
    {1, 0xC6, 0x2703    },  // MODE_OUTPUT_WIDTH_A
    {1, 0xC8, 0x00F0    },  // MODE_OUTPUT_WIDTH_A
    {1, 0xC6, 0x2705    },  // MODE_OUTPUT_HEIGHT_A
    {1, 0xC8, 0x0140    },  // MODE_OUTPUT_HEIGHT_A
    {1, 0xC6, 0x2727    },  // MODE_CROP_X0_A
    {1, 0xC8, 0x0000    },  // MODE_CROP_X0_A
    {1, 0xC6, 0x2729    },  // MODE_CROP_X1_A
    {1, 0xC8, 0x00F0    },  // MODE_CROP_X1_A
    {1, 0xC6, 0x272B    },  // MODE_CROP_Y0_A
    {1, 0xC8, 0x0000    },  // MODE_CROP_Y0_A
    {1, 0xC6, 0x272D    },  // MODE_CROP_Y1_A
    {1, 0xC8, 0x0140    },  // MODE_CROP_Y1_A
    {1, 0xC6, 0x270F    },  // MODE_SENSOR_ROW_START_A
    {1, 0xC8, 0x001C    },  // MODE_SENSOR_ROW_START_A
    {1, 0xC6, 0x2711    },  // MODE_SENSOR_COL_START_A
    {1, 0xC8, 0x003C    },  // MODE_SENSOR_COL_START_A
    {1, 0xC6, 0x2713    },  // MODE_SENSOR_ROW_HEIGHT_A
    {1, 0xC8, 0x0280    },  // MODE_SENSOR_ROW_HEIGHT_A
    {1, 0xC6, 0x2715    },  // MODE_SENSOR_COL_WIDTH_A
    {1, 0xC8, 0x03C0    },  // MODE_SENSOR_COL_WIDTH_A
    {1, 0xC6, 0x2717    },  // MODE_SENSOR_X_DELAY_A
    {1, 0xC8, 0x0088    },  // MODE_SENSOR_X_DELAY_A
    {1, 0xC6, 0x2719    },  // MODE_SENSOR_ROW_SPEED_A
    {1, 0xC8, 0x0011    },  // MODE_SENSOR_ROW_SPEED_A
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    },  // SEQ_CMD
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0006    },  // SEQ_CMD
};

static  const s_RegList preview_cmds_list[]= {

    {1, 0xC6, 0xA77D    },  // MODE_OUTPUT_FORMAT_A
    {1, 0xC8, 0x0020    },  // MODE_OUTPUT_FORMAT_A; RGB565
    {1, 0xC6, 0x270B    },  // MODE_CONFIG
    {1, 0xC8, 0x0030    },  // MODE_CONFIG, JPEG disabled for A and B
    {1, 0xC6, 0xA103    },  // SEQ_CMD
    {1, 0xC8, 0x0005    }   // SEQ_CMD, refresh
};
#endif
static  const s_RegList init_cmds_list[]= {
    {100,0x00,0x01F4    },
    {0, 0x33, 0x0343    }, // RESERVED_CORE_33
	{0, 0x0D, 0x0280    },
    {1, 0xC6, 0xA115    }, // SEQ_LLMODE
    {1, 0xC8, 0x0020    }, // SEQ_LLMODE
    {0, 0x38, 0x0866    }, // RESERVED_CORE_38
    //{1, 0x98, 0x0002	},	//	added Y suppression -CS
    //{1, 0x98, 0x0007	},	//	added Y, Cr, Cb suppression -CS
    //{0x02, 0x0D, 0x0267 },	//Enable status byte insertion - CS
    {2, 0x80, 0x0168    }, // LENS_CORRECTION_CONTROL
    {2, 0x81, 0x6432    }, // ZONE_BOUNDS_X1_X2
    {2, 0x82, 0x3296    }, // ZONE_BOUNDS_X0_X3
    {2, 0x83, 0x9664    }, // ZONE_BOUNDS_X4_X5
    {2, 0x84, 0x5028    }, // ZONE_BOUNDS_Y1_Y2
    {2, 0x85, 0x2878    }, // ZONE_BOUNDS_Y0_Y3
    {2, 0x86, 0x7850    }, // ZONE_BOUNDS_Y4_Y5
    {2, 0x87, 0x0000    }, // CENTER_OFFSET
    {2, 0x88, 0x0152    }, // FX_RED
    {2, 0x89, 0x015C    }, // FX_GREEN
    {2, 0x8A, 0x00F4    }, // FX_BLUE
    {2, 0x8B, 0x0108    }, // FY_RED
    {2, 0x8C, 0x00FA    }, // FY_GREEN
    {2, 0x8D, 0x00CF    }, // FY_BLUE
    {2, 0x8E, 0x09AD    }, // DF_DX_RED
    {2, 0x8F, 0x091E    }, // DF_DX_GREEN
    {2, 0x90, 0x0B3F    }, // DF_DX_BLUE
    {2, 0x91, 0x0C85    }, // DF_DY_RED
    {2, 0x92, 0x0CFF    }, // DF_DY_GREEN
    {2, 0x93, 0x0D86    }, // DF_DY_BLUE
    {2, 0x94, 0x163A    }, // SECOND_DERIV_ZONE_0_RED
    {2, 0x95, 0x0E47    }, // SECOND_DERIV_ZONE_0_GREEN
    {2, 0x96, 0x103C    }, // SECOND_DERIV_ZONE_0_BLUE
    {2, 0x97, 0x1D35    }, // SECOND_DERIV_ZONE_1_RED
    {2, 0x98, 0x173E    }, // SECOND_DERIV_ZONE_1_GREEN
    {2, 0x99, 0x1119    }, // SECOND_DERIV_ZONE_1_BLUE
    {2, 0x9A, 0x1663    }, // SECOND_DERIV_ZONE_2_RED
    {2, 0x9B, 0x1569    }, // SECOND_DERIV_ZONE_2_GREEN
    {2, 0x9C, 0x104C    }, // SECOND_DERIV_ZONE_2_BLUE
    {2, 0x9D, 0x1015    }, // SECOND_DERIV_ZONE_3_RED
    {2, 0x9E, 0x1010    }, // SECOND_DERIV_ZONE_3_GREEN
    {2, 0x9F, 0x0B0A    }, // SECOND_DERIV_ZONE_3_BLUE
    {2, 0xA0, 0x0D53    }, // SECOND_DERIV_ZONE_4_RED
    {2, 0xA1, 0x0D51    }, // SECOND_DERIV_ZONE_4_GREEN
    {2, 0xA2, 0x0A44    }, // SECOND_DERIV_ZONE_4_BLUE
    {2, 0xA3, 0x1545    }, // SECOND_DERIV_ZONE_5_RED
    {2, 0xA4, 0x1643    }, // SECOND_DERIV_ZONE_5_GREEN
    {2, 0xA5, 0x1231    }, // SECOND_DERIV_ZONE_5_BLUE
    {2, 0xA6, 0x0047    }, // SECOND_DERIV_ZONE_6_RED
    {2, 0xA7, 0x035C    }, // SECOND_DERIV_ZONE_6_GREEN
    {2, 0xA8, 0xFE30    }, // SECOND_DERIV_ZONE_6_BLUE
    {2, 0xA9, 0x4625    }, // SECOND_DERIV_ZONE_7_RED
    {2, 0xAA, 0x47F3    }, // SECOND_DERIV_ZONE_7_GREEN
    {2, 0xAB, 0x5859    }, // SECOND_DERIV_ZONE_7_BLUE
    {2, 0xAC, 0x0000    }, // X2_FACTORS
    {2, 0xAD, 0x0000    }, // GLOBAL_OFFSET_FXY_FUNCTION
    {2, 0xAE, 0x0000    }, // K_FACTOR_IN_K_FX_FY
    {1, 0x08, 0x01FC    }, // COLOR_PIPELINE_CONTROL
    {1, 0xC6, 0x2003    }, // MON_ARG1
    {1, 0xC8, 0x0748    }, // MON_ARG1
    {1, 0xC6, 0xA002    }, // MON_CMD
    {1, 0xC8, 0x0001    }, // MON_CMD
    {111, 0xC8,0x0000 },
    {1, 0xC6, 0xA361    }, // AWB_TG_MIN0
    {1, 0xC8, 0x00E2    }, // AWB_TG_MIN0
    {1, 0x1F, 0x0018    }, // RESERVED_SOC1_1F
    {1, 0x51, 0x7F40    }, // RESERVED_SOC1_51
    {0, 0x33, 0x0343    }, // RESERVED_CORE_33
    {0, 0x38, 0x0868    }, // RESERVED_CORE_38
    {1, 0xC6, 0xA10F    }, // SEQ_RESET_LEVEL_TH
    {1, 0xC8, 0x0042    }, // SEQ_RESET_LEVEL_TH
    {1, 0x1F, 0x0020    }, // RESERVED_SOC1_1F
    {1, 0xC6, 0xAB04    }, // HG_MAX_DLEVEL
    {1, 0xC8, 0x0008    }, // HG_MAX_DLEVEL
    {1, 0xC6, 0xA103    }, // SEQ_CMD
    {1, 0xC8, 0x0005    }, // SEQ_CMD
    {1, 0xC6, 0xA104    }, // SEQ_CMD
    {111, 0xC8,0x0003   },
    {1, 0x08, 0x01FC    }, // COLOR_PIPELINE_CONTROL
    {1, 0x08, 0x01EC    }, // COLOR_PIPELINE_CONTROL
    {1, 0x08, 0x01FC    }, // COLOR_PIPELINE_CONTROL
    //{1, 0x08, 0x01BC    }, // Invert the pixel clock
    {1, 0x36, 0x0F08    }, // APERTURE_PARAMETERS
    //{1, 0x48, 0x0003	},	//Test pattern -CS
    {1, 0xC6, 0xA103    }, // SEQ_CMD
    {1, 0xC8, 0x0005    }, // SEQ_CMD
};

#ifdef ENABLE_JPEG
static  const s_RegList capture_cmds_list[]= {
    {0, 0x65, 0xA000    },  // Bypass PLL
    {0, 0x65, 0xE000    },  // Power DOWN PLL
    {100, 0x00, 0x01F4  },  // Delay =500ms
//    {0,  0x66,  0x1E03  },	////{0,  0x66,  0x1E01  },
//    {0,  0x67,  0x0501  },
	{0,  0x66,  0x2401  },		//M = 36, N = 1
	{0,  0x67,  0x0501  },		//P=1
    //{0,  0x66,  0x1E01  },	//N = 1, M = 30
    //{0,  0x67,  0x0506  },	//P = 7		// Changed to 9 -Uthra
    {0, 0x65,   0xA000  },  // Power up PLL
    {0,  0x65,  0x2000  },  // Enable PLL
    {0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
	//{0, 0x20, 0x0400    },  // READ_MODE_B // Single ADC for low power. Not tested though. Should % shutter delay by 2 to maintain integration time
    {100, 0x00, 0x01F4  },  // Delay =500ms
    {100, 0x00, 0x01F4  },  // Delay =500ms
    {100, 0x00, 0x01F4  },  // Delay =500ms
    {1, 0xC6, 0xA102    },  // SEQ_MODE
    //{1, 0xC8, 0x0001    },  // SEQ_MODE
    {1, 0xC8, 0x0000    },  // SEQ_MODE Will turn off AE, AWB
    // Commenting off to not turn on auto white balance
    //{1, 0xC6, 0xA102    },  // SEQ_MODE
    //{1, 0xC8, 0x0005    },  // SEQ_MODE
    {1,  0xC6, 0xA120   },  // Enable Capture video
    {1,  0xC8, 0x0002   },

  /*  //Added by Uthra
    {1, 0xC6, 0x276B }, // Enable Capture video
    {1, 0xC8, 0x0067 },
    {1, 0xC6, 0x276D }, // Enable Capture video
    {1, 0xC8, 0x0408 },

    {1, 0xC6, 0x2772 }, // Enable Capture video
    {1, 0xC8, 0x0067 },
    {1, 0xC6, 0x2774 }, // Enable Capture video
    {1, 0xC8, 0x0408 },
    // {1, 0xC6, 0xA776 }, // Enable Capture video
    // {1, 0xC8, 0x0003 },*/

    {1, 0xC6, 0x2772 },	//Enable status byte insertion, Enable adaptive clocking
	//{1, 0xC8, 0x0267 },
	{1, 0xC8, 0x0227 },	//Disable adaptive clocking
	{1, 0xC6, 0x2774 }, // PCLk 1 and 2 setting
	{1, 0xC8, 0x02F8 },	// PCLK1 = 15
	{1, 0xC6, 0x2776 }, // Set Pclk 3
	{1, 0xC8, 0x0001 },

    {1,  0xC6, 0x270B   },  // Mode config, disable JPEG bypass
    {1,  0xC8, 0x0010   },	//Enable Jpeg only for Mode B
    {1,  0xC6, 0x2702   },  // FIFO_config0b, no spoof, adaptive clock
    {1,  0xC8, 0x001E   },
    {1,  0xC6, 0xA907   },  // JPEG mode config, video
    {1,  0xC8, 0x0011   },
    //{1,  0xC8, 0x0015   },	// Auto QScale selection disabled - CS
    //{1,  0xC8, 0x0013   },	// Enable handshake with CC3200, disable retry after failure - CS
    //{1,  0xC8, 0x0012   },	// Enable handshake with CC3200, disable retry after failure, snapshot - CS

    {1,  0xC6, 0xA906   },  // Format YCbCr422
    {1,  0xC8, 0x0000   },
    {1,  0xC6, 0xA90A   },  // Set the qscale1
    //{1,  0xC8, 0x0089   },	// SDK val: 9
    {1,  0xC8, (0x0080|IMAGE_QUANTIZ_SCALE)   },
    //{1,  0xC8, 0x0094   },	// 20
    //{1,  0xC8, 0x00C0   },	// 64
    //{1,  0xC8, 0x00A0   },	// 32
    {1,  0xC6, 0x2908   },  // Set the restartInt
    {1,  0xC8, 0x0006   },
	//{0x02, 0x0D, 0x0027 },	// Disable adaptive clocking
	//{0x02, 0x0E, 0x0305 },	// Set Pclk divisor
    //{0x02, 0x0D, 0x0067 },
    //{0x02, 0x0D, 0x0267 },		//Enable status byte insertion

    {100, 0x00, 0x01F4  },  // Delay =500ms
    {1, 0xC6, 0x2707    },  // MODE_OUTPUT_WIDTH_B
#ifdef HD_FRAME
    {1, 0xC8, 640      },
    //{1, 0xC8, 640       },	//Debug
#elif XGA_FRAME
    {1, 0xC8, 1024      },
#elif VGA_FRAME
    {1, 0xC8, 640       },
#elif QVGA_FRAME
    {1, 0xC8, 240       },
#endif
    {1, 0xC6, 0x2709    },  // MODE_OUTPUT_HEIGHT_B
#ifdef HD_FRAME
    {1, 0xC8, 480       },
    //{1, 0xC8, 480       },	//Debug
#elif XGA_FRAME
    {1, 0xC8, 768       },
#elif VGA_FRAME
    {1, 0xC8, 480       },
#elif QVGA_FRAME
    {1, 0xC8, 320       },
#endif

	// Added by Uthra
    {1, 0xC6, 0x2122 },{1, 0xC8, 0 },
	{1, 0xC6, 0x2124 },{1, 0xC8, 0 },
	{1, 0xC6, 0x2126 },{1, 0xC8, 0 },
	{1, 0xC6, 0x2129 },{1, 0xC8, 0 },
	{1, 0xC6, 0x212A },{1, 0xC8, 0 },
	{1, 0xC6, 0x212B },{1, 0xC8, 0 },
	{1, 0xC6, 0x212D },{1, 0xC8, 0 },
	{1, 0xC6, 0x2130 },{1, 0xC8, 0 },
	{1, 0xC6, 0x2132 },{1, 0xC8, 0 },
	{1, 0xC6, 0x2134 },{1, 0xC8, 0 },

    {1, 0xC6, 0x276B },	//Enable status byte insertion, Enable adaptive clocking
	{1, 0xC8, 0x0227 },	//Disable adaptive clocking
	{1, 0xC6, 0x276D }, // Enable Capture video
	{1, 0xC8, 0x02FF },
	{1, 0xC6, 0x276F }, // Enable Capture video
	{1, 0xC8, 0x0001 },

    {1, 0xC6, 0x2703    },  // MODE_OUTPUT_WIDTH_A
    {1, 0xC8, 120      },
    {1, 0xC6, 0x2705    },  // MODE_OUTPUT_HEIGHT_A
    {1, 0xC8, 120       },

    {1, 0xC6, 0x2727    },  // MODE_CROP_X0_A
    {1, 0xC8, 0x0000    },
    {1, 0xC6, 0x2729    },  // MODE_CROP_X1_A
    {1, 0xC8, 120  },
	{1, 0xC6, 0x2731    },  // MODE_CROP_Y0_A
    {1, 0xC8, 0x0000    },
    {1, 0xC6, 0x2733    },  // MODE_CROP_Y1_A
    {1, 0xC8, 120      },
	/******************************************/
    {1, 0xC6, 0x2735    },  // MODE_CROP_X0_B
    {1, 0xC8, 0x0000    },
    {1, 0xC6, 0x2737    },  // MODE_CROP_X1_B
    {1, 0xC8, 1600  },
    //{1, 0xC8, 1280  },
    {1, 0xC6, 0x2739    },  // MODE_CROP_Y0_B
    {1, 0xC8, 0x0000    },
    {1, 0xC6, 0x273B    },  // MODE_CROP_Y1_B
    {1, 0xC8, 1200      },
    //{1, 0xC8, 820      },
/*
// Tag:CS to solve the bent images after stanby issue
	{1, 0xC6, 0xA361    }, // AWB_TG_MIN0
	{1, 0xC8, 0x00E2    }, // AWB_TG_MIN0
	{1, 0xC6, 0xA10F    }, // SEQ_RESET_LEVEL_TH
	{1, 0xC8, 0x0042    }, // SEQ_RESET_LEVEL_TH
	{1, 0xC6, 0xAB04    }, // HG_MAX_DLEVEL
	{1, 0xC8, 0x0008    }, // HG_MAX_DLEVEL
	{1, 0xC6, 0xA103    }, // SEQ_CMD
	{1, 0xC8, 0x0005    }, // SEQ_CMD
	{1, 0xC6, 0xA104    }, // SEQ_CMD
	{111, 0xC8,0x0003   },
*/
	 {1, 0xC6, 0x2725    },{1, 0xC8, 0x0000    },  // Clock Settings R0x0A
	 {1, 0xC6, 0xA117    },{1, 0xC8, 0x005f    },        // Enable low lighting in the driver
    {1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture	//Moving this part after maual time and exposure settings
    {1, 0xC8, 0x0002    },
	{1, 0xC6, 0xA104    },  // wait till become capture
	{111, 0xC8, 0x0007  }
    //{100, 0x00, 0x01F4  },  // Delay =500ms
};

//static  const s_RegList recapture_cmds_list[]= {
//
////	{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 },  // SEQ_MODE Will turn off AE, AWB
////    {0,  0x65,  0x2000  },  // Enable PLL
////    {0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
////	{100, 0x00, 0x0064  },  // Delay =100ms
////    {1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture	//Moving this part after maual time and exposure settings
////    {1, 0xC8, 0x0002    },
////    {1, 0xC6, 0xA104    },  // wait till become capture
////    {111, 0xC8, 0x0007   }
//
//	{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 },  // SEQ_MODE Will turn off AE, AWB
//	{0,  0x65,  0x2000  },  // Enable PLL
//	{0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
//	{100, 0x00, 0x0064  },  // Delay =100ms
//
//	{0x00, 0x2B, ((0x0024<<1)|0x0080)},        //Lines exist. tfss-9203b65b-f7ea-42b8-b9e8-be5366de68bf-myPic1.jpg
//	{0x00, 0x2C, ((0x003A<<1)|0x0080)},
//	{0x00, 0x2D, ((0x0024<<1)|0x0080)},
//	{0x00, 0x2E, ((0x0024<<1)|0x0080)},
//
//	//{0x00, 0x09, (0x005F)},         //Integration time = 5mS
////	{0x00, 0x09, (200)},         //Integration time = 10mS
//	{0x00, 0x09, (270)},         //Integration time = 15mS
//	{0x00, 0x0C, 0x0000},
//
//	{0x01, 0x6E, 0x0085},
//	{0x01, 0x6A, 0x008D},
//	{0x01, 0x6B, 0x008D},
//	{0x01, 0x6C, 0x0093},
//	{0x01, 0x6D, 0x008d},
//	{0x01, 0x4E, 0x0020},
//
//
//
//	{0x01, 0x60, 0x291A},
//	{0x01, 0x61, 0x04E4},
//	{0x01, 0x62, 0xbab5},
//	{0x01, 0x63, 0xb001},
//	{0x01, 0x64, 0x4089},
//	{0x01, 0x65, 0xf17c},
//	//        {0x01, 0x48, 0x0101},
//	{0x00, 0x05, 0x015c},	//Hblank
//	{0x00, 0x06, 0x0020},	//Vblank
//
//
//	{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture        //Moving this part after maual time and exposure settings
//	{1, 0xC8, 0x0002    },
//	{1, 0xC6, 0xA104    },  // wait till become capture
//	{111, 0xC8, 0x0007  }
//};

//static  const s_RegList snap[]= {
//
//										{0, 0x0D, 0x0282    },
//
////										{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture        //Moving this part after maual time and exposure settings
////										{1, 0xC8, 0x0002    },
////
////										{1, 0xC6, 0xA104    },  // wait till become capture
////										{111, 0xC8, 0x0007  }
//};


#endif
//*****************************************************************************
// Static Function Declarations
//*****************************************************************************
static long RegLstWrite(s_RegList *pRegLst, unsigned long ulNofItems);
//static long RegLstWrite_fromFlash(s_RegList *pRegLst, unsigned long ulNofItems);
extern void MT9D111Delay(unsigned long ucDelay);
static long Register_Read(s_RegList *pRegLst, uint16_t* pusRegVal);

//*****************************************************************************
//
//! This function initilizes the camera sensor
//!
//! \param                      None
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
long CameraSensorInit()
{
    long lRetVal = -1;

//    unsigned char ucChipRevAddr = 0x00;
//    unsigned char ucBuffer[2];
//    while(1)
//    {
//    	lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR, &ucData, 1, 1);
//    }
//    lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,&ucChipRevAddr,1,1);
//    I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,I2C_SEND_STOP);
    lRetVal = RegLstWrite((s_RegList *)init_cmds_list, \
                                    sizeof(init_cmds_list)/sizeof(s_RegList));
//    ASSERT_ON_ERROR(lRetVal);

#ifndef ENABLE_JPEG
    lRetVal = RegLstWrite((s_RegList *)preview_cmds_list,
                      sizeof(preview_cmds_list)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)image_size_240_320_preview_cmds_list, \
                    sizeof(image_size_240_320_preview_cmds_list)/ \
                    sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)freq_setup_cmd_List,
                    sizeof(freq_setup_cmd_List)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)preview_on_cmd_list,
                    sizeof(preview_on_cmd_list)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
#endif 
    return lRetVal;
}

#ifdef FLASH_FILES
//Read ImageSensorConfig File from SFlash and initialise the camera
long CameraSensorInit_SettingsFromFlash()
{
	long lRetVal = -1;
	uint8_t ucFileContent[CONTENT_LENGTH_SENSORS_CONFIGS];
	uint8_t* puc;

	lRetVal = ReadFile_FromFlash(ucFileContent,
									FILENAME_SENSORCONFIGS,
									CONTENT_LENGTH_SENSORS_CONFIGS,
									OFFSET_MT9D111);

	puc = ucFileContent + 2;
	lRetVal = RegLstWrite((s_RegList*)puc, ucFileContent[0]);

	return lRetVal;
}

//Read ImageSensorConfig File from SFlash and start jpeg capture
long StartCapture_SettingsFromFlash()
{
	long lRetVal = -1;
	uint8_t ucFileContent[CONTENT_LENGTH_SENSORS_CONFIGS];
	uint8_t* puc;

	lRetVal = ReadFile_FromFlash(ucFileContent,
									FILENAME_SENSORCONFIGS,
									CONTENT_LENGTH_SENSORS_CONFIGS,
									OFFSET_MT9D111);

	puc = ucFileContent + 2 + (ucFileContent[0] * sizeof(s_RegList));
	lRetVal = RegLstWrite((s_RegList*)puc, ucFileContent[1]);

	return lRetVal;
}

//DBG. Write the hardcoded Image Sensor settings into Flash.
//Not likely to be used otherwise
long WriteConfigRegFile_toFlash()
{
	long lRetVal = -1;
	int32_t lFileHandle;
	uint8_t ucHeader[2];
	uint16_t offset = OFFSET_MT9D111;

	ucHeader[0] = sizeof(init_cmds_list)/sizeof(s_RegList);
	ucHeader[1] = sizeof(capture_cmds_list)/sizeof(s_RegList);

	lRetVal = CreateFile_Flash(FILENAME_SENSORCONFIGS, MAX_FILESIZE_SENSORCONFIGS);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = WriteFile_ToFlash(ucHeader,
								FILENAME_SENSORCONFIGS,
								sizeof(ucHeader),
								offset,
								MULTIPLE_WRITE_FIRST,
								&lFileHandle);
	ASSERT_ON_ERROR(lRetVal);
	offset += lRetVal;

	lRetVal = WriteFile_ToFlash((uint8_t*)&init_cmds_list[0],
									FILENAME_SENSORCONFIGS,
									sizeof(init_cmds_list),
									offset,
									MULTIPLE_WRITE_MIDDLE,
									&lFileHandle);
	ASSERT_ON_ERROR(lRetVal);
	offset += lRetVal;

	lRetVal = WriteFile_ToFlash((uint8_t*)&capture_cmds_list[0],
										FILENAME_SENSORCONFIGS,
										sizeof(capture_cmds_list),
										offset,
										MULTIPLE_WRITE_LAST,
										&lFileHandle);
	ASSERT_ON_ERROR(lRetVal);
	offset += lRetVal;

	return offset;
}

//Use this fn to write ImageSensConfig file sent by user OTA into Flash
long WriteConfigRegFilefromUser_toFlash(uint8_t* pFileContent,
										uint32_t uiDataSize)
{
	long lRetVal = -1;
	int32_t lFileHandle;

	lRetVal = WriteFile_ToFlash(pFileContent,
								FILENAME_SENSORCONFIGS,
								uiDataSize,
								OFFSET_MT9D111,
								SINGLE_WRITE,
								&lFileHandle);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}
#endif


//*****************************************************************************
//
//! This function configures the sensor in JPEG mode
//!
//! \param                      None
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
long StartSensorInJpegMode()
{
#ifdef ENABLE_JPEG
    long lRetVal = -1;

    lRetVal = RegLstWrite((s_RegList *)capture_cmds_list,
                        sizeof(capture_cmds_list)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
#endif
    return 0;
}

long RestartSensorInJpegMode(uint16_t *ImageConfig)
{
    long lRetVal = -1;

    s_RegList recapture_cmds_list[]= {

    	{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 },  // SEQ_MODE Will turn off AE, AWB
    	{0, 0x65,  0x2000  },  // Enable PLL
    	{0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
    	{100, 0x00, 0x0064  },  // Delay =100ms

		{0x00, 0x2B, (ImageConfig[0])},
		{0x00, 0x2C, (ImageConfig[1])},
		{0x00, 0x2D, ImageConfig[2]},
		{0x00, 0x2E, ImageConfig[3]},
		{0x00, 0x09, ImageConfig[4]},
		{0x00, 0x0C, ImageConfig[5]},
		{0x01, 0x4E, ImageConfig[6]},
		{0x01, 0x60, ImageConfig[7]},
		{0x01, 0x61, ImageConfig[8]},
		{0x01, 0x62, ImageConfig[9]},
		{0x01, 0x63, ImageConfig[10]},
		{0x01, 0x64, ImageConfig[11]},
		{0x01, 0x65, ImageConfig[12]},
		{0x01, 0x6E, ImageConfig[13]},
		{0x01, 0x6A, ImageConfig[14]},
		{0x01, 0x6B, ImageConfig[15]},
		{0x01, 0x6C, ImageConfig[16]},
		{0x01, 0x6D, ImageConfig[17]},

		{0x01, 0x30, ImageConfig[18]},
		{0x01, 0x31, ImageConfig[19]},
		{0x01, 0x32, ImageConfig[20]},
		{0x01, 0x35, ImageConfig[21]},
		{0x01, 0x36, ImageConfig[22]},
		{0x01, 0x3B, ImageConfig[23]},
		{0x01, 0x3C, ImageConfig[24]},
		{0x01, 0x66, ImageConfig[25]},

		{0x00, 0x05, 0x015c},	//Hblank
    	{0x00, 0x06, 0x0020},	//Vblank
	    {1, 0xC6, 0x2707    },  // MODE_OUTPUT_WIDTH_B
	    {1, 0xC8, 1280      },
	    {1, 0xC6, 0x2709    },  // MODE_OUTPUT_WIDTH_B
	    {1, 0xC8, 720      },


    	{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture        //Moving this part after maual time and exposure settings
    	{1, 0xC8, 0x0002    },
    	{1, 0xC6, 0xA104    },  // wait till become capture
    	{111, 0xC8, 0x0007  }
    };


//    for(lRetVal =0; lRetVal<26;lRetVal++)
//    	DEBG_PRINT("\nRegVal %x",ImageConfig[lRetVal]);

    lRetVal = RegLstWrite((s_RegList *)recapture_cmds_list,
                        sizeof(recapture_cmds_list)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
    return 0;
}

//******************************************************************************
//	Does a soft reset of MT9D111
//
//	return SUCCESS or failure code
//******************************************************************************
long SoftReset_ImageSensor()
{
	long lRetVal;

	s_RegList StatusRegLst[] = {{0x00, 0x65, 0xA000},	//Bypass PLL
								{0x01, 0xC3, 0x0501},	//MCU reset
								{0x00, 0x0D, 0x0021},	//SensorCore, SOC reset
								{0x00, 0x0D, 0x0000}};	//Disable the resets

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	MT9D111Delay(24/3 + MARGIN_NUMCLKS);	//Wait 24 clocks before using I2C

	return lRetVal;
}

//******************************************************************************
//	Verifies I2C communication with MT9D111
//
//	return SUCCESS or failure code
//******************************************************************************
long Verify_ImageSensor()
{
	long lRetVal;
	uint16_t usRegVal;
	s_RegList StatusRegLst = {0x00, 0x00, 0xBADD};	// Sensor Chip Version#

	lRetVal = Register_Read(&StatusRegLst, &usRegVal);

	DEBG_PRINT("MT9D111 DeviceID: %x\n", usRegVal);

	if (usRegVal == CHIP_VERSION)
	{
		DEBG_PRINT("I2C comm. with MT9D111 SUCCESS\n");
		lRetVal = SUCCESS;
	}
	else
	{
		DEBG_PRINT("Chip Ver# error\n");
		lRetVal = MT9D111_NOT_FOUND;
	}

	return lRetVal;
}

//******************************************************************************
//	Refresh MT9D111 MCU firmware
//
//	return SUCCESS or failure code
//******************************************************************************
int32_t Refresh_mt9d111Firmware()
{
	long lRetVal;

	s_RegList StatusRegLst[] = {
									{1, 0xC6, 0xA103}, {1, 0xC8, 0x0005}	//Refresh
								};	//Disable the resets

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	return lRetVal;
}

//******************************************************************************
//************************** IMAGING SETTINGS FNS ******************************

long WriteAllAEnAWBRegs()
{
	long lRetVal;

/*
	s_RegList StatusRegLst[] = {{0x00, 0x2B, 0x0026},
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

long ReadAllAEnAWBRegs()
{
	long lRetVal;

	lRetVal = AnalogGainReg_Read();
	ASSERT_ON_ERROR(lRetVal);
	lRetVal = DigitalGainRegs_Read();
	ASSERT_ON_ERROR(lRetVal);
	lRetVal = CCMRegs_Read();
	ASSERT_ON_ERROR(lRetVal);
	lRetVal = ShutterRegs_Read();
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

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


//Teg:Remove later
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

int32_t SetShutterWidth(uint16_t ShutterWidth)
{
	return Reg_Write(0x00,0x09,ShutterWidth);
}

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

int32_t SetInitialGain(uint8_t ValChange, bool IsInc)
{
	int32_t lRetVal;
	uint16_t CurGain=0,NewGain=0,tmpIGain=0;
	uint8_t Reg_Addr = 0x2B;
	uint8_t Cnt=0;

	for (Cnt=0;Cnt<4;Cnt++)
	{

		lRetVal = Reg_Read(0x00,(Reg_Addr+Cnt),&CurGain);

		NewGain = CurGain & ~INITIAL_GAIN_BITS;
		tmpIGain = CurGain &  INITIAL_GAIN_BITS;
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

		ASSERT_ON_ERROR(lRetVal);
	}

	return lRetVal;
}

int32_t ReadGainReg(uint16_t* Gains)
{
	uint8_t Reg_Addr = 0x2B;
	uint8_t Cnt=0;
	int32_t lRetVal;

	for (Cnt=0;Cnt<4;Cnt++)
	{

		lRetVal = Reg_Read(0x00,(Reg_Addr+Cnt),Gains + Cnt);
		DEBG_PRINT("Reg %x : %x\n",(Reg_Addr+Cnt),*(Gains + Cnt));
		ASSERT_ON_ERROR(lRetVal);
	}
	return lRetVal;
}
//************************* IMAGING SETTINGS FNS end ***************************
//******************************************************************************


//******************************************************************************
//***************************** STANDBY FNS ************************************

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
	ASSERT_ON_ERROR(lRetVal);

	if(ucMethod == SOFT_STANDBY)
	{
		//lRetVal = RegLstWrite(&stndby_cmds,1);
		lRetVal = RegLstWrite(&stndby_cmds[0],2);
		ASSERT_ON_ERROR(lRetVal);
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
		ASSERT_ON_ERROR(lRetVal);
	}
	else if(ucMethod == HARD_STANDBY)
	{
		MAP_GPIOPinWrite(GPIOA0_BASE, GPIO_PIN_7, 0);
	}

	lRetVal = RegLstWrite(stndby_cmds_list,
							(sizeof(stndby_cmds_list)/sizeof(s_RegList)));
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}
//****************************STANDBY FNS - end*********************************
//******************************************************************************



//******************************************************************************
//**********************BASIC REG READ/WRITE FNS *******************************

//******************************************************************************
//
//! This function implements the Register Write in MT9D111 sensor
//!
//! \param1                     Register List
//! \param2                     No. Of Items
//!
//! \return                     0 - Success
//!                             -1 - Error
//
//*****************************************************************************
static long RegLstWrite(s_RegList *pRegLst, unsigned long ulNofItems)
{
    unsigned long       ulNdx;
    unsigned short      usTemp;
    unsigned char       i;
    unsigned char       ucBuffer[20];
    unsigned long       ulSize;
    long lRetVal = -1;

    if(pRegLst == NULL)
    {
        return RET_ERROR;
    }

    for(ulNdx = 0; ulNdx < ulNofItems; ulNdx++)
    {
    	if(pRegLst->ucPageAddr == 100)
        {
    		DEBG_PRINT("1");
    		// PageAddr == 100, insret a delay equal to reg value
            //MT9D111Delay(pRegLst->usValue * 80000/3);
    		//MT9D111Delay(pRegLst->usValue * 80000/6);	//Change this based on no of clocks onclycle in MT9D111Delay takes
    		//MT9D111Delay(pRegLst->usValue * 4 * 80000/3);
    		osi_Sleep(pRegLst->usValue);
        }
        else if(pRegLst->ucPageAddr == 111)
        {
        	DEBG_PRINT("2:%d ", pRegLst->usValue);
        	// PageAddr == 111, wait for specified register value
        	//start_100mSecTimer();
        	uint32_t ulCounter = 0;
            do
            {
                ucBuffer[0] = pRegLst->ucRegAddr;
                lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,1,1);
                ASSERT_ON_ERROR(lRetVal);
                if(I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,1))
                {
                    return RET_ERROR;
                }

                usTemp = ucBuffer[0] << 8;
                usTemp |= ucBuffer[1];

                //DEBG_PRINT(".");
//                uint8_t ucTmp;
//                Variable_Read(0xA104, &ucTmp);
//                DEBG_PRINT("3:%d\n\r",ucTmp);
//
//                Variable_Read(0xA104, &ucTmp);
//                DEBG_PRINT("4:%d\n\r",ucTmp);
                //MT9D111Delay(10*10/2);	//Change 10/2 to 10 if UtilsDelay cycle is expected to take 3 clks only
                MT9D111Delay(.01 * 80000000 / 6);	//10m*80000000/6  = 10 milli sec
                DEBG_PRINT("%d", usTemp);
                ulCounter++;
                if(ulCounter > 1000)	//500*.01sec = 5 sec
                {
                	//stop_100mSecTimer();
                	return MT9D111_FIRMWARE_STATE_ERROR;
                }
            }while(usTemp != pRegLst->usValue);
            //stop_100mSecTimer();
        }
        else
        {
        	DEBG_PRINT("-");
            // Set the page
            ucBuffer[0] = SENSOR_PAGE_REG;
            ucBuffer[1] = 0x00;
            ucBuffer[2] = (unsigned char)(pRegLst->ucPageAddr);
            if(0 != I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,3,I2C_SEND_STOP))
            {
                return RET_ERROR;
            }

            ucBuffer[0] = SENSOR_PAGE_REG;
            lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,1,I2C_SEND_STOP);
            ASSERT_ON_ERROR(lRetVal);
            lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,I2C_SEND_STOP);
            ASSERT_ON_ERROR(lRetVal);

            ucBuffer[0] = pRegLst->ucRegAddr;

            if(pRegLst->ucPageAddr  == 0x1 && pRegLst->ucRegAddr == 0xC8)
            {
                usTemp = 0xC8;
                i=1;
                while(pRegLst->ucRegAddr == usTemp)
                {
                    ucBuffer[i] = (unsigned char)(pRegLst->usValue >> 8);
                    ucBuffer[i+1] = (unsigned char)(pRegLst->usValue & 0xFF);
                    i += 2;
                    usTemp++;
                    pRegLst++;
                    ulNdx++;
                }

                ulSize = (i-2)*2 + 1;
                ulNdx--;
                pRegLst--;
            }
            else
            {
                ulSize = 3;
                ucBuffer[1] = (unsigned char)(pRegLst->usValue >> 8);
                ucBuffer[2] = (unsigned char)(pRegLst->usValue & 0xFF);
            }

            if(0 != I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,
                                                      ulSize,I2C_SEND_STOP))
            {
                return RET_ERROR;
            }
        }

        pRegLst++;
        //MT9D111Delay(10);
        //MT9D111Delay(40);
        MT9D111Delay(10/2);	//Change 10/2 to 10 if UtilsDelay cycle is expected to take 3 clks only

    }

    return RET_OK;
}
//******************************************************************************
//	Variable_Read(): Reads value of one variable in MT9D111
//
//	param[in]	usVariableName:	16-bit variable name that contains driverID,
//									offset, etc.
//	param[out]	pusRegVal	Pointer to Value of Register
//
//	return SUCCESS or failure value
//
//******************************************************************************
long Variable_Read(uint16_t usVariableName, uint16_t* pusRegVal)
{
	long lRetVal;

	s_RegList RegLst[] = {	{1, 0xC6, usVariableName},
			    			{1, 0xC8, 0xBADD}	};
//	RegLst[0].usValue = usVariableName;

	lRetVal = RegLstWrite(RegLst, 1);
	lRetVal = Register_Read(&RegLst[1], &(RegLst[1].usValue));

	*pusRegVal = RegLst[1].usValue;

	return lRetVal;
}

//******************************************************************************
//	Variable_Write(): Writes value into one variable in MT9D111
//
//	param[in]	usVariableName:	16-bit variable name that contains driverID,
//									offset, etc.
//	param[out]	usRegVal	Value of Register
//
//	return SUCCESS or failure value
//
//******************************************************************************
long Variable_Write(uint16_t usVariableName, uint16_t usRegVal)
{
	long lRetVal;

	s_RegList RegLst[] = {	{1, 0xC6, usVariableName},
			    			{1, 0xC8, usRegVal}	};

	lRetVal = RegLstWrite(RegLst, 2);

	return lRetVal;
}

//******************************************************************************
//	Reg_Write(): Writes value into one register in MT9D111
//
//	param[in]	RegPage		page number
//	param[in]	usRegAddr	register address
//
//	param[in]	usRegVal	Value of Register
//
//	return SUCCESS or failure value
//******************************************************************************
long Reg_Write(uint8_t RegPage, uint16_t usRegAddr, uint16_t usRegVal)
{
	long lRetVal;

	s_RegList RegLst[] = {	{RegPage, usRegAddr, usRegVal} };

	lRetVal = RegLstWrite(RegLst, 1);

	return lRetVal;
}

//******************************************************************************
//	Reg_Read(): Reads value from one register in MT9D111
//
//	param[in]	RegPage		page number
//	param[in]	usRegAddr	register address
//
//	param[out]	usRegVal	Value of Register
//
//	return SUCCESS or failure value
//******************************************************************************
long Reg_Read(uint8_t RegPage, uint16_t usRegAddr, uint16_t* usRegVal)
{
	long lRetVal;

	s_RegList RegLst[] = {	{RegPage, usRegAddr, 0xBADD} };

	lRetVal = Register_Read(RegLst,usRegVal);

	return lRetVal;
}
//******************************************************************************
//	Register_Read(): Reads value of one register in MT9D111
//
//	param[in]	pRegLst(struct ptr):[in]ucPageAddr - Page of reg to be read
//									[in]ucRegAddr - Address of reg to be read
//									usVal - not used
//	param[out]	pusRegVal	Pointer to Value of Register
//
//	return SUCCESS or failure value
//
//	NOTE: Simple Register Read is implemented. Firmware Variable Read has been
//	implemented in Variable_Read(), or it has to be	implemented by calling
//	function using 0xC6 and 0xC8 registers.
//******************************************************************************
static long Register_Read(s_RegList *pRegLst, uint16_t* pusRegVal)
{
	unsigned char ucBuffer[20];
	//unsigned short usTemp;
	long lRetVal = -1;

	// Set the page
	ucBuffer[0] = SENSOR_PAGE_REG;	//Page Change register available in all pages
	ucBuffer[1] = 0x00;				//Most Significant Byte to be written in the register
	ucBuffer[2] = (unsigned char)(pRegLst->ucPageAddr);	//LSByte to be written in the register
	if(0 != I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,3,I2C_SEND_STOP))
	{
	   return RET_ERROR;
	}
	ucBuffer[0] = SENSOR_PAGE_REG;
	lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,1,I2C_SEND_STOP);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,I2C_SEND_STOP);
	ASSERT_ON_ERROR(lRetVal);

	//usTemp = ucBuffer[0] << 8;
	//usTemp |= ucBuffer[1];
	//DEBG_PRINT("Page no now: %x\n\r", usTemp);

	//Read from the register
	ucBuffer[0] = pRegLst->ucRegAddr;
	lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,1,1);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,1);
	ASSERT_ON_ERROR(lRetVal);

	*pusRegVal = ucBuffer[0] << 8;
	*pusRegVal |= ucBuffer[1];
	//DEBG_PRINT("Register Val: %x\n\r", *pusRegVal);

	pRegLst->usValue = *pusRegVal;

	return lRetVal;
}

//**********************BASIC REG READ/WRITE FNS - end**************************
//******************************************************************************

//******************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

/*

long JPEGDataLength_read()
{
	long lRetVal;
	uint16_t usRegVal;
	uint32_t uiLength;
	int i;

	s_RegList RegLst[] = {	{0x02, 0x02, 0xBADD},
							{0x02, 0x03, 0xBADD}};

	DEBG_PRINT("JPEG DataLen Reg:\n");

	for(i=0; i<(sizeof(RegLst)/sizeof(s_RegList)); i++)
	{
		lRetVal = Register_Read(&RegLst[i], &usRegVal);
		//DEBG_PRINT("Register Val: %x\n\r", StatusRegLst[i].usValue);
	}

	uiLength = (RegLst[1].usValue) + ( ((int)(RegLst[0].usValue & 0xFF00)) << 8);

	DEBG_PRINT("Image Len:%d\n", uiLength);
	return lRetVal;
}

// Dont use this fn. Not able to reset
// Trying ImageSensor MCU/Firmware disable or reset
// Not succeeded. Some fields in R195 which hold these controls are ReadOnly
long ResetImageSensorMCU()
{
	long lRetVal;
	uint16_t usRegVal;
	s_RegList StatusRegLst[] = {0x01, 0xC3, 0x0001};

	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);
	StatusRegLst[1].usValue = usRegVal | (StatusRegLst[1].usValue);
	lRetVal = RegLstWrite(StatusRegLst, 1);

	return lRetVal;
}

long ReadMCUBootModeReg()
{
	long lRetVal;
	uint16_t usRegVal;
	s_RegList StatusRegLst[] = {0x01, 0xC3, 0xBADD};

	lRetVal = Register_Read(&StatusRegLst[1], &usRegVal);

	return lRetVal;
}


long Snap()
{

    long lRetVal = -1;
//
    lRetVal = RegLstWrite((s_RegList *)snap,
                        sizeof(snap)/sizeof(s_RegList));
    ASSERT_ON_ERROR(lRetVal);
    return 0;

//	uint16_t usRegVal=0;
//	int i=0;
//	s_RegList StatusRegLst[] = {{0x00, 0x0D, 0xBADD},
//								};
//
//	for(i=0; i<(sizeof(StatusRegLst)/sizeof(s_RegList)); i++)
//	{
//		lRetVal = Register_Read(&StatusRegLst[i], &usRegVal);
//		DEBG_PRINT("\nReg %x : %x",StatusRegLst[i].ucRegAddr,usRegVal);
//	}
}

//Teg:Remove later
int32_t toggle_standby()
{
	int32_t lRetVal;

	s_RegList stndby_cmds_list[] = {
			{1, 0xC6, 0xA103},	//Conext A/Preview; seq.cmd = 1(preview cmd)
			{1, 0xC8, 0x0001},
			{1, 0xC6, 0xA104},	//Wait till in A; seq.state = 3(preview state)
			{111, 0xC8, 0x0003},

//			{1, 0xC6, 0xA103},	//Conext A/Preview; seq.cmd = 1(preview cmd)
//			{1, 0xC8, 0x0001},
//			{1, 0xC6, 0xA104},	//Wait till in A; seq.state = 3(preview state)
//			{111, 0xC8, 0x0003}

			{1, 0xC6, 0xA103},	//Standby firmware; seq.cmd = 3(standby cmd)
			{1, 0xC8, 0x0003},
			{1, 0xC6, 0xA104},	//Wait till stanby; seq.state = 9(standby state)
			{111, 0xC8, 0x0009}

				};

	lRetVal = RegLstWrite(stndby_cmds_list, (sizeof(stndby_cmds_list)/sizeof(s_RegList)));
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}


long RegStatusRead(uint16_t* pusRegVal)
{
	long lRetVal;

	s_RegList StatusRegLst = {0x02, 0x02, 0xBADD};

	DEBG_PRINT("Status Reg:");

	lRetVal = Register_Read(&StatusRegLst, pusRegVal);

//	if(usRegVal != 0x30)
//	{
//		DEBG_PRINT("Note Reg val\n\r");
//	}

	return lRetVal;
}

long LL_Configs()
{
	long lRetVal;

	s_RegList RegLst[] = {
							{1, 0xC6, 0xA743}, {1, 0xC8, 0x0042},	//Gamma and contrast. Context A
							{1, 0xC6, 0xA744}, {1, 0xC8, 0x0042},	//Gamma and contrast. Context B

							{1, 0xC6, 0xA115}, {1, 0xC8, 0x007f},	//LL
							{1, 0xC6, 0xA118}, {1, 0xC8, 0x0040},	//LL Saturation
							{1, 0xC6, 0xA103}, {1, 0xC8, 0x0005}	//Refresh
						};

	lRetVal = RegLstWrite(RegLst, (sizeof(RegLst)/sizeof(s_RegList)));

	return lRetVal;

}

long JpegConfigReg_Read()
{
	long lRetVal;

	s_RegList StatusRegLst[] = {{1,  0xC6, 0xA907   },
			    				{1, 0xC8, 0x0001    }  };
	DEBG_PRINT("JPEG Config Reg(0xA907):\n");
	lRetVal = RegLstWrite(StatusRegLst, 1);
	lRetVal = Register_Read(&StatusRegLst[1], &(StatusRegLst[1].usValue));

	return lRetVal;
}



int32_t BeginCapture_MT9D111()
{
	long lRetVal;

	s_RegList StatusRegLst[] = 	{
//									{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture
//									{1, 0xC8, 0x0002    },
									{100, 0x00, 0x01F4  },  // Delay =500ms
								};

	lRetVal = RegLstWrite(StatusRegLst, (sizeof(StatusRegLst)/sizeof(s_RegList)));

	return lRetVal;
}
*/
