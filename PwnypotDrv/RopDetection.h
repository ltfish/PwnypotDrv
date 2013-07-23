enum ROP_CALLEE
{
	_NtProtectVirtualMemory_,
	_NtAllocateVirtualMemory_
};

typedef struct _PROC_MONITORED
{
	LIST_ENTRY  ListEntry;
	ULONG uPid;
	BOOLEAN bRopFlag;
} PROC_MONITORED, *PPROC_MONITORED;

VOID InitializeProcList();
VOID InsertIntoProcList(ULONG uPid);
VOID RemoveFromProcList(ULONG uPid);
VOID DestroyProcList();
VOID ValidateSystemCallAgainstRop(IN ROP_CALLEE RopCallee, IN PVOID lpAddress, IN ULONG flProtect);