/*
 * experimentt_codes.c
 *
 *  Created on: 25-May-2015
 *      Author: Chrysolin
 */

// Slow Clk ctr working
uint64_t ullSlowCounterReturnVal1, ullSlowCounterReturnVal2;
while(1)
{
ullSlowCounterReturnVal1 = PRCMSlowClkCtrGet();
UtilsDelay(80000000/6);
ullSlowCounterReturnVal2 = PRCMSlowClkCtrGet();
UART_PRINT("%lld\n\r", ullSlowCounterReturnVal1);
UART_PRINT("%lld\n\r\n\r", ullSlowCounterReturnVal2);
}

//Reset cause get
uint32_t ulResetCause;
    ulResetCause = MAP_PRCMSysResetCauseGet();
    if(PRCM_POWER_ON == ulResetCause)
    {
    	UART_PRINT("Device is powering on\n\r");
    }
    else if(PRCM_LPDS_EXIT == ulResetCause)
    {
    	UART_PRINT("Device is exiting from LPDS.\n\r");
    }
    else if(PRCM_CORE_RESET == ulResetCause)
    {
    	UART_PRINT("Device is exiting soft core only reset\n\r");
    }
    else if(PRCM_MCU_RESET == ulResetCause)
    {
    	UART_PRINT("Device is exiting soft subsystem reset.\n\r");
    }
    else if(PRCM_WDT_RESET == ulResetCause)
    {
    	UART_PRINT("Device was reset by watchdog.\n\r");
    }
    else if(PRCM_SOC_RESET == ulResetCause)
    {
    	UART_PRINT("Device is exting SOC reset.\n\r");
    }
    else if(PRCM_HIB_EXIT == ulResetCause)
    {
    	UART_PRINT("Device is exiting hibernate\n\r");
    }





    //	PCLK_Rate_read();

    //	ReadAllAEnAWBRegs();

    	/*while(1)
    	{
    		//AnalogGainReg_Read();
    		ReadAllAEnAWBRegs();

    		disableAE();
    		disableAWB();

    		ReadAllAEnAWBRegs();

    		enableAE();
    		enableAWB();
    	}*/
