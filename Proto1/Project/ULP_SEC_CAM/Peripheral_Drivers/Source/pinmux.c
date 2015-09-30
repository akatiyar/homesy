//*****************************************************************************
// pinmux.c
//
// configure the device pins for different peripheral signals
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

#include "pinmux.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "rom.h"
#include "rom_map.h"
#include "gpio.h"
#include "prcm.h"

//*****************************************************************************
// Enable clocks to peripherals and set the pin mux
//
//	param	none
//	return	none
//*****************************************************************************
void PinMuxConfig(void)
{
    //
    // Enable Peripheral Clocks 
    //
    MAP_PRCMPeripheralClkEnable(PRCM_CAMERA, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);//Wake-src, Cam Standby
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);	//LED
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK); //For debug msgs
	//
	// Configure PIN_01 for UART1 UART1_TX
	//
	MAP_PinTypeUART(PIN_01, PIN_MODE_7);

    //
    // Configure PIN_55 for CAMERA0 CAM_pCLK
    //
    MAP_PinTypeCamera(PIN_55, PIN_MODE_4);

    //
    // Configure PIN_58 for CAMERA0 CAM_pDATA7
    //
    MAP_PinTypeCamera(PIN_58, PIN_MODE_4);

    //
    // Configure PIN_59 for CAMERA0 CAM_pDATA6
    //
    MAP_PinTypeCamera(PIN_59, PIN_MODE_4);

    //
    // Configure PIN_60 for CAMERA0 CAM_pDATA5
    //
    MAP_PinTypeCamera(PIN_60, PIN_MODE_4);

    //
    // Configure PIN_61 for CAMERA0 CAM_pDATA4
    //
    MAP_PinTypeCamera(PIN_61, PIN_MODE_4);

    //
    // Configure PIN_02 for CAMERA0 CAM_pXCLK
    //
    MAP_PinTypeCamera(PIN_02, PIN_MODE_4);

    //
    // Configure PIN_03 for CAMERA0 CAM_vS
    //
    MAP_PinTypeCamera(PIN_03, PIN_MODE_4);

    //
    // Configure PIN_04 for CAMERA0 CAM_hS
    //
    MAP_PinTypeCamera(PIN_04, PIN_MODE_4);

    //
    // Configure PIN_05 for CAMERA0 CAM_pDATA8
    //
    MAP_PinTypeCamera(PIN_05, PIN_MODE_4);

    //
    // Configure PIN_06 for CAMERA0 CAM_pDATA9
    //
    MAP_PinTypeCamera(PIN_06, PIN_MODE_4);

    //
    // Configure PIN_07 for CAMERA0 CAM_pDATA10
    //
    MAP_PinTypeCamera(PIN_07, PIN_MODE_4);

    //
    // Configure PIN_08 for CAMERA0 CAM_pDATA11
    //
    MAP_PinTypeCamera(PIN_08, PIN_MODE_4);

    //
    // Configure PIN_16 for I2C0 I2C_SCL
    //
    MAP_PinTypeI2C(PIN_16, PIN_MODE_9);

    //
    // Configure PIN_17 for I2C0 I2C_SDA
    //
    MAP_PinTypeI2C(PIN_17, PIN_MODE_9);

    //
	// Configure PIN_57 for GPIOIn - Wake-Src
	//
	MAP_PinTypeGPIO(PIN_57, PIN_MODE_0, false);
	MAP_GPIODirModeSet(GPIOA0_BASE, 0x04, GPIO_DIR_MODE_IN);

    //
	// Configure PIN_63 for GPIOIn - Config_SW1
	//
	MAP_PinTypeGPIO(PIN_63, PIN_MODE_0, false);
	MAP_GPIODirModeSet(GPIOA1_BASE, 0x01, GPIO_DIR_MODE_IN);

	//
	// Configure PIN_62 for GPIOOut - CameraModule Standby
	//
	MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
	MAP_GPIODirModeSet(GPIOA0_BASE, 0x80, GPIO_DIR_MODE_OUT);

	//
	// Configure PIN_64 for GPIOOut - LED
	//
	MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
	MAP_GPIODirModeSet(GPIOA1_BASE, 0x02, GPIO_DIR_MODE_OUT);
}
