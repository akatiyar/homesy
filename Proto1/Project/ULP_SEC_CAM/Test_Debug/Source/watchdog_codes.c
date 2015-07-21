//*****************************************************************************
//
//! The interrupt handler for the watchdog timer
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void WatchdogIntHandler(void)
{
	PRCMSOCReset();
}

void WatchdogIntHandler(void);

/*			UART_PRINT("###\n\r");
			WDT_IF_DeInit();
			bRetcode = MAP_WatchdogRunning(WDT_BASE);
			if(bRetcode)
			{
			   UART_PRINT("yes\n\r");
			}
			else
			{
				UART_PRINT("no\n\r");
			}*/
			
			/*#define IMAGE_CAPTURE_TIMEOUT	5000	//in milli sec
			uint8_t bRetcode;
			WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(5000));
			//WDT_IF_Init(NULL, MILLISECONDS_TO_TICKS(IMAGE_CAPTURE_TIMEOUT/2));
			bRetcode = MAP_WatchdogRunning(WDT_BASE);
			if(bRetcode)
			{
			   UART_PRINT("yes\n\r");
			}
			else
			{
				UART_PRINT("no\n\r");
			}
			UART_PRINT("$$$\n\r");*/