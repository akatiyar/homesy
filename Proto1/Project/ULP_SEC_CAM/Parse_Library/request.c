/*
 *  Copyright (c) 2015, Parse, LLC. All rights reserved.
 *
 *  You are hereby granted a non-exclusive, worldwide, royalty-free license to use,
 *  copy, modify, and distribute this software in source code or binary form for use
 *  in connection with the web services and APIs provided by Parse.
 *
 *  As with any software that integrates with the Parse platform, your use of
 *  this software is subject to the Parse Terms of Service
 *  [https://www.parse.com/about/terms]. This copyright notice shall be
 *  included in all copies or substantial portions of the software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <simplelink.h>
#include "utils.h"
#include "parse_impl.h"


const char g_deviceClientVersion[] = "c-ti-cc3200-rtos-1.0.0";

static char parseServer[] = "api.parse.com";
unsigned short sshPort = 443;

#define REQUEST_HEADERS_SIZE	400
#define REQUEST_BODY_SIZE		500

#define SIZE_SOCK_WRITE_DATA	1024
// We need roughly 400 bytes for headers
// and about 500 bytes for a reasonably big object
// (enough to fit a full installation object)
// Both numbers are padded up, but let's add another
// 124 bytes just in case
//#define RECEIVE_BUFFER_SIZE 13024

// Use only one preallocated buffer for now
// Consider allocating per request
char* dataBuffer;

//char dataBuffer[1024];

volatile int cnt = 0;
unsigned long dataBufferSize = 0;

void setDataBufferPointer(unsigned long* ucDataBufferPtr,unsigned long len)
{
	dataBuffer = (char* ) ucDataBufferPtr;
	dataBufferSize = len;
	memset(dataBuffer,NULL,dataBufferSize);
}

int buildRequestHeaders(ParseClientInternal *parseClient, const char *host, const char *httpVerb, const char *httpRequestBody, int addInstallationHeader, Payload_Type payloadType) {
    int status = 0;

    int currentPosition = 0;
//    int currentSize = sizeof(dataBuffer) - currentPosition;
    int currentSize = dataBufferSize - currentPosition;
    memset(dataBuffer,NULL,dataBufferSize);

    int isGetRequest = strncasecmp(httpVerb, "GET", 3) == 0;
    int hasBody = (httpRequestBody != NULL) && (strlen(httpRequestBody) > 0);

    if (isGetRequest != FALSE) {
        hasBody = FALSE;
    }

    if (isGetRequest != FALSE) {
        status = beginHttpGetRequest(dataBuffer + currentPosition, currentSize, host, httpVerb, httpRequestBody);
    } else {
        status = beginHttpRequest(dataBuffer + currentPosition, currentSize, host, httpVerb);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "Host", parseServer);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "User-Agent", g_deviceClientVersion);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-OS-Version", g_osVersion);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-Client-Version", g_deviceClientVersion);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-Application-Id", parseClient->applicationId);
    }

    if (status >= 0) {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-Client-Key", parseClient->clientKey);
    }

    if (addInstallationHeader) {
        if (status >= 0) {
            currentPosition += status;
            currentSize -= status;
            status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-Installation-Id",  parseClient->installationId);
        }

        if (status >= 0) {
            currentPosition += status;
            currentSize -= status;
            status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "X-Parse-Session-Token", parseClient->sessionToken);
        }
    }

    if (hasBody)
    {
    	//if(cnt == 0)
    	if (payloadType == jsonObject)
    	{
    		//DEBUG_PRINT("Object Payload");
			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "Content-Type", "application/json; charset=utf-8");
			}

			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = addHttpRequestHeaderInt(dataBuffer + currentPosition, currentSize, "Content-Length", strlen(httpRequestBody));
			}
			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = snprintf(dataBuffer + currentPosition, currentSize, "\r\n");
			}

			if (status >= 0)
			{
				currentPosition += status;
			}

			/*if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = snprintf(dataBuffer + currentPosition, currentSize, "%s", httpRequestBody);
			}*/
    	}

//#ifdef POST_IMAGE
//    	if(cnt == 1)
    	if( payloadType == image)
    	{
    		//DEBUG_PRINT("Image Payload");
    		if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = addHttpRequestHeader(dataBuffer + currentPosition, currentSize, "Content-Type", "image/jpeg");
			}

			 // open a user file for reading
//    		unsigned char myFile[] = "www/images/testimage.jpg";
//    		unsigned char myFile[] = "www/images/cc3200_camera_capture.jpg";
    		unsigned long ulToken = NULL;
    		long lRetVal;
/*//			 lRetVal = sl_FsOpen(myFile, FS_MODE_OPEN_READ, &ulToken, &lFileHandle);
    		lRetVal = sl_FsOpen(httpRequestBody, FS_MODE_OPEN_READ, &ulToken, &lFileHandle);
			 if(lRetVal < 0)
			 {
				 DEBUG_PRINT("\nFile open\n%l\n\n",lRetVal);
				 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
				 for( ;; );
			 }
*/
			 SlFsFileInfo_t fileInfo;
//			 sl_FsGetInfo(myFile, ulToken, &fileInfo);
			 lRetVal = sl_FsGetInfo((_u8*)httpRequestBody, ulToken, &fileInfo);
			 if (lRetVal < 0)
			 {
				 for( ;; );
			 }

			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = addHttpRequestHeaderInt(dataBuffer + currentPosition, currentSize, "Content-Length", fileInfo.FileLen);
//				status = addHttpRequestHeaderInt(dataBuffer + currentPosition, currentSize, "Content-Length", 64000);
			}

			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				status = snprintf(dataBuffer + currentPosition, currentSize, "\r\n");
			}

			if (status >= 0)
			{
				currentPosition += status;
			}

/*			if (status >= 0)
			{
				currentPosition += status;
				currentSize -= status;
				//status = snprintf(dataBuffer + currentPosition, currentSize, "%s", httpRequestBody);
				lRetVal = sl_FsRead(lFileHandle, 0, dataBuffer + currentPosition , fileInfo.FileLen);
//				lRetVal = sl_FsRead(lFileHandle, 0, dataBuffer + currentPosition , 64000);
				 if (lRetVal < 0)
				 {
					 DEBUG_PRINT("\nFile read\n%ld\n\n",lRetVal);
					 lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
					 for( ;; );
				 }
				lRetVal = sl_FsClose(lFileHandle, 0, 0, 0);
				if (SL_RET_CODE_OK != lRetVal)
				{
					for( ;; );
				}
				status = fileInfo.FileLen;
//				status = 64000;
			}*/

    	}
//#endif
    }


//#ifdef POST_IMAGE
//			cnt++;
//#endif
    return (status < 0) ? status : currentPosition;
}

int headerSend(ParseClientInternal *parseClient,
				const char *host,
				const char *httpVerb,
				const char *httpRequestBody,
				int addInstallationHeader,
				Payload_Type payloadType,
				short socketHandle)
{
	long status;

	status = buildRequestHeaders(parseClient, host, httpVerb, httpRequestBody, addInstallationHeader, payloadType);

	//(status >= 0)?DEBUG_PRINT("Header:\n\r%s", dataBuffer):DEBUG_PRINT("[Parse] Build request header error: %d\r\n", status);
	(status >= 0)?DEBUG_PRINT("Header Built\n\r%s"):DEBUG_PRINT("[Parse] Build request header error: %d\r\n", status);

	if (status >= 0)
	{
		long temp = status;
		status = socketWrite(socketHandle, dataBuffer, status);
		DEBUG_PRINT("[Parse] Header Socket Write status: %d\r\n", status);
		if(status != temp)
		{
			DEBUG_PRINT("\nHeader data written != data supposed to be written\n\r");
			while(1);
		}
	}

	return status;
}

int payloadSend( const char *httpRequestBody,
					Payload_Type payloadType,
					short socketHandle)
{
	long status;

	if (payloadType == jsonObject)
	{
		status = snprintf(dataBuffer, dataBufferSize , "%s", httpRequestBody);


	}
	else if( payloadType == image )
	{
		long lFileHandle;
		unsigned long ulToken = 0;
		SlFsFileInfo_t fileInfo;

		sl_FsGetInfo((_u8*)httpRequestBody, ulToken, &fileInfo);

		status = sl_FsOpen((_u8*)httpRequestBody, FS_MODE_OPEN_READ,
							&ulToken, &lFileHandle);
		if(status < 0)
		{
			DEBUG_PRINT("\nFile open\n%l\n\n",status);
			status = sl_FsClose(lFileHandle, 0, 0, 0);
			for( ;; );
		}

		unsigned int i, size_readBytes = 0;

		for (i=0; i<(fileInfo.FileLen/SIZE_SOCK_WRITE_DATA); i++)
		{
			status = sl_FsRead(lFileHandle,
								(SIZE_SOCK_WRITE_DATA*i),
								(unsigned char*)dataBuffer,
								SIZE_SOCK_WRITE_DATA);
			 if (status < 0)
			 {
				 DEBUG_PRINT("\nFile read failed: \n%ld\n\n",status);
				 status = sl_FsClose(lFileHandle, 0, 0, 0);
				 for( ;; );
			}
			size_readBytes += status;

			if (status >= 0)
			{
				long temp = status;
				status = socketWrite(socketHandle, dataBuffer, (status));
				//DEBUG_PRINT("[Parse] Payload Socket Write status: %d\r\n", status);
				if(status != temp)
				{
					DEBUG_PRINT("\nPayload data written != data supposed to be written\n\r");
					while(1);
				}
			}
		}

		status = sl_FsRead(lFileHandle,
							(SIZE_SOCK_WRITE_DATA*i),
							(unsigned char*)dataBuffer,
							(fileInfo.FileLen%SIZE_SOCK_WRITE_DATA));
		 if (status < 0)
		 {
			 DEBUG_PRINT("\nFile read failed: \n%ld\n\n",status);
			 status = sl_FsClose(lFileHandle, 0, 0, 0);
			 for( ;; );
		 }
		 size_readBytes += status;


		status = sl_FsClose(lFileHandle, 0, 0, 0);
		if (SL_RET_CODE_OK != status)
		{
			for( ;; );
		}

		status = (fileInfo.FileLen%SIZE_SOCK_WRITE_DATA);
	}
	else
	{
		//DEBUG_PRINT("\n\r^^^^^^^^^^NO PAYLOAD^^^^^^^^^^^^^^\n\r");
		if (status >= 0)
		{
			status = snprintf(dataBuffer, dataBufferSize , "\r\n");
		}
	}

	if (status >= 0)
    {
        dataBuffer[status] = 0;
        status++;
    }

	//(status >= 0)?DEBUG_PRINT("Payload:\n\r%s\n\r", dataBuffer):DEBUG_PRINT("[Parse] Build request Payload error: %d\r\n", status);
	(status >= 0)?DEBUG_PRINT("Payload built\n\r"):DEBUG_PRINT("[Parse] Build request Payload error: %d\r\n", status);

	if (status >= 0)
	{
		long temp = status;
		status = socketWrite(socketHandle, dataBuffer, (status));
		DEBUG_PRINT("[Parse] Payload Socket Write status: %d\r\n", status);
		if(status != temp)
		{
			DEBUG_PRINT("\nPayload data written != data supposed to be written\n\r");
			while(1);
		}
	}

	return status;
}

int sendRequest(ParseClientInternal *parseClient, const char *host, const char *httpVerb, const char *httpRequestBody, int addInstallationHeader, Payload_Type payloadType) {
    short socketHandle = -1;
    char responseObjectID[OBJECT_ID_MAX_LEN];
    char imageName[IMAGE_NAME_MAX_LEN];

    long status = socketSslConnect(parseServer, sshPort);

    if (status >= 0)
    {
        socketHandle = status;

#ifdef REQUEST_DEBUG
        DEBUG_PRINT("\r\n[Parse] --------Request--------- Start -\r\n");
#endif /* REQUEST_DEBUG */

        headerSend(parseClient, host, httpVerb, httpRequestBody, addInstallationHeader, payloadType, socketHandle);

        payloadSend(httpRequestBody, payloadType, socketHandle);

//        if (status >= 0)
//		{
//			long temp = status;
//			status = socketWrite(socketHandle, dataBuffer, status);
//			DEBUG_PRINT("[Parse] Header Socket Write status: %d\r\n", status);
//			if(status != temp)
//			{
//				DEBUG_PRINT("\nHeader data written != data supposed to be written\n\r");
//				while(1);
//			}
//		}

#ifdef REQUEST_DEBUG
        //DEBUG_PRINT("[Parse] --------Request--------- End -\r\n");
#endif /* REQUEST_DEBUG */
    }


    if (status >= 0)
    {
#ifdef REQUEST_DEBUG
    	DEBUG_PRINT("\r\n[Parse] --------Response--------- Start -\r\n");
#endif /* REQUEST_DEBUG */

        memset(dataBuffer, 0, dataBufferSize);
        status = socketRead(socketHandle, dataBuffer, dataBufferSize, 20000);
        //status = socketRead(socketHandle, dataBuffer, dataBufferSize, 5000);
        //status = socketRead(socketHandle, dataBuffer, dataBufferSize, 10000);
        /*//DBG - to see if trying again helps
        if( status == 0 )
        {
        	DEBUG_PRINT("Trying Read again\n\r");
        	status = socketRead(socketHandle, dataBuffer, dataBufferSize, 10000);
        }*/

        DEBUG_PRINT("[Parse] Socket read status: %d\r\n", status);
#ifdef REQUEST_DEBUG
        if (status >= 0)
        {
            //DEBUG_PRINT("%s\r\n", dataBuffer);
        	if (payloadType == jsonObject)
        	{
        		simpleJsonProcessor(dataBuffer, "objectId", responseObjectID, OBJECT_ID_MAX_LEN);
        		DEBUG_PRINT("objectID : %s\r\n", responseObjectID);
        	}
        	else if (payloadType == image)
        	{
        		simpleJsonProcessor(dataBuffer, "name", imageName, IMAGE_NAME_MAX_LEN);
        		DEBUG_PRINT("Image Name : %s\r\n", imageName);
        	}
        } else
        {
            DEBUG_PRINT("[Parse] Response read error: %d\r\n", status);
        }
        //DEBUG_PRINT("[Parse] --------Response--------- End -\r\n");
#endif /* REQUEST_DEBUG */

        UtilsDelay(80000000);
        socketClose(socketHandle);
#ifdef REQUEST_DEBUG
    } else {
        DEBUG_PRINT("[Parse] Request write error: %d\r\n", status);
#endif /* REQUEST_DEBUG */
    }

    return status;
}

long parseSendRequestInternal(ParseClient client, const char *httpVerb, const char *httpPath, const char *httpRequestBody, parseRequestCallback callback, int addInstallationHeader, Payload_Type payloadType) {
    long status = sendRequest(getInternalClient(client), httpPath, httpVerb, httpRequestBody, addInstallationHeader, payloadType);

    if (callback) {
        if (status >= 0) {
            callback(client, 0, getHttpResponseStatus(dataBuffer), getHttpResponseBody(dataBuffer));
        } else {
            callback(client, status, -1, NULL);
        }
    }

    return status;
}

long parseSendRequest(ParseClient client, const char *httpVerb, const char *httpPath, const char *httpRequestBody, parseRequestCallback callback, Payload_Type payloadType) {

	long status;

	getInstallation(getInternalClient(client));

#ifdef REQUEST_TRACE
    DEBUG_PRINT("[Parse] Send %s request to %s \r\n", httpVerb, httpPath);
#endif /* REQUEST_TRACE */

    status = parseSendRequestInternal(client, httpVerb, httpPath, httpRequestBody, callback, TRUE, payloadType);

    return status;
}

int parseGetErrorCode(const char *httpResponseBody) {
    // Simple implementation, just extract the 'code' value
    char code[10] = {0};
    if (simpleJsonProcessor(httpResponseBody, "code", code, 10) != 0) {
        return atoi(code);
    }

    return 2;
}

