/*
 * experimentt_codes.c
 *
 *  Created on: 25-May-2015
 *      Author: Chrysolin
 */

//		sl_Start(0,0,0);
//		sl_extlib_BootImg2();
//		sl_Stop(0xFFFF);

/*	struct u64_time time_now;
	uint32_t time;
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);

	UtilsDelay(4*80000000/6);
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);

	UtilsDelay(2*80000000/6);
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);

	UtilsDelay(80000000/6);
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);

	UtilsDelay(0.5*80000000/6);
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);

	UtilsDelay(0.1*80000000/6);
	cc_rtc_get(&time_now);
	time = time_now.secs * 1000 + time_now.nsec / 1000000;
	DEBG_PRINT("%d secs and %d nsecs\n", time_now.secs, time_now.nsec);
	DEBG_PRINT("%d milli sec\n", time);*/

		/*
		Config_CameraCapture();
		Standby_ImageSensor();
		while(1)
		{
			DEBG_PRINT("T3 Hib\n\r");
			start_100mSecTimer();

			g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_HAPPENING;
			Wakeup_ImageSensor();
			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			DEBG_PRINT("wake over - %d ms\n\r", ulTimeDuration_ms);

			start_100mSecTimer();

			Start_CameraCapture();
			g_ulAppStatus = IMAGESENSOR_CAPTURECONFIGS_DONE;

			ulTimeDuration_ms = get_timeDuration();
			stop_100mSecTimer();
			DEBG_PRINT("configs over - %d ms\n\r", ulTimeDuration_ms);

			Standby_ImageSensor();
		}
		*/


	lRetVal = sl_FsDel((uint8_t *)USER_CONFIGS_FILENAME, ulToken);
	DEBG_PRINT("Del %f", lRetVal);

	lRetVal = sl_FsGetInfo((uint8_t *)USER_CONFIGS_FILENAME, ulToken, &FileInfo);
	if(SL_FS_ERR_FILE_NOT_EXISTS == lRetVal)
	{
		lRetVal = CreateFile_Flash((uint8_t *)USER_CONFIGS_FILENAME, CONTENT_LENGTH_USER_CONFIGS);
		ASSERT_ON_ERROR(lRetVal);
	}



// Slow Clk ctr working
uint64_t ullSlowCounterReturnVal1, ullSlowCounterReturnVal2;
while(1)
{
ullSlowCounterReturnVal1 = PRCMSlowClkCtrGet();
UtilsDelay(80000000/6);
ullSlowCounterReturnVal2 = PRCMSlowClkCtrGet();
DEBG_PRINT("%lld\n\r", ullSlowCounterReturnVal1);
DEBG_PRINT("%lld\n\r\n\r", ullSlowCounterReturnVal2);
}

//Reset cause get
uint32_t ulResetCause;
    ulResetCause = MAP_PRCMSysResetCauseGet();
    if(PRCM_POWER_ON == ulResetCause)
    {
    	DEBG_PRINT("Device is powering on\n\r");
    }
    else if(PRCM_LPDS_EXIT == ulResetCause)
    {
    	DEBG_PRINT("Device is exiting from LPDS.\n\r");
    }
    else if(PRCM_CORE_RESET == ulResetCause)
    {
    	DEBG_PRINT("Device is exiting soft core only reset\n\r");
    }
    else if(PRCM_MCU_RESET == ulResetCause)
    {
    	DEBG_PRINT("Device is exiting soft subsystem reset.\n\r");
    }
    else if(PRCM_WDT_RESET == ulResetCause)
    {
    	DEBG_PRINT("Device was reset by watchdog.\n\r");
    }
    else if(PRCM_SOC_RESET == ulResetCause)
    {
    	DEBG_PRINT("Device is exting SOC reset.\n\r");
    }
    else if(PRCM_HIB_EXIT == ulResetCause)
    {
    	DEBG_PRINT("Device is exiting hibernate\n\r");
    }




    uint16_t data = 0;
	verifyISL29035();
	while(1)
	{
		data = getLightsensor_data();
		DEBG_PRINT("%d\n\r",data);
		UtilsDelay(80000000/6);
	}

	uint16_t data = 0;
	verifyISL29035();
	while(1)
	{
		while(getLightsensor_data() > (0x01FF))
		{
			UtilsDelay(80000000/6);
		}
		DEBG_PRINT("******\n\r");
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

    //Profiling time taken by sl_start
    		int32_t lRetVal = -1;
    		lRetVal = sl_Start(0,0,0);
    		DEBG_PRINT("%d\n\r", lRetVal);
    		lRetVal = sl_WlanSetMode(ROLE_STA);
    		DEBG_PRINT("%d\n\r", lRetVal);
    		//_u16 usPMPolicy[] = {0,0,2000,0};
    		InitializeTimer();
    		StartTimer();
    		//sl_WlanPolicySet(SL_POLICY_PM, SL_LONG_SLEEP_INTERVAL_POLICY,
    		//					usPMPolicy, sizeof(usPMPolicy));
    		lRetVal = sl_WlanPolicySet(SL_POLICY_PM, SL_LOW_POWER_POLICY,
    									NULL, 0);

    		//DEBG_PRINT("III\n\r");
    		StopTimer();
    	    float_t fPicCaptureDuration;
    	    GetTimeDuration(&fPicCaptureDuration);
    	    DEBG_PRINT("T takes: %fms\n\r", fPicCaptureDuration);
    	    sl_Stop(0xFFFF);
    	    DEBG_PRINT("%d\n\r", lRetVal);



    	    /*
    	        sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);

    	        lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
    	        					FS_MODE_OPEN_READ,
    	        					&ulToken,
    	        					&lFileHandle);
    	    	 if(lRetVal < 0)
    	    	 {
    	    		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    	    		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    	    	 }
    	    	 lRetVal = sl_FsRead(lFileHandle, 0, (unsigned char*)g_image_buffer , 500);
    	    	 if (lRetVal < 0)
    	    	 {
    	    		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    	    		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    	    	 }
    	    	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    	    	ASSERT_ON_ERROR(lRetVal);
    	    */

//**************************************************************************************************
    //	//
    //	// Initialize camera controller
    //	//
    //	CamControllerInit();
    //
    //	//
    //	// Configure DMA in ping-pong mode
    //	//
    //	DMAConfig();
    //
    //	//
    //	// Perform Image Capture
    //	//
    //	DEBG_PRINT("sB");
    //	MAP_CameraCaptureStart(CAMERA_BASE);
    //	// HWREG(0x4402609C) |= 1 << 8;
    //
    //    while(g_frame_end == 0);
    //
    //    MAP_CameraCaptureStop(CAMERA_BASE, true);
    //    DEBG_PRINT("pA");
    //
    //
    //    lRetVal = sl_Start(0, 0, 0);
    //    ASSERT_ON_ERROR(lRetVal);
    //
    //   /*
    //   	sl_FsDel((unsigned char *)USER_FILE_NAME, ulToken);
    //       //
    //       // Error handling if File Operation fails
    //       //
    //       if(lRetVal < 0)
    //       {
    //           lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //           ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //       }
    //   */
    //
    //   //
    //   // NVMEM File Open to write to SFLASH
    //   //
    //   lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,//0x00212001,
    //					FS_MODE_OPEN_CREATE(MAX_IMAGE_SIZE_BYTES,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
    //					   &ulToken,
    //					   &lFileHandle);
    //
    //   //
    //   // Error handling if File Operation fails
    //   //
    //   if(lRetVal < 0)
    //   {
    //	DEBG_PRINT("File Open Error: %i", lRetVal);
    //	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //	   ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //   }
    //   //
    //   // Close the created file
    //   //
    //   lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //   ASSERT_ON_ERROR(lRetVal);
    //
    //   /*
    //       SlFsFileInfo_t fileInfo;
    //       sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);
    //   */
    //
    //       //
    //       // Open the file for Write Operation
    //       //
    //       lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
    //                           FS_MODE_OPEN_WRITE,
    //                           &ulToken,
    //                           &lFileHandle);
    //       //
    //       // Error handling if File Operation fails
    //       //
    //       if(lRetVal < 0)
    //       {
    //           lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //           ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //       }
    //       //
    //       // Create JPEG Header
    //       //
    //   #ifdef ENABLE_JPEG
    //       memset(g_header, '\0', sizeof(g_header));
    //       g_header_length = CreateJpegHeader((char *)&g_header[0], PIXELS_IN_X_AXIS,
    //                                          PIXELS_IN_Y_AXIS, 0, 0x0006,32);
    //
    //       //
    //       // Write the Header
    //       //
    //       p_header = (unsigned char *)&g_header[0];
    //       lRetVal = sl_FsWrite(lFileHandle, 0, p_header, g_header_length);
    //       //
    //       // Error handling if file operation fails
    //       //
    //       if(lRetVal < 0)
    //       {
    //           lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //           ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //       }
    //   #endif
    //       //
    //       // Write the Image Buffer
    //       //
    //   #ifdef ENABLE_JPEG
    //       lRetVal =  sl_FsWrite(lFileHandle, g_header_length,
    //                         (unsigned char *)g_image_buffer, g_frame_size_in_bytes);
    //   #else
    //       lRetVal =  sl_FsWrite(lFileHandle, 0,
    //                             (unsigned char *)g_image_buffer, g_frame_size_in_bytes);
    //   #endif
    //       //
    //       // Error handling if file operation fails
    //       //
    //       if (lRetVal <0)
    //       {
    //           lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //           ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //       }
    //       DEBG_PRINT("Image Write No of bytes: %ld\n", lRetVal);
    //
    //       //
    //       // Close the file post writing the image
    //       //
    //       lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //       ASSERT_ON_ERROR(lRetVal);
    //
    //   /*
    //       sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);
    //
    //       lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,
    //       					FS_MODE_OPEN_READ,
    //       					&ulToken,
    //       					&lFileHandle);
    //   	 if(lRetVal < 0)
    //   	 {
    //   		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //   		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //   	 }
    //   	 lRetVal = sl_FsRead(lFileHandle, 0, (unsigned char*)g_image_buffer , 500);
    //   	 if (lRetVal < 0)
    //   	 {
    //   		 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //   		 ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
    //   	 }
    //   	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    //   	ASSERT_ON_ERROR(lRetVal);
    //   */
    //
    //       lRetVal = sl_Stop(0xFFFF);
    //       //lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    //   	ASSERT_ON_ERROR(lRetVal);
    //
    //   	DEBG_PRINT("DONE: Image Write to Flash\n\r");
//**************************************************************************************************
