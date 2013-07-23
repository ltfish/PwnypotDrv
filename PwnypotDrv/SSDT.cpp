#include "stdafx.h"
#include "SSDT.h"
#include "Locate.h"
#include "Kernel.h"

//////////////////////////////////////////////////////////////////////////
// Members
//////////////////////////////////////////////////////////////////////////

PVOID SSDTEntryBackup[1024] = {0};

//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////

VOID DisableWP(PULONG puAttrribute)
{
	ULONG dwAttribute;

	__asm
	{
		cli
		mov eax, cr0
		mov dwAttribute, eax
		and eax, 0xfffeffff // Set the 16th bit to 0
		mov cr0, eax
	}

	*puAttrribute = dwAttribute;
}

VOID RestoreWP(ULONG uOldAttribute)
{
	__asm
	{
		mov eax, uOldAttribute
		mov cr0, eax
		sti
	}
}

//////////////////////////////////////////////////////////////////////////
// Public functions
//////////////////////////////////////////////////////////////////////////

/*
 * Install SSDT Hook
 *
 *
 */
VOID SsdtInstallHook(ULONG uServiceNumber, PVOID pNewServiceAddr)
{
	ULONG uOldAttribute;

	// Backup the old address
	SSDTEntryBackup[uServiceNumber] = SSDT_ENTRY_FUNCTION(uServiceNumber);

	DisableWP(&uOldAttribute);
	
	// TODO: Put APC onto other cores to fix the synchronization problem

	SSDT_ENTRY_FUNCTION(uServiceNumber) = pNewServiceAddr;

	RestoreWP(uOldAttribute);
}

/*
 *
 *
 *
 */
ULONG SsdtUninstallHook(ULONG uServiceNumber)
{
	ULONG uOldAttribute;

	if(SSDTEntryBackup[uServiceNumber] == 0)
	{
		return STATUS_INVALID_HANDLE;
	}

	DisableWP(&uOldAttribute);

	SSDT_ENTRY_FUNCTION(uServiceNumber) = SSDTEntryBackup[uServiceNumber];

	RestoreWP(uOldAttribute);

	return STATUS_SUCCESS;
}

/*
 *
 *
 *
 */
ULONG SsdtRelocateServiceTable()
{
	if(!KeServiceDescriptorTable.Table1.ServiceTable)
	{
		return STATUS_NOT_SUPPORTED;
	}

	switch(NtBuildNumber)
	{
	case 7600:
	case 7601:
		NtProtectVirtualMemory = (NtProtectVirtualMemory__)SSDT_ENTRY_FUNCTION(NtProtectVirtualMemoryIndex);
		NtAllocateVirtualMemory = (NtAllocateVirtualMemory__)SSDT_ENTRY_FUNCTION(NtAllocateVirtualMemoryIndex);
	}
}