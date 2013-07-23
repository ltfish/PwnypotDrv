#include "stdafx.h"
#include "Kernel.h"
#include "Hook.h"
#include "Locate.h"
#include "RopDetection.h"

VOID Hook()
{
	SsdtInstallHook(NtProtectVirtualMemoryIndex, HookedNtProtectVirtualMemory);
	SsdtInstallHook(NtAllocateVirtualMemoryIndex, HookedNtAllocateVirtualMemory);
	KdPrint(("SSDT hooking finished.\n"));
}

VOID Unhook()
{
	SsdtUninstallHook(NtProtectVirtualMemoryIndex);
	SsdtUninstallHook(NtAllocateVirtualMemoryIndex);
}

NTSTATUS NTAPI HookedNtProtectVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN OUT PULONG NumberOfBytesToProtect,
	IN ULONG NewAccessProtection,
	OUT PULONG OldAccessProtection)
{
	HANDLE uProcessId = PsGetCurrentProcessId();

	// Log it
	/*KdPrint(("Process %d calling NtProtectVirtualMemory %x %x %x %x %x\n", 
		uProcessId,
		ProcessHandle, 
		BaseAddress,
		NumberOfBytesToProtect,
		NewAccessProtection,
		OldAccessProtection));*/

	ValidateSystemCallAgainstRop(_NtProtectVirtualMemory_, BaseAddress, NewAccessProtection);

	// Call the original API
	return NtProtectVirtualMemory(
		ProcessHandle, 
		BaseAddress, 
		NumberOfBytesToProtect, 
		NewAccessProtection, 
		OldAccessProtection);
}

NTSTATUS NTAPI HookedNtAllocateVirtualMemory(
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN PULONG ZeroBits,
	IN OUT PSIZE_T RegionSize,
	IN ULONG AllocationType,
	IN ULONG Protect)
{
	ValidateSystemCallAgainstRop(_NtAllocateVirtualMemory_, BaseAddress, Protect);

	return NtAllocateVirtualMemory(
		ProcessHandle,
		BaseAddress,
		ZeroBits,
		RegionSize,
		AllocationType,
		Protect);
}