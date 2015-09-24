/*
 * mt9d111_driver_read_write_fns.c
 *
 *  Created on: 21-Sep-2015
 *      Author: Chrysolin
 */

#include "mt9d111.h"

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
long RegLstWrite(s_RegList *pRegLst, unsigned long ulNofItems)
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
                RETURN_ON_ERROR(lRetVal);
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
                	DEBG_PRINT("\n");
                	return MT9D111_FIRMWARE_STATE_ERROR;
                }
            }while(usTemp != pRegLst->usValue);
            //stop_100mSecTimer();
            DEBG_PRINT("\n");
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
            RETURN_ON_ERROR(lRetVal);
            lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,I2C_SEND_STOP);
            RETURN_ON_ERROR(lRetVal);

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
long Register_Read(s_RegList *pRegLst, uint16_t* pusRegVal)
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
	RETURN_ON_ERROR(lRetVal);

	lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,I2C_SEND_STOP);
	RETURN_ON_ERROR(lRetVal);

	//usTemp = ucBuffer[0] << 8;
	//usTemp |= ucBuffer[1];
	//DEBG_PRINT("Page no now: %x\n\r", usTemp);

	//Read from the register
	ucBuffer[0] = pRegLst->ucRegAddr;
	lRetVal = I2CBufferWrite(CAM_I2C_SLAVE_ADDR,ucBuffer,1,1);
	RETURN_ON_ERROR(lRetVal);

	lRetVal = I2CBufferRead(CAM_I2C_SLAVE_ADDR,ucBuffer,2,1);
	RETURN_ON_ERROR(lRetVal);

	*pusRegVal = ucBuffer[0] << 8;
	*pusRegVal |= ucBuffer[1];
	//DEBG_PRINT("Register Val: %x\n\r", *pusRegVal);

	pRegLst->usValue = *pusRegVal;

	return lRetVal;
}
