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
#include "mt9d111.h"

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

	 {1, 0xC6, 0x2725    },{1, 0xC8, 0x0000    },  // Clock Settings R0x0A
	 {1, 0xC6, 0xA117    },{1, 0xC8, 0x005f    },  // Enable low lighting in the driver
    {1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture	//Moving this part after maual time and exposure settings
    {1, 0xC8, 0x0002    },
	{1, 0xC6, 0xA104    },  // wait till become capture
	{111, 0xC8, 0x0007  }
    //{100, 0x00, 0x01F4  },  // Delay =500ms
};

/*static  const s_RegList recapture_cmds_list[]= {

//	{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 },  // SEQ_MODE Will turn off AE, AWB
//    {0,  0x65,  0x2000  },  // Enable PLL
//    {0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
//	{100, 0x00, 0x0064  },  // Delay =100ms
//    {1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture	//Moving this part after maual time and exposure settings
//    {1, 0xC8, 0x0002    },
//    {1, 0xC6, 0xA104    },  // wait till become capture
//    {111, 0xC8, 0x0007   }

	{1, 0xC6, 0xA102},{1, 0xC8, 0x0000 },  // SEQ_MODE Will turn off AE, AWB
	{0,  0x65,  0x2000  },  // Enable PLL
	{0, 0x20, 0x0000    },  // READ_MODE_B (Image flip settings)
	{100, 0x00, 0x0064  },  // Delay =100ms

	{0x00, 0x2B, ((0x0024<<1)|0x0080)},        //Lines exist. tfss-9203b65b-f7ea-42b8-b9e8-be5366de68bf-myPic1.jpg
	{0x00, 0x2C, ((0x003A<<1)|0x0080)},
	{0x00, 0x2D, ((0x0024<<1)|0x0080)},
	{0x00, 0x2E, ((0x0024<<1)|0x0080)},

	//{0x00, 0x09, (0x005F)},         //Integration time = 5mS
//	{0x00, 0x09, (200)},         //Integration time = 10mS
	{0x00, 0x09, (270)},         //Integration time = 15mS
	{0x00, 0x0C, 0x0000},

	{0x01, 0x6E, 0x0085},
	{0x01, 0x6A, 0x008D},
	{0x01, 0x6B, 0x008D},
	{0x01, 0x6C, 0x0093},
	{0x01, 0x6D, 0x008d},
	{0x01, 0x4E, 0x0020},



	{0x01, 0x60, 0x291A},
	{0x01, 0x61, 0x04E4},
	{0x01, 0x62, 0xbab5},
	{0x01, 0x63, 0xb001},
	{0x01, 0x64, 0x4089},
	{0x01, 0x65, 0xf17c},
	//        {0x01, 0x48, 0x0101},
	{0x00, 0x05, 0x015c},	//Hblank
	{0x00, 0x06, 0x0020},	//Vblank


	{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture        //Moving this part after maual time and exposure settings
	{1, 0xC8, 0x0002    },
	{1, 0xC6, 0xA104    },  // wait till become capture
	{111, 0xC8, 0x0007  }
};

static  const s_RegList snap[]= {

										{0, 0x0D, 0x0282    },

//										{1, 0xC6, 0xA103    },  // SEQ_CMD, Do capture        //Moving this part after maual time and exposure settings
//										{1, 0xC8, 0x0002    },
//
//										{1, 0xC6, 0xA104    },  // wait till become capture
//										{111, 0xC8, 0x0007  }
};*/

#endif

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

    lRetVal = RegLstWrite((s_RegList *)init_cmds_list, \
                                    sizeof(init_cmds_list)/sizeof(s_RegList));
//    RETURN_ON_ERROR(lRetVal);

#ifndef ENABLE_JPEG
    lRetVal = RegLstWrite((s_RegList *)preview_cmds_list,
                      sizeof(preview_cmds_list)/sizeof(s_RegList));
    RETURN_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)image_size_240_320_preview_cmds_list, \
                    sizeof(image_size_240_320_preview_cmds_list)/ \
                    sizeof(s_RegList));
    RETURN_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)freq_setup_cmd_List,
                    sizeof(freq_setup_cmd_List)/sizeof(s_RegList));
    RETURN_ON_ERROR(lRetVal);
    lRetVal = RegLstWrite((s_RegList *)preview_on_cmd_list,
                    sizeof(preview_on_cmd_list)/sizeof(s_RegList));
    RETURN_ON_ERROR(lRetVal);
#endif 
    return lRetVal;
}

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
    RETURN_ON_ERROR(lRetVal);
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
    RETURN_ON_ERROR(lRetVal);
    return 0;
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
	RETURN_ON_ERROR(lRetVal);

	lRetVal = WriteFile_ToFlash(ucHeader,
								FILENAME_SENSORCONFIGS,
								sizeof(ucHeader),
								offset,
								MULTIPLE_WRITE_FIRST,
								&lFileHandle);
	RETURN_ON_ERROR(lRetVal);
	offset += lRetVal;

	lRetVal = WriteFile_ToFlash((uint8_t*)&init_cmds_list[0],
									FILENAME_SENSORCONFIGS,
									sizeof(init_cmds_list),
									offset,
									MULTIPLE_WRITE_MIDDLE,
									&lFileHandle);
	RETURN_ON_ERROR(lRetVal);
	offset += lRetVal;

	lRetVal = WriteFile_ToFlash((uint8_t*)&capture_cmds_list[0],
										FILENAME_SENSORCONFIGS,
										sizeof(capture_cmds_list),
										offset,
										MULTIPLE_WRITE_LAST,
										&lFileHandle);
	RETURN_ON_ERROR(lRetVal);
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
	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}
#endif

//******************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
