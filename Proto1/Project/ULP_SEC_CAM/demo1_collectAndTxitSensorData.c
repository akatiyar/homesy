void collectTxitSensorData(void *pvParameters)
{
    int iSocketDesc;
    long lRetVal = -1;

    unsigned long ulDestinationIP = 0xC0A801AC;
    uint8_t ucData[] = "hello";
    void* pvTcpData;
    float fTcpData[4];
    float* pfTcpData = fTcpData;


    DBG_PRINT("Collect and Transmit Sensor Data Function Begins\n\r");

    //
    // Reset The state of the machine
    //
    Network_IF_ResetMCUStateMachine();

    //
    // Start the driver
    //
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to start SimpleLink Device\n\r");
       return;
    }

    // Initialize AP security params
    SecurityParams.Key = (signed char *)SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    //
    // Connect to the Access Point
    //
    lRetVal = Network_IF_ConnectAP(SSID_NAME,SecurityParams);
    if(lRetVal < 0)
    {
       UART_PRINT("Connection to an AP failed\n\r");
       LOOP_FOREVER();
    }

    //
    // Create a TCP connection to the server
    //
    iSocketDesc = CreateConnection(ulDestinationIP);
    if(iSocketDesc < 0)
    {
        DBG_PRINT("Socket creation failed.\n\r");
        goto end;
    }

    struct SlTimeval_t timeVal;
    timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
    timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
    lRetVal = sl_SetSockOpt(iSocketDesc,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
                    (unsigned char*)&timeVal, sizeof(timeVal));
    if(lRetVal < 0)
    {
       ERR_PRINT(lRetVal);
       LOOP_FOREVER();
    }

    lRetVal = send(iSocketDesc, ucData, sizeof(ucData), 0);
    if(lRetVal < 0)
	{
		UART_PRINT("Failed to send data \n\r",lRetVal);
		goto end;
	}
    UART_PRINT("\nTest send() return %ld\n\r", lRetVal);

    // Initialization of accelerometer/Magnetometer
    verifyAccelMagnSensor();
    configureFXOS8700(0);

    // Initialization of Temperature and RH Sensor
    softResetTempRHSensor();
    configureTempRHSensor();

    while(1)
    {
    	//if(GPIOPinRead(GPIOA2_BASE,0x40))
    	{
    		UART_PRINT("\n\rCollecting Sensor data\n\r");

    		UART_PRINT("\n\rTemperature RH Sensor\n\r");
    		float_t fTemp, fRH;
    		softResetTempRHSensor();
 		    configureTempRHSensor();
    		getTempRH( &fTemp, &fRH );
    		UART_PRINT("Temp: %f\n RH: %f\n", fTemp, fRH);

    		UART_PRINT("\n\r\n\rAcceleration, Magnetic Fld Sensor\n\r");
    		float_t fAccel;
    		getAccelerationMagnitude( &fAccel );
    		UART_PRINT( "\nAcceleration: %f\n\r", fAccel);

//    		UART_PRINT("\n\rLight Sensor\n\r");
//    		uint16_t usLightIntensity;
//    		float_t fLightIntensity;
//    		verifyISL29035();
//    		configureISL29035(0);
//    		usLightIntensity = getLightsensor_data();
//    		fLightIntensity = (float)usLightIntensity;
//    		UART_PRINT( "\nLight Intensity: %d\n\r", usLightIntensity);
//    		UART_PRINT( "\nLight Intensity: %f\n\r", fLightIntensity);

//    		pvTcpData = ucData;
//    		*(float_t*)pvTcpData = fTemp;
//    		pvTcpData += sizeof(float_t);
//    		*(float_t*)pvTcpData = fRH;
//    		pvTcpData += sizeof(float_t);
//    		*(float_t*)pvTcpData = fAccel;
//    		pvTcpData += sizeof(float_t);

    		pfTcpData = fTcpData;
    		fTcpData[0] = fTemp;
    		fTcpData[1] = fRH;
    		fTcpData[2] = fAccel;
//    		fTcpData[3] = fLightIntensity;

    		UART_PRINT("Sending Sensor data\n\r");
    		lRetVal = send(iSocketDesc,
//    						ucData,
    						(char* )pfTcpData,
    					//	((char*)pvTcpData - ucData),
    						(sizeof(fTcpData)/* * sizeof(float)*/),
    						0);
			if(lRetVal < 0)
			{
				UART_PRINT("Failed to send data \n\r",lRetVal);
				goto end;
			}
			UART_PRINT("Sent Sensor data\n\r");

    		UtilsDelay(2*8000000/3);
    	}
    }

    //
    // Close the socket
    //
    lRetVal = close(iSocketDesc);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    DBG_PRINT("Socket closed\n\r");

end:
    //
    // Stop the driver
    //
    lRetVal = Network_IF_DeInitDriver();
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to stop SimpleLink Device\n\r");
       LOOP_FOREVER();
    }

    //
    // Loop here
    //
    LOOP_FOREVER();
}
