#include "stdafx.h"
#include "Hook.h"
#include "Locate.h"
#include "Kernel.h"
#include "RopDetection.h"

void PwnypotDrvUnload(IN PDRIVER_OBJECT DriverObject);
NTSTATUS PwnypotDrvCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PwnypotDrvClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PwnypotDrvDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PwnypotDrvDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP pIrp);

const PCWSTR PwnypotDriverName = L"\\Device\\PwnypotDrv0";
const PCWSTR PwnypotWin32DeviceName = L"\\DosDevices\\PwnypotDrv0";

#ifdef __cplusplus
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING  RegistryPath);
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,IN PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING DeviceName,Win32Device;
	PDEVICE_OBJECT DeviceObject = NULL;

	NTSTATUS status;
	unsigned i;

	RtlInitUnicodeString(&DeviceName, PwnypotDriverName);
	RtlInitUnicodeString(&Win32Device, PwnypotWin32DeviceName);

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = PwnypotDrvDefaultHandler;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = PwnypotDrvCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = PwnypotDrvClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PwnypotDrvDeviceControl;
	
	DriverObject->DriverUnload = PwnypotDrvUnload;
	status = IoCreateDevice(DriverObject,
							0,
							&DeviceName,
							FILE_DEVICE_UNKNOWN,
							0,
							FALSE,
							&DeviceObject);
	if (!NT_SUCCESS(status))
		return status;
	if (!DeviceObject)
		return STATUS_UNEXPECTED_IO_ERROR;

	DeviceObject->Flags |= DO_DIRECT_IO;
	DeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	status = IoCreateSymbolicLink(&Win32Device, &DeviceName);

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	InitializeProcList();

	// TODO: This is just a test
	Locate();
	Hook();

	return STATUS_SUCCESS;
}

void PwnypotDrvUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING Win32Device;

	Unhook();

	DestroyProcList();

	RtlInitUnicodeString(&Win32Device, L"\\DosDevices\\PwnypotDrv0");
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS PwnypotDrvCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS PwnypotDrvClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS PwnypotDrvDefaultHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS PwnypotDrvDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP pIrp)
{
	PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	PVOID pBufIn = pIrp->AssociatedIrp.SystemBuffer;
	PVOID pBufOut = pIrp->AssociatedIrp.SystemBuffer;
	ULONG uBufInSize = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG uBufOutSize = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	NTSTATUS uNtStatus = STATUS_SUCCESS;
	ULONG uOutputLength = 0;

	pIrp->IoStatus.Information = 0;

	KdPrint(("Enter PwnypotDrvDeviceControl().\n"));

	// A pseudo loop
	while(TRUE)
	{
		uOutputLength = PwnypotDispatch(pBufIn, uBufInSize, pBufOut, uBufOutSize);

		break;
	}

	pIrp->IoStatus.Status = uNtStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return uNtStatus;
}