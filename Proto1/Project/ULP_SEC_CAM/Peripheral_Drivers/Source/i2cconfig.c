//*****************************************************************************
// i2cconfig.c
//
// I2C features APIs
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
//! \addtogroup i2cconfig
//! @{
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "i2c.h"
#include "prcm.h"
#include "i2cconfig.h"
#include "common.h"

#define I2C_MRIS_CLKTOUT        0x2
//*****************************************************************************
// I2C transaction time-out value.
// Set to value 0x7D. (@100KHz it is 20ms, @400Khz it is 5 ms)
//*****************************************************************************
#define I2C_TIMEOUT_VAL         0x7D
//#define I2C_TIMEOUT_VAL         0x10
#define I2C_BASE                I2CA0_BASE


void MT9D111Delay(unsigned long ucDelay);

//*****************************************************************************
//
//! This function implements delay in the camera sensor
//!
//! \param                      delay value
//!
//! \return                     None
//
//*****************************************************************************
#if defined(ewarm)
    void MT9D111Delay(unsigned long ucDelay)
    {
    __asm("    subs    r0, #1\n"
          "    bne.n   MT9D111Delay\n"
          "    bx      lr");
    }
#endif
#if defined(ccs)	//Eac hcycle takes 6 clocks now

    __asm("    .sect \".text:MT9D111Delay\"\n"
          "    .clink\n"
          "    .thumbfunc MT9D111Delay\n"
          "    .thumb\n"
          "    .global MT9D111Delay\n"
          "MT9D111Delay:\n"
          "    subs r0, #1\n"
          "    bne.n MT9D111Delay\n"
          "    bx lr\n");

#endif

//****************************************************************************
//
//! Invokes the I2C driver APIs to read from the device. This assumes the 
//! device local address to read from is set using the I2CWrite API.
//!
//! \param      ucDevAddr is the device I2C slave address
//! \param      ucBuffer is the pointer to the read data to be placed
//! \param      ulSize is the length of data to be read
//! \param      ucFlags Flag
//! 
//! This function works in a polling mode,
//!    1. Writes the device register address to be written to.
//!    2. In a loop, reads all the bytes over I2C
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
long I2CBufferRead(unsigned char ucDevAddr, unsigned char *ucBuffer,
                            unsigned long ulSize,unsigned char ucFlags)
{
    unsigned long ulNdx;

    // Set I2C codec slave address 
    MAP_I2CMasterSlaveAddrSet(I2CA0_BASE,ucDevAddr, true);
    MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);
    //
	// Set the time-out. Not to be used with breakpoints.
	//
    MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

    if(ulSize == 1)
    {
        // Start single transfer. 
        MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    }
    else
    {
        // Start the transfer. 
        MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

        // Wait for transfer completion. 
//        while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) &
//                                                   I2C_INT_MASTER) == 0)
        while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                           & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)
        {
        }

        //
        // Check for any errors in transfer
        //
        if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
        {
            switch(I2C_MASTER_CMD_BURST_RECEIVE_START)
            {
            case I2C_MASTER_CMD_BURST_SEND_START:
            case I2C_MASTER_CMD_BURST_SEND_CONT:
            case I2C_MASTER_CMD_BURST_SEND_STOP:
                MAP_I2CMasterControl(I2C_BASE,
                             I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
                break;
            case I2C_MASTER_CMD_BURST_RECEIVE_START:
            case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
            case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
                MAP_I2CMasterControl(I2C_BASE,
                             I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
                break;
            default:
                break;
            }
            return FAILURE;
            //return MAP_I2CMasterErr(I2C_BASE);
        }

        // Read first byte from the controller. 
        ucBuffer[0] = MAP_I2CMasterDataGet(I2CA0_BASE);

        for(ulNdx=1; ulNdx < ulSize-1; ulNdx++)
        {
        	//MT9D111Delay(10);
        	MT9D111Delay(10/2);	//Change 10/2 to 10 if UtilsDelay cycle is expected to take 3 clks only
            //MT9D111Delay(40);
            MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);

            //
		   // Set the time-out. Not to be used with breakpoints.
		   //
		   MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

            // continue the transfer. 
            MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            // Wait for transfer completion. 
//            while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) &
//                                                         I2C_INT_MASTER) == 0)
            while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                               & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)
            {
            }

            //
            // Check for any errors in transfer
            //
            if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
            {
                switch(I2C_MASTER_CMD_BURST_RECEIVE_CONT)
                {
                case I2C_MASTER_CMD_BURST_SEND_START:
                case I2C_MASTER_CMD_BURST_SEND_CONT:
                case I2C_MASTER_CMD_BURST_SEND_STOP:
                    MAP_I2CMasterControl(I2C_BASE,
                                 I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
                    break;
                case I2C_MASTER_CMD_BURST_RECEIVE_START:
                case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
                case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
                    MAP_I2CMasterControl(I2C_BASE,
                                 I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
                    break;
                default:
                    break;
                }
                return FAILURE;
                //return MAP_I2CMasterErr(I2C_BASE);
            }

            // Read next byte from the controller. 
            ucBuffer[ulNdx] = MAP_I2CMasterDataGet(I2CA0_BASE);
        }

        MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);

		//
		// Set the time-out. Not to be used with breakpoints.
		//
		MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

        MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    }

    // Wait for transfer completion. 
    //while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) & I2C_INT_MASTER) == 0)
    while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                       & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)
    {
    }

    //
    // Check for any errors in transfer
    //
    if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
    {
        switch(I2C_MASTER_CMD_BURST_RECEIVE_FINISH)
        {
        case I2C_MASTER_CMD_BURST_SEND_START:
        case I2C_MASTER_CMD_BURST_SEND_CONT:
        case I2C_MASTER_CMD_BURST_SEND_STOP:
            MAP_I2CMasterControl(I2C_BASE,
                         I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            break;
        case I2C_MASTER_CMD_BURST_RECEIVE_START:
        case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
        case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
            MAP_I2CMasterControl(I2C_BASE,
                         I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
            break;
        default:
            break;
        }
        return FAILURE;
        //return MAP_I2CMasterErr(I2C_BASE);
    }
    // Read the last byte from the controller. 
    ucBuffer[ulSize-1] = MAP_I2CMasterDataGet(I2CA0_BASE);

    return 0;
}

//****************************************************************************
//
//! Invokes the I2C driver APIs to write to the specified address
//!
//! \param ucDevAddr is the device I2C slave address
//! \param ucBuffer is the pointer to the data to be written
//! \param ulSize is the length of data to be written
//! \param ucFlags
//! 
//! This function works in a polling mode,
//!    1. Writes the device register address to be written to.
//!    2. In a loop, writes all the bytes over I2C
//!
//! \return 0: Success, < 0: Failure.
//
//****************************************************************************
long I2CBufferWrite(unsigned char ucDevAddr, unsigned char *ucBuffer,
                             unsigned long ulSize,unsigned char ucFlags)
{
    unsigned long ulNdx;

   // Set I2C codec slave address 
    MAP_I2CMasterSlaveAddrSet(I2CA0_BASE,ucDevAddr, false);

   // Write the first byte to the controller. 
    MAP_I2CMasterDataPut(I2CA0_BASE,ucBuffer[0]);
    MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);
    //
	// Set the time-out. Not to be used with breakpoints.
	//
    MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

    if( ulSize == 1)
    {

        MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    }
    else
    {
        // Continue the transfer.
        MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

       // Wait until the current byte has been transferred. 
        while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                    & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)
     //while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) & I2C_INT_MASTER) == 0)
        {
        }
        //
        // Check for any errors in transfer
        //
        if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
        {
            switch(I2C_MASTER_CMD_BURST_SEND_START)
            {
            case I2C_MASTER_CMD_BURST_SEND_START:
            case I2C_MASTER_CMD_BURST_SEND_CONT:
            case I2C_MASTER_CMD_BURST_SEND_STOP:
                MAP_I2CMasterControl(I2C_BASE,
                             I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
                break;
            case I2C_MASTER_CMD_BURST_RECEIVE_START:
            case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
            case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
                MAP_I2CMasterControl(I2C_BASE,
                             I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
                break;
            default:
                break;
            }
            return FAILURE;
            //return MAP_I2CMasterErr(I2C_BASE);
        }
        for(ulNdx=1; ulNdx < ulSize-1; ulNdx++)
        {
           // Write the next byte to the controller. 
            MAP_I2CMasterDataPut(I2CA0_BASE,ucBuffer[ulNdx]);

           // Clear Master Interrupt 
            MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);

            //
            // Set the time-out. Not to be used with breakpoints.
            //
            MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

           // Continue the transfer. 
            MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

           // Wait until the current byte has been transferred. 
//            while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) & I2C_INT_MASTER)
//                                                                 == 0)
            while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                               & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)

            {
            }
            //
            // Check for any errors in transfer
            //
            if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
            {
                switch(I2C_MASTER_CMD_BURST_SEND_CONT)
                {
                case I2C_MASTER_CMD_BURST_SEND_START:
                case I2C_MASTER_CMD_BURST_SEND_CONT:
                case I2C_MASTER_CMD_BURST_SEND_STOP:
                    MAP_I2CMasterControl(I2C_BASE,
                                 I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
                    break;
                case I2C_MASTER_CMD_BURST_RECEIVE_START:
                case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
                case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
                    MAP_I2CMasterControl(I2C_BASE,
                                 I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
                    break;
                default:
                    break;
                }
                return FAILURE;
                //return MAP_I2CMasterErr(I2C_BASE);
            }
        }

       // Write the last byte to the controller. 
        MAP_I2CMasterDataPut(I2CA0_BASE, ucBuffer[ulSize-1]);
        MAP_I2CMasterIntClearEx(I2CA0_BASE, I2C_INT_MASTER);

        //
	   // Set the time-out. Not to be used with breakpoints.
	   //
	   MAP_I2CMasterTimeoutSet(I2C_BASE, I2C_TIMEOUT_VAL);

       // End the transfer. 
        MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);
    }

   // Wait until the current byte has been transferred. 
    //while((MAP_I2CMasterIntStatusEx(I2CA0_BASE, false) & I2C_INT_MASTER) == 0)
    while((MAP_I2CMasterIntStatusEx(I2C_BASE, false)
                       & (I2C_INT_MASTER | I2C_MRIS_CLKTOUT)) == 0)
    {
    }
    //
    // Check for any errors in transfer
    //
    if(MAP_I2CMasterErr(I2C_BASE) != I2C_MASTER_ERR_NONE)
    {
        switch(I2C_MASTER_CMD_BURST_SEND_FINISH)
        {
        case I2C_MASTER_CMD_BURST_SEND_START:
        case I2C_MASTER_CMD_BURST_SEND_CONT:
        case I2C_MASTER_CMD_BURST_SEND_STOP:
            MAP_I2CMasterControl(I2C_BASE,
                         I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
            break;
        case I2C_MASTER_CMD_BURST_RECEIVE_START:
        case I2C_MASTER_CMD_BURST_RECEIVE_CONT:
        case I2C_MASTER_CMD_BURST_RECEIVE_FINISH:
            MAP_I2CMasterControl(I2C_BASE,
                         I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
            break;
        default:
            break;
        }
        return FAILURE;
        //return MAP_I2CMasterErr(I2C_BASE);
    }

    return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
