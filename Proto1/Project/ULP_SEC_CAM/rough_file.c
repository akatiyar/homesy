/*
 * rough_file.c
 *
 *  Created on: 27-May-2015
 *      Author: Chrysolin
 */


int32_t createAndWrite_ImageHeaderFile()
{
	long lFileHandle;
	unsigned long ulToken = NULL;
	long lRetVal;
	//
	// NVMEM File Open to write to SFLASH
	//
	lRetVal = sl_FsOpen((unsigned char *)IMAGE_HEADER_FILE_NAME,//0x00212001,
						FS_MODE_OPEN_CREATE(MAX_IMAGE_HEADER_SIZE_BYTES,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						&ulToken,
						&lFileHandle);
	if(lRetVal < 0)
	{
		UART_PRINT("File Open Error: %i", lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}

	// Close the created file
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	//
	// JPEG Header - create and write to Flash
	//
	// Open the file for Write Operation
	lRetVal = sl_FsOpen((unsigned char *)IMAGE_HEADER_FILE_NAME,
						FS_MODE_OPEN_WRITE,
						&ulToken,
						&lFileHandle);
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}

	// Create Header
	memset(g_header, '\0', sizeof(g_header));
	g_header_length = CreateJpegHeader((char *)&g_header[0],
										PIXELS_IN_X_AXIS,PIXELS_IN_Y_AXIS,
										0, 0x0006,(int)IMAGE_QUANTIZ_SCALE);
	//InitializeTimer();
	//StartTimer();
	// Write Header to Flash
	lRetVal = sl_FsWrite(lFileHandle, 0, g_header, g_header_length - 1);
	//StopTimer();
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}
	UART_PRINT("Image Headr Write No of bytes: %ld\n", lRetVal);

	// Close the file post writing the image
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

