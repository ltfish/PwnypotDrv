#include "stdafx.h"
#include "Error.h"
#include "PipeServer.h"
#include "DrvComm.h"

#define MAX_BUFFER_SIZE 512

TCHAR szPipename[] = L"\\\\.\\pipe\\PwnypotDrvComm";

void ProcessRequest(void* hPipe)
{
	CHAR szBuf[MAX_BUFFER_SIZE] = {0};
	DWORD uBytesRead = 0;
	DWORD uBytesWritten = 0;

	BOOL bIsPipeBroken = FALSE;

	while(!bIsPipeBroken)
	{
		memset(szBuf, MAX_BUFFER_SIZE, 0);
		BOOL bSuccess = ReadFile(hPipe, szBuf, MAX_BUFFER_SIZE, &uBytesRead, NULL);

		if(!bSuccess || uBytesRead == 0)
		{
			bIsPipeBroken = TRUE;
			break;
		}

		if(szBuf[0] == PDC_PIPE_PING)
		{
			// Respond with a 'pong'
			szBuf[0] = PDC_PIPE_PONG;
			bSuccess = WriteFile(hPipe, szBuf, 1, &uBytesWritten, NULL);
			if(!bSuccess || uBytesWritten != 1)
			{
				bIsPipeBroken = TRUE;
				break;
			}
		}
		else if(szBuf[0] == PDC_PIPE_ADD_PID)
		{
			// Add a PID
			DWORD uPid = *(DWORD*)(szBuf + 1);
			szBuf[0] = PDC_PIPE_ADD_PID_RESP;
			if(DrvAddPid(uPid) == PDC_STATUS_SUCCESS)
			{
				*(DWORD*)(szBuf + 1) = PDC_STATUS_SUCCESS;

				bSuccess = WriteFile(hPipe, szBuf, 5, &uBytesWritten, NULL);
				if(!bSuccess || uBytesWritten != 5)
				{
					bIsPipeBroken = TRUE;
					break;
				}
			}
			else
			{
				// TODO:
			}
		}
		else if(szBuf[0] == PDC_PIPE_REMOVE_PID)
		{
			// Remove a PID
			DWORD uPid = *(DWORD*)(szBuf + 1);
			szBuf[0] = PDC_PIPE_ADD_PID_RESP;
			if(DrvRemovePid(uPid) == PDC_STATUS_SUCCESS)
			{
				*(DWORD*)(szBuf + 1) = PDC_STATUS_SUCCESS;

				bSuccess = WriteFile(hPipe, szBuf, 5, &uBytesWritten, NULL);
				if(!bSuccess || uBytesWritten != 5)
				{
					bIsPipeBroken = TRUE;
					break;
				}
			}
			else
			{
				// TODO:
			}
		}
	}

	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
}

STATUS CreatePipeServer()
{
	void* hPipe = (void*)-1;
	while(true)
	{
		hPipe = CreateNamedPipe(
			szPipename,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024,
			1024,
			0,
			NULL);
		if(hPipe == (void*)-1)
		{
			return PDC_STATUS_INTERNAL_ERROR;
		}

		BOOL bIsConnected = 
			ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if(bIsConnected)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessRequest, hPipe, 0, NULL);
		}
		else
		{
			CloseHandle(hPipe);
		}
	}

	return PDC_STATUS_SUCCESS;
}