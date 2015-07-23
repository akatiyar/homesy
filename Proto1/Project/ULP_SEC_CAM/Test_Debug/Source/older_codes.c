/*
 * older_codes.c
 *
 *  Created on: 16-Jul-2015
 *      Author: Chrysolin
 */

//void TimerBaseIntHandler(void)
//{
//    Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);
//
//    v_TimerOverflows++;
//
//    if( v_TimerOverflows >= (10 * 80000000 / 65536) )
//    {
//    	v_OneSecFlag = 1;
//    }
//}
//
//int32_t InitializeTimer()
//{
//	Timer_IF_Init(PRCM_TIMERA0, APP_PROFILING_TIMER_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
//
//	Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);
//	//Timer_IF_IntSetup(APP_PROFILING_TIMER_BASE, TIMER_A, TimerBaseIntHandler);
//
//	Timer_IF_InterruptClear(APP_PROFILING_TIMER_BASE);
//
//	v_TimerOverflows = 0;
//	v_OneSecFlag = 0;
//	return 0;
//}
//
//int32_t StartTimer()
//{
//	MAP_TimerLoadSet(APP_PROFILING_TIMER_BASE, TIMER_A, 0xFFFF);
//	//
//	// Enable the GPT
//	//
//	MAP_TimerEnable(APP_PROFILING_TIMER_BASE, TIMER_A);
//
//	return 0;
//}
//
//int32_t StopTimer()
//{
//	MAP_TimerDisable(APP_PROFILING_TIMER_BASE, TIMER_A);
//
//	return 0;
//}
//
//int32_t GetTimeDuration(float* pfDurationMilli)
//{
//	uint16_t ulCounter;
//
//	ulCounter = MAP_TimerValueGet(APP_PROFILING_TIMER_BASE, TIMER_A);
//	ulCounter = 0xFFFFFFFF - ulCounter;
//	//*pfDurationMilli = /*(v_TimerOverflows * 4294967296.0 / 80000.0) +*/ ((float_t)ulCounter / 80000.0); //in milli sec
//	*pfDurationMilli = (v_TimerOverflows * 65536 / 80000.0) + ((float_t)ulCounter / 80000.0); //in milli sec
//
//	return 0;
//}
//

//*****************************************************************************
//
//!    ConnectToNetwork
//!    Setup SimpleLink in AP Mode
//!
//!    \param                      None
//!     \return                     0 - Success
//!                                   Negative - Failure
//!
//
//*****************************************************************************
long ConnectToNetwork()
{
    long lRetVal = -1;
    unsigned char ucAPSSID[AP_SSID_LEN_MAX];
        unsigned short len = 32;
        unsigned short  config_opt = WLAN_AP_OPT_SSID;
    //Start Simplelink Device
    lRetVal =  sl_Start(NULL,NULL,NULL);
    ASSERT_ON_ERROR(lRetVal);

    // Device is in STA mode, Switch to AP Mode
    if(lRetVal == ROLE_STA)
    {
        //
        // Configure to AP Mode
        //
        if(ConfigureMode(ROLE_AP) !=ROLE_AP)
        {
            UART_PRINT("Unable to set AP mode...\n\r");
            lRetVal = sl_Stop(SL_STOP_TIMEOUT);
            CLR_STATUS_BIT_ALL(g_ulStatus);
            ASSERT_ON_ERROR(DEVICE_NOT_IN_AP_MODE);
        }
    }

    while(!IS_IP_ACQUIRED(g_ulStatus))
    {
      //looping till ip is acquired
    }

     //Read the AP SSID

    memset(ucAPSSID,'\0',AP_SSID_LEN_MAX);
    lRetVal = sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, (unsigned char*) ucAPSSID);
    ASSERT_ON_ERROR(lRetVal);

    //Stop Internal HTTP Server
    lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
    ASSERT_ON_ERROR(lRetVal);

    //Start Internal HTTP Server
    lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
    ASSERT_ON_ERROR(lRetVal);
    return SUCCESS;
}

//long ConnectToNetwork_2()
//{
//    long lRetVal = -1;
//	unsigned int uiConnectTimeoutCnt =0;
//    //unsigned int uiConnectTimeoutCnt =0; //uncomment if wifi details are not hard-coded
//
//    //Start Simplelink Device
//    lRetVal =  sl_Start(NULL,NULL,NULL);
//    ASSERT_ON_ERROR(lRetVal);
//
//    lRetVal = sl_WlanSetMode(ROLE_STA);
//	if (lRetVal < 0) {
//		sl_Stop(SL_STOP_TIMEOUT);
//		ERR_PRINT(lRetVal);
//		LOOP_FOREVER();
//	}
//
//		SlSecParams_t secParams = {0};
//		lRetVal = 0;
//
//		secParams.Key = (signed char*)SECURITY_KEY;
//		secParams.KeyLen = strlen(SECURITY_KEY);
//		secParams.Type = SECURITY_TYPE;
//
//		lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
//		ASSERT_ON_ERROR(lRetVal);
//
//		/*// Wait for WLAN Event
//		while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
//		{
//#ifdef LED_INDICATION
//			// Toggle LEDs to Indicate Connection Progress
//			GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
//			MAP_UtilsDelay(800000);
//			GPIO_IF_LedOn(MCU_IP_ALLOC_IND);
//			MAP_UtilsDelay(800000);
//#endif
//		}*/
//
//
//	    // Wait for WLAN Event
//	    while(uiConnectTimeoutCnt<CONNECTION_TIMEOUT_COUNT &&
//			((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus))))
//	    {
//	        osi_Sleep(1); //Sleep 1 millisecond
//	        uiConnectTimeoutCnt++;
//	    }
//
//		return SUCCESS;
//}
//
//void ConnectToNetwork_STA_2()
//{
//	 long lRetVal = -1;
//
//	//Initialize Global Variable
//	InitializeAppVariables();
//
//	ConfigureSimpleLinkToDefaultState();
//
//	//Connect to Network
//	lRetVal = ConnectToNetwork_2();
//	if(lRetVal < 0)
//	{
//		UART_PRINT("Failed to establish connection w/ an AP \n\r");
//		LOOP_FOREVER();
//	}
//}


//int32_t create_AngleValuesFile()
//{
//	long lFileHandle;
//	unsigned long ulToken = NULL;
//	long lRetVal;
//
//	lRetVal = sl_Start(0,0,0);
//	ASSERT_ON_ERROR(lRetVal);
//
//	//
//	// NVMEM File Open to write to SFLASH
//	//
//	lRetVal = sl_FsOpen((unsigned char *)USER_CONFIGS_FILENAME,//0x00212001,
//						FS_MODE_OPEN_CREATE(CONTENT_LENGTH_USER_CONFIGS,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
//						&ulToken,
//						&lFileHandle);
//	if(lRetVal < 0)
//	{
//		UART_PRINT("File Open Error: %i", lRetVal);
//		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
//		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
//	}
//
//	// Close the created file
//	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
//	ASSERT_ON_ERROR(lRetVal);
//
//	lRetVal = sl_Stop(0xFFFF);
//	ASSERT_ON_ERROR(lRetVal);
//
//	return lRetVal;
//}

int32_t createAndWrite_ImageHeaderFile()
{
	long lFileHandle;
	unsigned long ulToken = NULL;
	long lRetVal;

	sl_Start(0,0,0);

	//
	// NVMEM File Open to write to SFLASH
	//
	lRetVal = sl_FsOpen((unsigned char *)JPEG_HEADER_FILE_NAME,//0x00212001,
						FS_MODE_OPEN_CREATE(JPEG_HEADER_MAX_FILESIZE,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
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
	lRetVal = sl_FsOpen((unsigned char *)JPEG_HEADER_FILE_NAME,
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
	lRetVal = sl_FsWrite(lFileHandle, 0, (unsigned char*)g_header, g_header_length - 1);
	//StopTimer();
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(CAMERA_CAPTURE_FAILED);
	}
	//UART_PRINT("Image Headr Write No of bytes: %ld\n", lRetVal);

	// Close the file post writing the image
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	SlFsFileInfo_t fileInfo;
	sl_FsGetInfo((unsigned char *)JPEG_HEADER_FILE_NAME, ulToken, &fileInfo);

	sl_Stop(0xFFFF);

	return lRetVal;
}

int32_t create_JpegImageFile()
{
	long lFileHandle;
	unsigned long ulToken = NULL;
	long lRetVal;

	lRetVal = sl_Start(0,0,0);
	ASSERT_ON_ERROR(lRetVal);

	//
	// NVMEM File Open to write to SFLASH
	//
	lRetVal = sl_FsOpen((unsigned char *)JPEG_IMAGE_FILE_NAME,//0x00212001,
						FS_MODE_OPEN_CREATE(JPEG_IMAGE_MAX_FILESIZE,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
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

	lRetVal = sl_Stop(0xFFFF);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}


int32_t Write_JPEGHeader()
{
	long lFileHandle;
	unsigned long ulToken = NULL;
	long lRetVal;

	// Create Header
	memset(g_header, '\0', sizeof(g_header));
	g_header_length = CreateJpegHeader((char *)&g_header[0],
										PIXELS_IN_X_AXIS,PIXELS_IN_Y_AXIS,
										0, 0x0006,(int)IMAGE_QUANTIZ_SCALE);

	lRetVal = sl_Start(0,0,0);
	ASSERT_ON_ERROR(lRetVal);

	// Open the file for Write Operation
	lRetVal = sl_FsOpen((unsigned char *)JPEG_HEADER_FILE_NAME,
						FS_MODE_OPEN_WRITE,
						&ulToken,
						&lFileHandle);
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	}

	// Write
	lRetVal = sl_FsWrite(lFileHandle, 0, (unsigned char*)g_header, g_header_length - 1);
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	}

	// Close the file post writing the image
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	lRetVal = sl_Stop(0xFFFF);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

//*****************************************************************************
//
//!     JfifApp0Marker
//!
//!    \param                      Pointer to the output buffer
//!     \return                     Length of the Marker or error code
//
//*****************************************************************************

#ifdef ENABLE_JPEG
static int JfifApp0Marker(char *pbuf)
{
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    *pbuf++= 0xFF;                  // APP0 marker
    *pbuf++= 0xE0;
    *pbuf++= 0x00;                  // length
    *pbuf++= 0x10;
    *pbuf++= 0x4A;                  // JFIF identifier
    *pbuf++= 0x46;
    *pbuf++= 0x49;
    *pbuf++= 0x46;
    *pbuf++= 0x00;
    *pbuf++= 0x01;                  // version
    *pbuf++= 0x02;
    *pbuf++= 0x00;                  // units
    *pbuf++= 0x00;                  // X density
    *pbuf++= 0x01;
    *pbuf++= 0x00;                  // Y density
    *pbuf++= 0x01;
    *pbuf++= 0x00;                  // X thumbnail
    *pbuf++= 0x00;                  // Y thumbnail
    return 18;
}


//*****************************************************************************
//
//!    FrameHeaderMarker
//!
//!    \param1                      pointer to the output buffer
//!    \param2                      width
//!    \param3                      height
//!    \param4                      format
//!
//!     \return                       Length of the header marker or error code
//
//*****************************************************************************
static int FrameHeaderMarker(char *pbuf, int width, int height, int format)
{
    int length;
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer");
        LOOP_FOREVER();
    }
    if (format == FORMAT_MONOCHROME)
        length = 11;
    else
        length = 17;

    *pbuf++= 0xFF;                      // start of frame: baseline DCT
    *pbuf++= 0xC0;
    *pbuf++= length>>8;                 // length field
    *pbuf++= length&0xFF;
    *pbuf++= 0x08;                      // sample precision
    *pbuf++= height>>8;                 // number of lines
    *pbuf++= height&0xFF;
    *pbuf++= width>>8;                  // number of samples per line
    *pbuf++= width&0xFF;

    if (format == FORMAT_MONOCHROME)    // monochrome
    {
        *pbuf++= 0x01;              // number of image components in frame
        *pbuf++= 0x00;              // component identifier: Y
        *pbuf++= 0x11;              // horizontal | vertical sampling factor: Y
        *pbuf++= 0x00;              // quantization table selector: Y
    }
    else if (format == FORMAT_YCBCR422) // YCbCr422
    {
        *pbuf++= 0x03;        // number of image components in frame
        *pbuf++= 0x00;        // component identifier: Y
        *pbuf++= 0x21;        // horizontal | vertical sampling factor: Y
        *pbuf++= 0x00;        // quantization table selector: Y
        *pbuf++= 0x01;        // component identifier: Cb
        *pbuf++= 0x11;        // horizontal | vertical sampling factor: Cb
        *pbuf++= 0x01;        // quantization table selector: Cb
        *pbuf++= 0x02;        // component identifier: Cr
        *pbuf++= 0x11;        // horizontal | vertical sampling factor: Cr
        *pbuf++= 0x01;        // quantization table selector: Cr
    }
    else                                // YCbCr420
    {
        *pbuf++= 0x03;         // number of image components in frame
        *pbuf++= 0x00;         // component identifier: Y
        *pbuf++= 0x22;         // horizontal | vertical sampling factor: Y
        *pbuf++= 0x00;         // quantization table selector: Y
        *pbuf++= 0x01;         // component identifier: Cb
        *pbuf++= 0x11;         // horizontal | vertical sampling factor: Cb
        *pbuf++= 0x01;         // quantization table selector: Cb
        *pbuf++= 0x02;         // component identifier: Cr
        *pbuf++= 0x11;         // horizontal | vertical sampling factor: Cr
        *pbuf++= 0x01;        // quantization table selector: Cr
    }

    return (length+2);
}


//*****************************************************************************
//
//!     ScanHeaderMarker
//!
//!    \param1                     pointer to output buffer
//!    \param2                     Format
//!
//!     \return                     Length or error code
//
//*****************************************************************************
static int ScanHeaderMarker(char *pbuf, int format)
{
    int length;

    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    if (format == FORMAT_MONOCHROME)
        length = 8;
    else
        length = 12;

    *pbuf++= 0xFF;                  // start of scan
    *pbuf++= 0xDA;
    *pbuf++= length>>8;             // length field
    *pbuf++= length&0xFF;
    if (format == FORMAT_MONOCHROME)// monochrome
    {
        *pbuf++= 0x01;              // number of image components in scan
        *pbuf++= 0x00;              // scan component selector: Y
        *pbuf++= 0x00;              // DC | AC huffman table selector: Y
    }
    else                            // YCbCr
    {
        *pbuf++= 0x03;              // number of image components in scan
        *pbuf++= 0x00;              // scan component selector: Y
        *pbuf++= 0x00;              // DC | AC huffman table selector: Y
        *pbuf++= 0x01;              // scan component selector: Cb
        *pbuf++= 0x11;              // DC | AC huffman table selector: Cb
        *pbuf++= 0x02;              // scan component selector: Cr
        *pbuf++= 0x11;              // DC | AC huffman table selector: Cr
    }

    *pbuf++= 0x00;         // Ss: start of predictor selector
    *pbuf++= 0x3F;         // Se: end of spectral selector
    *pbuf++= 0x00;         // Ah | Al: successive approximation bit position

    return (length+2);
}


//*****************************************************************************
//
//!     DefineQuantizationTableMarker
//!      Calculate and write the quantisation tables
//! qscale is the customised scaling factor  see MT9D131 developer guide page 78
//!
//!    \param1                      pointer to the output buffer
//!    \param2                      Quantization Scale
//!    \param3                      Format
//!
//!     \return                      Length of the Marker or error code
//
//*****************************************************************************
static int DefineQuantizationTableMarker (unsigned char *pbuf, int qscale, int format)
{
    int i, length, temp;
    // temp array to store scaled zigzagged quant entries
    unsigned char newtbl[64];

    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer");
        LOOP_FOREVER();
    }

    if (format == FORMAT_MONOCHROME)    // monochrome
        length  =  67;
    else
        length  =  132;

    *pbuf++  =  0xFF;                   // define quantization table marker
    *pbuf++  =  0xDB;
    *pbuf++  =  length>>8;              // length field
    *pbuf++  =  length&0xFF;
     // quantization table precision | identifier for luminance
    *pbuf++  =  0;

    // calculate scaled zigzagged luminance quantisation table entries
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblY[i] * qscale + 16) / 32;
        // limit the values to the valid range
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting luminance quant table to the output buffer
    for (i=0; i<64; i++)
        *pbuf++ = newtbl[i];

    // if format is monochrome we're finished,
    // otherwise continue on, to do chrominance quant table
    if (format == FORMAT_MONOCHROME)
        return (length+2);

    *pbuf++ = 1;   // quantization table precision | identifier for chrominance

    // calculate scaled zigzagged chrominance quantisation table entries
    for (i=0; i<64; i++) {
        temp = (JPEG_StdQuantTblC[i] * qscale + 16) / 32;
        // limit the values to the valid range
        if (temp <= 0)
            temp = 1;
        if (temp > 255)
            temp = 255;
        newtbl[zigzag[i]] = (unsigned char) temp;
    }

    // write the resulting chrominance quant table to the output buffer
    for (i=0; i<64; i++)
        *pbuf++ = newtbl[i];

    return (length+2);
}


//*****************************************************************************
//
//!     DefineHuffmanTableMarkerDC
//!
//!    \param1                      pointer to Marker buffer
//!    \param2                      Huffman table
//!    \param3                      Class Identifier
//!
//!     \return                      Length of the marker or error code
//
//*****************************************************************************
static int DefineHuffmanTableMarkerDC(char *pbuf, unsigned int *htable,
                                                                int class_id)
{
    int i, l, count;
    int length;
    char *plength;

    if((pbuf == NULL) || (htable == NULL))
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }
    *pbuf++= 0xFF;                  // define huffman table marker
    *pbuf++= 0xC4;
    plength = pbuf;                 // place holder for length field
    *pbuf++;
    *pbuf++;
    *pbuf++= class_id;              // huffman table class | identifier

    for (l = 0; l < 16; l++)
    {
        count = 0;
        for (i = 0; i < 12; i++)
        {
            if ((htable[i] >> 8) == l)
                count++;
        }
        *pbuf++= count;             // number of huffman codes of length l+1
    }

    length = 19;
    for (l = 0; l < 16; l++)
    {
        for (i = 0; i < 12; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                *pbuf++= i;         // HUFFVAL with huffman codes of length l+1
                length++;
            }
        }
    }

    *plength++= length>>8;          // length field
    *plength = length&0xFF;

    return (length + 2);
}


//*****************************************************************************
//
//!     DefineHuffmanTableMarkerAC
//!     1. Establishes connection w/ AP//
//!     2. Initializes the camera sub-components//! GPIO Enable & Configuration
//!     3. Listens and processes the image capture requests from user-applications
//!
//!    \param1                      pointer to Marker buffer
//!    \param2                      Huffman table
//!    \param3                      Class Identifier
//!
//!     \return                      Length of the Marker or error code
//!
//
//*****************************************************************************
static int DefineHuffmanTableMarkerAC(char *pbuf, unsigned int *htable,
                                                                int class_id)
{
    int i, l, a, b, count;
    char *plength;
    int length;

    if((pbuf == NULL) || (htable == NULL))
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    *pbuf++= 0xFF;                      // define huffman table marker
    *pbuf++= 0xC4;
    plength = pbuf;                     // place holder for length field
    *pbuf++;
    *pbuf++;
    *pbuf++= class_id;                  // huffman table class | identifier

    for (l = 0; l < 16; l++)
    {
        count = 0;
        for (i = 0; i < 162; i++)
        {
            if ((htable[i] >> 8) == l)
                count++;
        }

        *pbuf++= count;          // number of huffman codes of length l+1
    }

    length = 19;
    for (l = 0; l < 16; l++)
    {
        // check EOB: 0|0
        if ((htable[160] >> 8) == l)
        {
            *pbuf++= 0;         // HUFFVAL with huffman codes of length l+1
            length++;
        }

        // check HUFFVAL: 0|1 to E|A
        for (i = 0; i < 150; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                a = i/10;
                b = i%10;
                // HUFFVAL with huffman codes of length l+1
                *pbuf++= (a<<4)|(b+1);
                length++;
            }
        }

        // check ZRL: F|0
        if ((htable[161] >> 8) == l)
        {
        // HUFFVAL with huffman codes of length l+1
            *pbuf++= 0xF0;
            length++;
        }

        // check HUFFVAL: F|1 to F|A
        for (i = 150; i < 160; i++)
        {
            if ((htable[i] >> 8) == l)
            {
                a = i/10;
                b = i%10;
                 // HUFFVAL with huffman codes of length l+1
                *pbuf++= (a<<4)|(b+1);
                length++;
            }
        }
    }

    *plength++= length>>8;              // length field
    *plength = length&0xFF;
    return (length + 2);
}


//*****************************************************************************
//
//!     DefineRestartIntervalMarker
//!
//!    \param1                      pointer to Marker buffer
//!    \param2                      return interval
//!
//!     \return                      Length or error code
//
//*****************************************************************************
static int DefineRestartIntervalMarker(char *pbuf, int ri)
{
    if(pbuf == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    *pbuf++= 0xFF;                  // define restart interval marker
    *pbuf++= 0xDD;
    *pbuf++= 0x00;                  // length
    *pbuf++= 0x04;
    *pbuf++= ri >> 8;               // restart interval
    *pbuf++= ri & 0xFF;
    return 6;
}
//*****************************************************************************
//
//!     CreateJpegHeader
//!     Create JPEG Header in JFIF format
//!
//!    \param1                     header - pointer to JPEG header buffer
//!    \param2                     width - image width
//!    \param3                     height - image height
//!    \param4                     format - color format
//!                                 (0 = YCbCr422, 1 = YCbCr420, 2 = monochrome)
//!    \param5                     restart_int - restart marker interval
//!    \param6                     qscale - quantization table scaling factor
//!
//!     \return               length of JPEG header (bytes) or error code -1
//
//*****************************************************************************

static int CreateJpegHeader(char *header, int width, int height,
                            int format, int restart_int, int qscale)
{
    char *pbuf = header;
    int length;
    if(header == NULL)
    {
        UART_PRINT("Null pointer\n\r");
        LOOP_FOREVER();
    }

    // SOI
    *pbuf++= 0xFF;
    *pbuf++= 0xD8;
    length = 2;

    // JFIF APP0
    length += JfifApp0Marker(pbuf);

    // Quantization Tables
    pbuf = header + length;
    length += DefineQuantizationTableMarker
                                       ((unsigned char *)pbuf, qscale, format);

    // Frame Header
    pbuf = header + length;
    length += FrameHeaderMarker(pbuf, width, height, format);

    // Huffman Table DC 0 for Luma
    pbuf = header + length;
    length += DefineHuffmanTableMarkerDC(pbuf, &JPEG_StdHuffmanTbl[352], 0x00);

    // Huffman Table AC 0 for Luma
    pbuf = header + length;
    length += DefineHuffmanTableMarkerAC(pbuf, &JPEG_StdHuffmanTbl[0], 0x10);

    if (format != FORMAT_MONOCHROME)// YCbCr
    {
        // Huffman Table DC 1 for Chroma
        pbuf = header + length;
        length += DefineHuffmanTableMarkerDC
                                        (pbuf, &JPEG_StdHuffmanTbl[368], 0x01);

        // Huffman Table AC 1 for Chroma
        pbuf = header + length;
        length += DefineHuffmanTableMarkerAC
                                        (pbuf, &JPEG_StdHuffmanTbl[176], 0x11);
    }

    // Restart Interval
    if (restart_int > 0)
    {
        pbuf = header + length;
        length += DefineRestartIntervalMarker(pbuf, restart_int);
    }

    // Scan Header
    pbuf = header + length;
    length += ScanHeaderMarker(pbuf, format);

    return length;
}
#endif

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


//int32_t OTA_BootImg2()
//{
//	int SetCommitInt;
//	long OptionLen;
//    unsigned char OptionVal;
//
//    UART_PRINT("Entered OTA_BootImg2()\n\r");
//	sl_Start(0,0,0);
//
//	OTA_Init();
//
//	 sl_extLib_Ota_BootImg2(pvOtaApp, Option, OptionLen, pOptionVal);
//
//    sl_Stop(0xFF);
//
//
//    return 0;
//}

//int32_t OTA_Update_2()
//{
//	UART_PRINT("Entered OTA_Update_2()\n\r");
//
//	int32_t lRetVal;
//
//	//
//	//	Connect cc3200 to wifi AP
//	//
//	ConfigureSimpleLinkToDefaultState();
//	sl_Start(0, 0, 0);
//	ConnectToNetwork_STA();
//
//	//Tag:OTA
//	int SetCommitInt;
//	unsigned char ucVendorStr[20];
//	long OptionLen;
//	unsigned char OptionVal;
//
//    //
//	// Initialize OTA
//	//
//	pvOtaApp = sl_extLib_OtaInit(RUN_MODE_NONE_OS | RUN_MODE_BLOCKING,0);
//	//
//	// Initializa OTA service
//	//
//	strcpy((char *)ucVendorStr,OTA_VENDOR_STRING);
//    OTAServerInfoSet(&pvOtaApp,(char *)ucVendorStr);
//
//    //
//    //	Check if this image is booted in test mode
//    //
//    sl_extLib_OtaGet(pvOtaApp, EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT, &OptionLen,(_u8*)&OptionVal);
//    UART_PRINT("EXTLIB_OTA_GET_OPT_IS_PENDING_COMMIT? %d\n\r", OptionVal);
//    if(OptionVal == true)
//    {
//    	UART_PRINT("OTA: Pending commit and WLAN ok ==> perform commit\n\r");
//    	SetCommitInt = OTA_ACTION_IMAGE_COMMITED;
//    	sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_COMMIT, sizeof(int),(_u8 *)&SetCommitInt);
//    }
//    else
//    {
//    	UART_PRINT("Starting OTA\n\r");
//    	lRetVal = 0;
//
//    	while(!lRetVal)
//    	{
//    		lRetVal = sl_extLib_OtaRun(pvOtaApp);
//    		UART_PRINT("%d\n\r", lRetVal);
//    	}
//
//    	UART_PRINT("OTA run = %d\n\r", lRetVal);
//    	if(lRetVal < 0)
//    	{
//    		UART_PRINT("OTA:Error with OTA Server\n\r");
//    	}
//    	else if(lRetVal == RUN_STAT_NO_UPDATES)
//    	{
//    		UART_PRINT("OTA: RUN_STAT_NO_UPDATES");
//    	}
//    	else if((lRetVal & RUN_STAT_DOWNLOAD_DONE))
//    	{
//    		//
//    		//	Set OTA File for testing
//    		//
//    		lRetVal = sl_extLib_OtaSet(pvOtaApp, EXTLIB_OTA_SET_OPT_IMAGE_TEST, sizeof(int), (_u8 *)&SetCommitInt);
//    		UART_PRINT("OTA: NEW IMAGE DOWNLOAD COMPLETE\n\r");
//    		UART_PRINT("Rebooting...\n\r");
//    		RebootMCU();
//    	}
//    }
//    sl_Stop(0xFF);
//
//    return 0;
//}

//Did not work. long long UART_PRINT
//		UART_PRINT("g_TimeStamp_cc3200Up: %d milli sec\n", g_TimeStamp_cc3200Up);
//		UART_PRINT("g_TimeStamp_NWPUp : %d milli sec\n", g_TimeStamp_NWPUp);
//		UART_PRINT("g_TimeStamp_CamUp : %d milli sec\n", g_TimeStamp_CamUp);
//		UART_PRINT("g_TimeStamp_PhotoSnap : %d milli sec\n", g_TimeStamp_PhotoSnap);
//		UART_PRINT("g_TimeStamp_PhotoUploaded: %d milli sec\n", g_TimeStamp_PhotoUploaded);
//		UART_PRINT("g_TimeStamp_maxAngle: %d milli sec\n", g_TimeStamp_maxAngle);
//		UART_PRINT("g_TimeStamp_minAngle: %d milli sec\n", g_TimeStamp_minAngle);
