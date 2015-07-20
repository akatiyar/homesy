/*
 * codes_usedfor_debugging.c
 *
 *  Created on: 16-Jul-2015
 *      Author: Chrysolin
 */




long CaptureinRAM()
{
	// Initialize camera controller
	//
	CamControllerInit();

	//
	// Configure DMA in ping-pong mode
	//
	DMAConfig();

	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, GPIO_PIN_1);	//LED on
	//WaitFor40Degrees();
	MAP_GPIOPinWrite(GPIOA1_BASE, GPIO_PIN_1, 0);			//LED off

	//
	// Perform Image Capture
	//
	UART_PRINT("sB");
	MAP_CameraCaptureStart(CAMERA_BASE);
	// HWREG(0x4402609C) |= 1 << 8;
	while(g_frame_end == 0);
	MAP_CameraCaptureStop(CAMERA_BASE, true);
	UART_PRINT("pA");

	return 0;
}

long CaptureinRAM_StoreAfterCapture_Image()
{
    long lFileHandle;
    unsigned long ulToken = NULL;
    long lRetVal;

	// Initialize camera controller
	//
	CamControllerInit();

	//
	// Configure DMA in ping-pong mode
	//
	DMAConfig();

	LED_On();
	//Wait for angle condition
	LED_Off();

	//
	// Perform Image Capture
	//
	UART_PRINT("sB");
	MAP_CameraCaptureStart(CAMERA_BASE);
	// HWREG(0x4402609C) |= 1 << 8;
	while(g_frame_end == 0);
	MAP_CameraCaptureStop(CAMERA_BASE, true);
	UART_PRINT("pA");

	lRetVal = sl_Start(0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	//
	// Open the file for Write Operation
	//
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,
					   FS_MODE_OPEN_WRITE,
					   &ulToken,
					   &lFileHandle);
	if(lRetVal < 0)
	{
	   lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	   ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}

	//
	// Write the Image Buffer
	//
	lRetVal =  sl_FsWrite(lFileHandle, 0,
					 (unsigned char *)g_image_buffer, g_frame_size_in_bytes);
	//
	// Error handling if file operation fails
	//
	if (lRetVal <0)
	{
	   lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	   ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}
	UART_PRINT("Image Write No of bytes: %ld\n", lRetVal);

	//
	// Close the file post writing the image
	//
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = sl_Stop(0xFFFF);
	//lRetVal = sl_Stop(SL_STOP_TIMEOUT);
	ASSERT_ON_ERROR(lRetVal);

	UART_PRINT("DONE: Image Write to Flash\n\r");

	return lRetVal;
}
