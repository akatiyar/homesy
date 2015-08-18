#include "app.h"

#include "accelomtrMagntomtr_fxos8700.h"
#include "tempRHSens_si7020.h"
#include "lightSens_isl29035.h"
#include "test_fns.h"

#include "math.h"
//void collectTxitSensorData(void *pvParameters)
//{
//    int iSocketDesc;
//    long lRetVal = -1;
//
//    unsigned long ulDestinationIP = 0xC0A801AC;
//    uint8_t ucData[] = "hello";
//   // void* pvTcpData;
//    float fTcpData[4];
//    float* pfTcpData = fTcpData;
//
//
//    DBG_PRINT("Collect and Transmit Sensor Data Function Begins\n\r");
//
//    //
//    // Reset The state of the machine
//    //
//    Network_IF_ResetMCUStateMachine();
//
//    //
//    // Start the driver
//    //
//    lRetVal = Network_IF_InitDriver(ROLE_STA);
//    if(lRetVal < 0)
//    {
//       DEBG_PRINT("Failed to start SimpleLink Device\n\r");
//       return;
//    }
//
//    // Initialize AP security params
//    SecurityParams.Key = (signed char *)SECURITY_KEY;
//    SecurityParams.KeyLen = strlen(SECURITY_KEY);
//    SecurityParams.Type = SECURITY_TYPE;
//
//    //
//    // Connect to the Access Point
//    //
//    lRetVal = Network_IF_ConnectAP(SSID_NAME,SecurityParams);
//    if(lRetVal < 0)
//    {
//       DEBG_PRINT("Connection to an AP failed\n\r");
//       LOOP_FOREVER();
//    }
//
//    //
//    // Create a TCP connection to the server
//    //
//    iSocketDesc = CreateConnection(ulDestinationIP);
//    if(iSocketDesc < 0)
//    {
//        DBG_PRINT("Socket creation failed.\n\r");
//        goto end;
//    }
//
//    struct SlTimeval_t timeVal;
//    timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
//    timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
//    lRetVal = sl_SetSockOpt(iSocketDesc,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
//                    (unsigned char*)&timeVal, sizeof(timeVal));
//    if(lRetVal < 0)
//    {
//       ERR_PRINT(lRetVal);
//       LOOP_FOREVER();
//    }
//
//    lRetVal = send(iSocketDesc, ucData, sizeof(ucData), 0);
//    if(lRetVal < 0)
//	{
//		DEBG_PRINT("Failed to send data \n\r",lRetVal);
//		goto end;
//	}
//    DEBG_PRINT("\nTest send() return %ld\n\r", lRetVal);
//
//    // Initialization of accelerometer/Magnetometer
//    verifyAccelMagnSensor();
//    configureFXOS8700(0);
//
//    // Initialization of Temperature and RH Sensor
//    softResetTempRHSensor();
//    configureTempRHSensor();
//
//    while(1)
//    {
//    	//if(GPIOPinRead(GPIOA2_BASE,0x40))
//    	{
//    		DEBG_PRINT("\n\rCollecting Sensor data\n\r");
//
//    		DEBG_PRINT("\n\rTemperature RH Sensor\n\r");
//    		float_t fTemp, fRH;
//    		softResetTempRHSensor();
// 		    configureTempRHSensor();
//    		getTempRH( &fTemp, &fRH );
//    		DEBG_PRINT("Temp: %f\n RH: %f\n", fTemp, fRH);
//
//    		DEBG_PRINT("\n\r\n\rAcceleration, Magnetic Fld Sensor\n\r");
//    		float_t fAccel;
//    		getAccelerationMagnitude( &fAccel );
//    		DEBG_PRINT( "\nAcceleration: %f\n\r", fAccel);
//
//    		DEBG_PRINT("\n\rLight Sensor\n\r");
//    		uint16_t usLightIntensity;
//    		float_t fLightIntensity;
//    		verifyISL29035();
//    		configureISL29035(0);
//    		usLightIntensity = getLightsensor_data();
//    		fLightIntensity = (float)usLightIntensity;
//    		DEBG_PRINT( "\nLight Intensity: %d\n\r", usLightIntensity);
//    		DEBG_PRINT( "\nLight Intensity: %f\n\r", fLightIntensity);
//
////    		pvTcpData = ucData;
////    		*(float_t*)pvTcpData = fTemp;
////    		pvTcpData += sizeof(float_t);
////    		*(float_t*)pvTcpData = fRH;
////    		pvTcpData += sizeof(float_t);
////    		*(float_t*)pvTcpData = fAccel;
////    		pvTcpData += sizeof(float_t);
//
//    		pfTcpData = fTcpData;
//    		fTcpData[0] = fTemp;
//    		fTcpData[1] = fRH;
//    		fTcpData[2] = fAccel;
//    		fTcpData[3] = fLightIntensity;
//
//    		DEBG_PRINT("Sending Sensor data\n\r");
//    		lRetVal = send(iSocketDesc,
////    						ucData,
//    						(char* )pfTcpData,
//    					//	((char*)pvTcpData - ucData),
//    						(sizeof(fTcpData)/* * sizeof(float)*/),
//    						0);
//			if(lRetVal < 0)
//			{
//				DEBG_PRINT("Failed to send data \n\r",lRetVal);
//				goto end;
//			}
//			DEBG_PRINT("Sent Sensor data\n\r");
//
//    		UtilsDelay(2*8000000/3);
//    	}
//    }
//
//    //
//    // Close the socket
//    //
//  //  lRetVal = close(iSocketDesc);
//    if(lRetVal < 0)
//    {
//        ERR_PRINT(lRetVal);
//        LOOP_FOREVER();
//    }
//    DBG_PRINT("Socket closed\n\r");
//
//end:
//    //
//    // Stop the driver
//    //
//    lRetVal = Network_IF_DeInitDriver();
//    if(lRetVal < 0)
//    {
//       DEBG_PRINT("Failed to stop SimpleLink Device\n\r");
//       LOOP_FOREVER();
//    }
//
//    //
//    // Loop here
//    //
//    LOOP_FOREVER();
//}

void TestFxosApiReadWrite(void *pvParameters)
{
	float_t fDoorDirectionAngle, fAccel;

	//softResetAccelMagnSensor();

	verifyAccelMagnSensor();

//	FXOS8700CQ_Mag_Calibration();

	configureFXOS8700(0);

	while(1)
	{
		//while(!GPIOPinRead(GPIOA0_BASE,0x80))
		{
			//DEBG_PRINT("*");
		}
		getAccelerationMagnitude( &fAccel );
		UtilsDelay(80000000 * .001);
//		DEBG_PRINT( "\nAcceleration: %f", fAccel);
//		getDoorDirection( &fDoorDirectionAngle );
//		DEBG_PRINT("\nAngle:%f\n\n", fDoorDirectionAngle);
		//UtilsDelay( 8000000/3 ); 	// Can uncomment this if data ready
									// interrupt is not set up
	}
//	if( setMotionDetectionThreshold(2) != 0 )
//	{
//		DEBG_PRINT("\nFailed\n");
//	}
}

void TestFxosMovementDetection(void *pvParameters)
{
	float_t fDoorDirectionAngle, fAccel;

	verifyAccelMagnSensor();
	configureFXOS8700(MODE_ACCEL_INTERRUPT);

	while(1)
	{
		while(GPIOPinRead(GPIOA0_BASE,0x04))
		{
		}

		DEBG_PRINT("MD	");

		clearAccelMotionIntrpt();
	}

}

void TestSi7020ApiReadWrite(void *pvParameters)
{
	float_t fTemp, fRH;

	verifyTempRHSensor();

	softResetTempRHSensor();

	configureTempRHSensor();

	while(1)
	{
		getTempRH(&fTemp, &fRH);
		DEBG_PRINT("\nTemp: %2.2f\n RH: %2.2f\n", fTemp, fRH);
		//UtilsDelay(8000000*10);
	}
}


void TestIsl29035Api(void *pvParameters)
{
	if(!verifyISL29035())
	{
		DEBG_PRINT("Device Connection success");
	}
	configureISL29035(0);

	verifyISL29035();
	configureISL29035(0);
	setThreshold_lightsensor(0x0000, 0x8000);
	 while(1)
	   {
		   if(GPIOPinRead(GPIOA0_BASE,0x04))
		   {
			   DEBG_PRINT("Low Lux");
			   getLightsensor_data();
		   }
		   else
		   {
			   DEBG_PRINT("Hi Lux");
			   getLightsensor_intrptStatus();
			   getLightsensor_data();
		   }
		   UtilsDelay(8000000);
	   }

//	while(1)
//	{
//		usData = getLightsensor_data();
//		DEBG_PRINT("Light Sensor Data: %d", usData);
//		UtilsDelay(8000000/3);
//	}
}


/*
 	//Test Logic
 	//fxos_waitFor40Degrees(80);
	//fxos_waitFor40Degrees(90);
	//float_t fYawArray[] = {80, 60, 40, 25, 20, 15, 10, 0, 350, 340, 330, 320, 0, 20, 25, 30, 35, 40, 50, 70 };

	fxos_waitFor40Degrees(30);
	fxos_waitFor40Degrees(50);
	float_t fYawArray[] = {25, 20, 15, 10, 0, 350, 340, 330, 320, 330, 340, 350, 0, 20, 25, 30, 35, 40, 50, 70 };
	int i = 0;
	while((!flag_L)&&(i!=sizeof(fYawArray)/sizeof(float_t)))
	{
		fYaw = fYawArray[i++];
		if(!flag_U)
		{
			switch(Case_U)
			{
			case CASE_NO_360CCROSSOVER:
				if((fYaw <= fYaw_U)||(fYaw > fYaw_closedDoor))
					flag_U = 1;
				break;
			case CASE_360CCROSSOVER:
				if((fYaw <= fYaw_U)&&(fYaw > fYaw_closedDoor))
					flag_U = 1;
				break;
			}
		}
		else if(!flag_L)
		{
			switch(Case_L)
			{
			case CASE_NO_360CCROSSOVER:
				if((fYaw >= fYaw_L)&&(fYaw < fYaw_closedDoor))
					flag_L = 1;
				break;
			case CASE_360CCROSSOVER:
				if((fYaw >= fYaw_L)||(fYaw < fYaw_closedDoor))
					flag_L = 1;
				break;
			}
		}
		DEBG_PRINT("%d 	%3.2f 	%x %x\n\r", i, fYaw, flag_U, flag_L);
	}
	return;

 */
