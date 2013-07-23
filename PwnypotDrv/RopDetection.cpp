#include "stdafx.h"
#include "Locate.h"
#include "RopDetection.h"

extern "C"
{
	NTSTATUS
		PsLookupThreadByThreadId(
		__in HANDLE ThreadId,
		__deref_out PETHREAD *Thread
		);
};

typedef unsigned short WORD;

typedef struct _EXCEPTION_REGISTRATION_RECORD
{
	struct _EXCEPTION_REGISTRATION_RECORD *Next;
	PVOID Handler;
} EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

typedef struct _KTRAP_FRAME
{
	UINT32 DbgEbp;
	UINT32 DbgEip;
	UINT32 DbgArgMark;
	UINT32 DbgArgPointer;
	UINT32 TempSegCs;
	UINT32 TempEsp;
	UINT32 Dr0;
	UINT32 Dr1;
	UINT32 Dr2;
	UINT32 Dr3;
	UINT32 Dr6;
	UINT32 Dr7;
	UINT32 SegGs;
	UINT32 SegEs;
	UINT32 SegDs;
	UINT32 Edx;
	UINT32 Ecx;
	UINT32 Eax;
	UINT32 PreviousPreviousMode;
	PEXCEPTION_REGISTRATION_RECORD ExceptionList;
	UINT32 SegFs;
	UINT32 Edi;
	UINT32 Esi;
	UINT32 Ebx;
	UINT32 Ebp;
	UINT32 ErrCode;
	UINT32 Eip;
	UINT32 SegCs;
	UINT32 EFlags;
	UINT32 HardwareEsp;
	UINT32 HardwareSegSs;
	UINT32 V86Es;
	UINT32 V86Ds;
	UINT32 V86Fs;
	UINT32 V86Gs;
} KTRAP_FRAME, *PKTRAP_FRAME;

LIST_ENTRY ProcMonitored;

VOID InitializeProcList()
{
	InitializeListHead(&ProcMonitored);
}

PPROC_MONITORED GetProcessInfoBlock(HANDLE uPid)
{
	PLIST_ENTRY pEntry = NULL;

	for(pEntry = ProcMonitored.Flink;
		pEntry != ProcMonitored.Flink;
		pEntry = pEntry->Flink)
	{
		PPROC_MONITORED pData = CONTAINING_RECORD(pEntry,
			PROC_MONITORED,
			ListEntry);
		if(pData->uPid == (ULONG)uPid)
		{
			return pData;
		}
	}

	return NULL;
}

VOID InsertIntoProcList(ULONG uPid)
{
	// TODO: Spin lock!

	PPROC_MONITORED pData =
		(PPROC_MONITORED)ExAllocatePool(PagedPool, sizeof(PROC_MONITORED));
	pData->uPid = uPid;
	pData->bRopFlag = FALSE;
	InsertTailList(&ProcMonitored, &pData->ListEntry);
}

VOID RemoveFromProcList(ULONG uPid)
{
	// TODO: Spin lock!

	PPROC_MONITORED pData =
		GetProcessInfoBlock((HANDLE)uPid);

	if(pData != NULL)
	{
		RemoveEntryList(&pData->ListEntry);
	}
}

VOID DestroyProcList()
{
	while(!IsListEmpty(&ProcMonitored))
	{
		PLIST_ENTRY pEntry = RemoveTailList(&ProcMonitored);
		PPROC_MONITORED pData = CONTAINING_RECORD(pEntry,
			PROC_MONITORED,
			ListEntry);
		ExFreePool(pData);
	}
}

BOOLEAN IsMonitoringStackPagePermission(HANDLE uPid)
{
	// TODO
	return TRUE;
}

BOOLEAN IsMonitoringStackPointer(HANDLE uPid)
{
	// TODO
	return TRUE;
}

BOOLEAN IsMonitoringProcess(HANDLE uPid)
{
	PPROC_MONITORED pData =
		GetProcessInfoBlock(uPid);

	if(pData == NULL)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOLEAN DbgGetRopFlag(HANDLE uPid)
{
	PPROC_MONITORED pData = 
		GetProcessInfoBlock(uPid);

	if(pData == NULL)
	{
		return FALSE;
	}

	return pData->bRopFlag;
}

BOOLEAN DbgSetRopFlag(HANDLE uPid)
{
	// TODO
	return TRUE;
}

VOID
ValidateSystemCallAgainstRop(
	IN ROP_CALLEE RopCallee,
	IN PVOID lpAddress,
	IN ULONG flProtect
	)
{
	HANDLE uPid = PsGetCurrentProcessId();

	if(IsMonitoringProcess(uPid))
	{
		PNT_TIB pThreadInfo;
		HANDLE uThreadId;
		PETHREAD pEThread;
		NTSTATUS uNtStatus;
		
		uThreadId = PsGetCurrentThreadId();
		uNtStatus = PsLookupThreadByThreadId(uThreadId, &pEThread);
		
		if(uNtStatus != STATUS_SUCCESS)
		{
			return;
		}
		if(*((PCHAR)pEThread + KTHREAD_PreviousMode_Offset) == 1)
		{
			// The caller is in Ring 3
			ULONG lpEspAddress;
			PKTRAP_FRAME pTrapFrame = *(PKTRAP_FRAME*)((PCHAR)pEThread + KTHREAD_TrapFrame_Offset);
			lpEspAddress = pTrapFrame->HardwareEsp;
			pThreadInfo = (PNT_TIB)__readfsdword(0x18);
			if(IsMonitoringStackPointer(uPid))
			{
				if(lpEspAddress < (ULONG)pThreadInfo->StackLimit ||
					lpEspAddress >= (ULONG)pThreadInfo->StackBase)
				{
					// TODO: Tell the target program that this is a ROP attack
					DbgSetRopFlag(uPid);
					KdPrint(("ROP detected by STACK_MONITOR. Esp [%08x] out of bound stack.\n", lpEspAddress));
				}
			}

			if(IsMonitoringStackPagePermission(uPid))
			{
				if(lpAddress > pThreadInfo->StackLimit ||
					lpAddress <= pThreadInfo->StackBase)
				{
					if((flProtect & PAGE_EXECUTE) ||
						(flProtect & PAGE_EXECUTE_READ) ||
						(flProtect & PAGE_EXECUTE_READWRITE) ||
						(flProtect & PAGE_EXECUTE_WRITECOPY))
					{
						// TODO: Tell the target program that this is a ROP attack
						// KdPrint(("ROP detected by STACK_RWX. Stack is set to be executable.\n", lpEspAddress));
					}
				}
			}
		}
	}
}