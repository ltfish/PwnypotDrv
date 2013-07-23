// Dispatch
ULONG PwnypotDispatch(PVOID inData, ULONG inDataLength, PVOID outData, ULONG outDataLength);

// SSDT
VOID SsdtInstallHook(ULONG uServiceNumber, PVOID pNewServiceAddr);
ULONG SsdtUninstallHook(ULONG uServiceNumber);

// Locate
typedef 
NTSTATUS
(NTAPI *NtProtectVirtualMemory__) (
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN OUT PULONG NumberOfBytesToProtect,
	IN ULONG NewAccessProtection,
	OUT PULONG OldAccessProtection
	);

typedef
NTSTATUS
(NTAPI *NtAllocateVirtualMemory__) (
	IN HANDLE ProcessHandle,
	IN OUT PVOID *BaseAddress,
	IN PULONG ZeroBits,
	IN OUT PSIZE_T RegionSize,
	IN ULONG AllocationType,
	IN ULONG Protect
	);

extern NtProtectVirtualMemory__ NtProtectVirtualMemory;
extern NtAllocateVirtualMemory__ NtAllocateVirtualMemory;