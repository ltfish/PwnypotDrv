#include "stdafx.h"
#include "Error.h"
#include "..\PwnypotDrv\FunctionDef.h"

#define IOCTL_CALL_PWNYPOT_DRV CTL_CODE(\
	FILE_DEVICE_UNKNOWN, \
	0x800, \
	METHOD_BUFFERED, \
	FILE_ANY_ACCESS)

WCHAR szDriverName[] = L"PwnypotDrv0";
WCHAR szDriverFileName[] = L"PwnypotDrv.sys";
HANDLE DrvHandle = NULL;

STATUS LoadDriver()
{
	WCHAR szDriverImagePath[MAX_PATH];
	GetFullPathName(szDriverFileName, 
		sizeof(szDriverImagePath) / sizeof(WCHAR), 
		szDriverImagePath, 
		NULL);

	SC_HANDLE hServiceMgr, hService;

	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	DWORD dwRet = PDC_STATUS_SUCCESS;
	DWORD dwLastError = 0;

	while(TRUE)
	{
		if(!hServiceMgr)
		{
			dwRet = PDC_STATUS_INTERNAL_ERROR;
			break;
		}

		hService = CreateService(
			hServiceMgr,
			szDriverName,
			szDriverName,
			SERVICE_ALL_ACCESS,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_IGNORE,
			szDriverImagePath,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);

		if(hService == NULL)
		{
			dwLastError = GetLastError();
			if(dwLastError == ERROR_IO_PENDING || dwLastError == ERROR_SERVICE_EXISTS)
			{
				// The service already exists
			}
			else
			{
				dwRet = PDC_STATUS_INTERNAL_ERROR;
				break;
			}

			hService = OpenService(hServiceMgr, szDriverName, SERVICE_ALL_ACCESS);

			if(hService == NULL)
			{
				dwLastError = GetLastError();
				// TODO: Process the dwRet
				dwRet = PDC_STATUS_INTERNAL_ERROR;
				break;
			}
		}

		BOOL bRet;
		bRet = StartService(hService, NULL, NULL);
		if(!bRet)
		{
			dwLastError = GetLastError();
			if(dwRet == ERROR_SERVICE_ALREADY_RUNNING)
			{
				// Service is already running
			}
			else
			{
				dwRet = PDC_STATUS_INTERNAL_ERROR;
				break;
			}
		}

		break;
	}
	
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	if(hService)
	{
		CloseServiceHandle(hService);
	}
	if(dwRet != PDC_STATUS_SUCCESS)
	{
		MessageBox(NULL, L"Error occurs.", L"Result", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(NULL, L"Driver loaded.", L"Result", MB_OK | MB_ICONINFORMATION);

		HANDLE hDevice = 
			CreateFile(L"\\\\.\\PwnypotDrv0",
						GENERIC_READ | GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		if(hDevice == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, L"Error while opening the driver.", L"Result", MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			// TODO: Close the handle when exiting the program!
			DrvHandle = hDevice;
		}
	}

	return dwRet;
}

DWORD PrepareInputBuffer(BYTE* bytBufferIn, DWORD uFunctionNo, DWORD uParamCount, DWORD uParam1)
{
	// TODO: Check the size of bytBufferIn

	DWORD* puFunctionNo = (DWORD*)bytBufferIn;
	*puFunctionNo = uFunctionNo;
	if(uParamCount == 1)
	{
		DWORD* puParam1 = (DWORD*)bytBufferIn + 1;
		*puParam1 = uParam1;
	}

	return 8;
}

STATUS DrvAddPid(DWORD uPid)
{
	if(!DrvHandle)
	{
		MessageBox(NULL, L"DrvHandle is NULL", L"Error", MB_OK);
		return PDC_STATUS_INTERNAL_ERROR;
	}

	MessageBox(NULL, L"Adding pid...", L"Prompt", MB_OK);

	BYTE bytBufferOut[256];
	BYTE bytBufferIn[256];
	DWORD uSize = 
		PrepareInputBuffer(bytBufferIn, FUNCTION_ADD_PID, 1, uPid);
	DWORD uOutput;

	if(!DeviceIoControl(DrvHandle,
		IOCTL_CALL_PWNYPOT_DRV,
		bytBufferIn, uSize,
		bytBufferOut, sizeof(bytBufferOut),
		&uOutput,
		NULL))
	{
		DWORD dwLastError = GetLastError();
		WCHAR szPrompt[2048];
		wsprintf(szPrompt, L"DeviceIoControl failed. DrvHandle = %d, Lasterror = %x", DrvHandle, dwLastError);
		MessageBox(NULL, szPrompt, L"Error", MB_OK);
		return PDC_STATUS_INTERNAL_ERROR;
	}

	return PDC_STATUS_SUCCESS;
}

STATUS DrvRemovePid(DWORD uPid)
{
	if(!DrvHandle)
	{
		return PDC_STATUS_INTERNAL_ERROR;
	}

	BYTE bytBufferIn[256];
	BYTE bytBufferOut[256];
	DWORD uSize = 
		PrepareInputBuffer(bytBufferIn, FUNCTION_REMOVE_PID, 1, uPid);
	DWORD uOutput;

	if(!DeviceIoControl(DrvHandle,
		IOCTL_CALL_PWNYPOT_DRV,
		bytBufferIn, uSize,
		bytBufferOut, sizeof(bytBufferOut),
		&uOutput,
		NULL))
	{
		return PDC_STATUS_INTERNAL_ERROR;
	}

	return PDC_STATUS_SUCCESS;
}