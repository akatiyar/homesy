#include "flash_files.h"

#include "fs.h"
#include "app.h"

//******************************************************************************
//	Creates a file in flash
//
//	param[in]	pucFileName - pointer to the File Name character array
//	param[in]	uiMaxFileSize - maximum file size
//	param[out]
//
//	return SUCCESS or failure code
//
//
//******************************************************************************
int32_t CreateFile_Flash(uint8_t* pucFileName, uint32_t uiMaxFileSize)
{
	int32_t lRetVal;
	int32_t lFileHandle;
	uint32_t ulToken = NULL;

	lRetVal = sl_FsOpen(pucFileName,
	    					FS_MODE_OPEN_CREATE(uiMaxFileSize,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
	                        &ulToken,
	                        &lFileHandle);
	//
	// Error handling if File Operation fails
	//
	if(lRetVal < 0)
	{
		ASSERT_ON_ERROR(lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	}
	//
	// Close the created file
	//
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//	Writes data into an existing file in flash
//
//	param[in]	pucData - pointer to the data array to be written into file
//	param[in]	pucFileName - pointer to the File Name character array
//	param[in]	uiDataSize - size of data to be written, in bytes
//	param[in]	uiOffsetInFile - offset as to where to write within the file
//
//	return SUCCESS or failure code
//
//******************************************************************************
int32_t WriteFile_ToFlash(uint8_t* pucData,
			uint8_t* pucFileName,
			uint32_t uiDataSize,
			uint32_t uiOffsetInFile)
{
	int32_t lRetVal;
	int32_t lFileHandle;
	uint32_t ulToken = NULL;

	//
	// Open the file for Write Operation
	//
	lRetVal = sl_FsOpen(pucFileName,
						FS_MODE_OPEN_WRITE,
						&ulToken,
						&lFileHandle);
	//
	// Error handling if File Operation fails
	//
	if(lRetVal < 0)
	{
		ASSERT_ON_ERROR(lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	}
    //
    // Write the Image Buffer
    //
    lRetVal =  sl_FsWrite(lFileHandle, uiOffsetInFile, pucData, uiDataSize);
    //
    // Error handling if file operation fails
    //
    if (lRetVal <0)
    {
    	ASSERT_ON_ERROR(lRetVal);
        lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);
    }
    //
    // Close the file post writing the image
    //
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    ASSERT_ON_ERROR(lRetVal);

    return lRetVal;
}

//******************************************************************************
//	Writes data into an existing file in flash
//
//	param[in]	pucData - pointer to the data array into which data read from
//							file is stored
//	param[in]	pucFileName - pointer to the File Name character array
//	param[in]	uiDataSize - number of bytes to be read
//	param[in]	uiOffsetInFile - offset as to where to read from in the file
//
//	return SUCCESS or failure code
//
//******************************************************************************
int32_t ReadFile_FromFlash(uint8_t* pucData,
			uint8_t* pucFileName,
			uint32_t uiDataSize,
			uint32_t uiOffsetInFile)
{
	int32_t lRetVal;
	int32_t lFileHandle;
	uint32_t ulToken = NULL;

	lRetVal = sl_FsOpen(pucFileName,
						FS_MODE_OPEN_READ,
						&ulToken,
						&lFileHandle);
	 if(lRetVal < 0)
	 {
		ASSERT_ON_ERROR(lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	 }

	 lRetVal = sl_FsRead(lFileHandle, uiOffsetInFile, pucData , uiDataSize);
	 if (lRetVal < 0)
	 {
		ASSERT_ON_ERROR(lRetVal);
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		ASSERT_ON_ERROR(lRetVal);
	 }

	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	ASSERT_ON_ERROR(lRetVal);

	return lRetVal;
}
