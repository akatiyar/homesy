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
	    					(_u32*)&ulToken,
	                        (_i32*)&lFileHandle);
	//
	// Error handling if File Operation fails
	//
	if(lRetVal < 0)
	{
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		RETURN_ON_ERROR(lRetVal);
	}
	//
	// Close the created file
	//
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	RETURN_ON_ERROR(lRetVal);

	return lRetVal;
}

//******************************************************************************
//	Writes data into an existing file in flash
//
//	param[in]	pucData - pointer to the data array to be written into file
//	param[in]	pucFileName - pointer to the File Name character array
//	param[in]	uiDataSize - size of data to be written, in bytes
//	param[in]	uiOffsetInFile - offset as to where to write within the file
//	param[in]	ucWriteType - Whether the file has to be opened or is already open,
//							and whether to close the file. Use one of the Macros
//	param[in/out] plFileHandle - in for middle and last file writes and out for
//									first and single file writes
//
//	return SUCCESS or number of bytes written
//
//******************************************************************************
int32_t WriteFile_ToFlash(uint8_t* pucData,
			uint8_t* pucFileName,
			uint32_t uiDataSize,
			uint32_t uiOffsetInFile,
			uint8_t ucWriteType,
			int32_t* plFileHandle)
{
	int32_t lRetVal;
	int32_t lFileHandle;
	uint32_t ulToken = NULL;
	uint32_t uiNumBytesWritten;

	if((MULTIPLE_WRITE_FIRST == ucWriteType)||(SINGLE_WRITE == ucWriteType))
	{
		//
		// Open the file for Write Operation
		//
		lRetVal = sl_FsOpen(pucFileName,
							FS_MODE_OPEN_WRITE,
							(_u32*)&ulToken,
							(_i32*)&lFileHandle);
		//
		// Error handling if File Operation fails
		//
		if(lRetVal < 0)
		{
			sl_FsClose(lFileHandle, 0, 0, 0);
			RETURN_ON_ERROR(lRetVal);
		}

		*plFileHandle = lFileHandle;
	}
	else
	{
		lFileHandle = *plFileHandle;
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
        sl_FsClose(lFileHandle, 0, 0, 0);
        RETURN_ON_ERROR(lRetVal);
    }
    else
    {
    	uiNumBytesWritten = lRetVal;
    }

    if((MULTIPLE_WRITE_LAST == ucWriteType)||(SINGLE_WRITE == ucWriteType))
    {
		//
		// Close the file post writing the image
		//
		lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
		RETURN_ON_ERROR(lRetVal);
    }

    return uiNumBytesWritten;
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
//	return no of bytes read or failure code
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
	uint32_t uiNumBytesRead;

	lRetVal = sl_FsOpen(pucFileName,
						FS_MODE_OPEN_READ,
						(_u32*)&ulToken,
						(_i32*)&lFileHandle);
	 if(lRetVal < 0)
	 {
		sl_FsClose(lFileHandle, 0, 0, 0);
		RETURN_ON_ERROR(lRetVal);
	 }

	 lRetVal = sl_FsRead(lFileHandle, uiOffsetInFile, pucData, uiDataSize);
	 if (lRetVal < 0)
	 {
		sl_FsClose(lFileHandle, 0, 0, 0);
		RETURN_ON_ERROR(lRetVal);
	 }
	 else
	 {
		 uiNumBytesRead = lRetVal;
	 }

	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
	RETURN_ON_ERROR(lRetVal);

	return uiNumBytesRead;
}


#ifdef COMPILE_THIS
sl_FsDel((unsigned char *)USER_FILE_NAME, ulToken);
//
// Error handling if File Operation fails
//
if(lRetVal < 0)
{
    lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    RETURN_ON_ERROR(CAMERA_CAPTURE_FAILED);
}
#endif

#ifdef COMPILE_THIS
//
// NVMEM File Open to write to SFLASH
//
lRetVal = sl_FsOpen((unsigned char *)USER_FILE_NAME,//0x00212001,
					//FS_MODE_OPEN_CREATE(65260,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
					//FS_MODE_OPEN_CREATE(65,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
						//FS_MODE_OPEN_CREATE(120535,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
					FS_MODE_OPEN_CREATE((MAX_IMAGE_SIZE_BYTES+JPEG_HEADER_MAX_FILESIZE),_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
                    &ulToken,
                    &lFileHandle);
if(lRetVal < 0)
{
	DEBG_PRINT("File Open Error: %i", lRetVal);
	lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
    RETURN_ON_ERROR(CAMERA_CAPTURE_FAILED);
}
// Close the created file
lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
RETURN_ON_ERROR(lRetVal);
#endif

#ifdef COMPILE_THIS
SlFsFileInfo_t fileInfo;
sl_FsGetInfo((unsigned char *)USER_FILE_NAME, ulToken, &fileInfo);
#endif
