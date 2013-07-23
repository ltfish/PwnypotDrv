typedef struct _SYSTEM_SERVICE_TABLE {
	PVOID *ServiceTable;
	PVOID *CounterTable;
	ULONG ServiceLimit;
	PUCHAR ArgumentTable;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

typedef struct _SERVICE_DESCRIPTOR_TABLE {
	SYSTEM_SERVICE_TABLE Table1;
	SYSTEM_SERVICE_TABLE Table2;
	SYSTEM_SERVICE_TABLE Table3;
	SYSTEM_SERVICE_TABLE Table4;
} SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;

extern "C"
{
	NTKERNELAPI SERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable;
};

#define SSDT_ENTRY_FUNCTION(index) KeServiceDescriptorTable.Table1.ServiceTable[index]

ULONG SsdtRelocateServiceTable();