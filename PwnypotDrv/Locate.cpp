#include "stdafx.h"
#include "Kernel.h"
#include "SSDT.h"

ULONG NtBuildNumber;
ULONG NtAllocateVirtualMemoryIndex;
ULONG NtProtectVirtualMemoryIndex;
ULONG KTHREAD_PreviousMode_Offset;
ULONG KTHREAD_TrapFrame_Offset;

NtProtectVirtualMemory__ NtProtectVirtualMemory;
NtAllocateVirtualMemory__ NtAllocateVirtualMemory;

ULONG Locate()
{
	PsGetVersion(NULL, NULL, &NtBuildNumber, NULL);
	switch(NtBuildNumber)
	{
	case 7600:
	case 7601:
		NtAllocateVirtualMemoryIndex = 19;
		NtProtectVirtualMemoryIndex = 215;
		KTHREAD_PreviousMode_Offset = 0x13a;
		KTHREAD_TrapFrame_Offset = 0x128;
		break;
	default:
		return STATUS_NOT_SUPPORTED;
	}

	SsdtRelocateServiceTable();
}