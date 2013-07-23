#include "stdafx.h"
#include "FunctionDef.h"
#include "RopDetection.h"

ULONG PwnypotAddPid(PULONG pParamList, ULONG uParamListSize, PULONG pDataOut, ULONG uDataOutSize);
ULONG PwnypotRemovePid(PULONG pParamList, ULONG uParamListSize, PULONG pDataOut, ULONG uDataOutSize);

/*
 * in_buffer = [FunctionNo(ULONG)][Param1(ULONG)]...
 *
 */
ULONG PwnypotDispatch(PVOID pDataIn, ULONG uDataInSize, PVOID pDataOut, ULONG uDataOutLength)
{
	ULONG uFunctionNo = *((PULONG)pDataIn);
	PULONG pParamList = (PULONG)((ULONG)pDataIn + sizeof(ULONG));
	ULONG uParamCount = (uDataInSize - 1) / sizeof(ULONG);
	ULONG uResult = 0;

	KdPrint(("Calling PwnypotDispatch with FunctionNo[%02x], BufferOutSize = %d.\n", uFunctionNo, uDataOutLength));

	switch(uFunctionNo)
	{
	case FUNCTION_ADD_PID:
		uResult = PwnypotAddPid(pParamList, uParamCount, (PULONG)pDataOut, uDataOutLength);
		break;
	case FUNCTION_REMOVE_PID:
		uResult = PwnypotRemovePid(pParamList, uParamCount, (PULONG)pDataOut, uDataOutLength);
	default:
		// TODO: Get a better error number
		uResult = 1;
		break;
	}

	return uResult;
}

/*
 * Add a pid into link table
 *
 * Input:
 *		0x0 - PID
 *
 * Output:
 *		0x0 - NTSTATUS
 *
 */
ULONG PwnypotAddPid(PULONG pParamList, ULONG uParamListSize, PULONG pDataOut, ULONG uDataOutSize)
{
	if(uDataOutSize < sizeof(NTSTATUS))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	KdPrint(("Adding PID %d.\n", pParamList[0]));
	InsertIntoProcList(pParamList[0]);

	*pDataOut = STATUS_SUCCESS;
	return STATUS_SUCCESS;
}

ULONG PwnypotRemovePid(PULONG pParamList, ULONG uParamListSize, PULONG pDataOut, ULONG uDataOutSize)
{
	if(uDataOutSize < sizeof(NTSTATUS))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	KdPrint(("Removing PID %d.\n", pParamList[0]));
	RemoveFromProcList(pParamList[0]);

	*pDataOut = STATUS_SUCCESS;
	return STATUS_SUCCESS;
}