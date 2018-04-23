/*++
Module Name:

    hide_files_filter.c

Abstract:

    This is the main module of the hide_files_filter mini filter driver.

Environment:

    Kernel mode

--*/

#include "structures_and_routines.h"
#include "Unit_test.h"

#ifdef DBG
    //#define UNIT_TEST
#endif


#define FILE_PATH   L"\\SystemRoot\\Hide_files_drivers_files"
#define TEMP_PATH   L"\\SystemRoot\\Temp\\"
#define EXPANSION   L".tmp"
//Количество символов, которые нужны для получения 32 битного числа в 16 виде
#define NUMBER_OF_SYMBOLS_FOR_32_BIT_NUMBER_IN_16_BASE	(10)

//---------------------------------------------------------------------------
//      Global variables
//---------------------------------------------------------------------------

HIDE_FILES_FILTER_DATA GlobalData;


/*************************************************************************
    Prototypes for the startup and unload routines used for
    this Filter.

    Implementation
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    );

NTSTATUS
DriverUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
InstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );
NTSTATUS
InstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );
VOID InstanceContextCleanupCallback ( __in PRULE_CONTAINER container, __in FLT_CONTEXT_TYPE ContextType );

FLT_POSTOP_CALLBACK_STATUS
PostOperation_QUERY_DIRECTORY (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );
NTSTATUS
_FltGetVolumeGuidName (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __inout PUNICODE_STRING VolumeGuidName
    );

NTSTATUS
DEVICE_CONTROL_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );
NTSTATUS
always_SUCCESSFULLY_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );
NTSTATUS
CREATE_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );
NTSTATUS
CLEANUP_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );
NTSTATUS
WRITE_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );
NTSTATUS
READ_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    );


//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, init_Global_container)
#pragma alloc_text(INIT, init_Globa_CDO_DATA)
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, Free_Globa_CDO_DATA)
#pragma alloc_text(PAGE, InstanceSetup)
#pragma alloc_text(PAGE, InstanceQueryTeardown)
#pragma alloc_text(PAGE, InstanceContextCleanupCallback)
#pragma alloc_text(PAGE, _FltGetVolumeGuidName)
#pragma alloc_text(PAGE, DEVICE_CONTROL_routine)
#pragma alloc_text(PAGE, CREATE_routine)
#pragma alloc_text(PAGE, CLEANUP_routine)
#pragma alloc_text(PAGE, READ_routine)
#pragma alloc_text(PAGE, WRITE_routine)

#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_DIRECTORY_CONTROL, //IRP_MJ_QUERY_DIRECTORY
      FLTFL_OPERATION_REGISTRATION_SKIP_CACHED_IO | FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO, //флаги показывают, что нужно игнорить фильтр для запросов на достук к файлам кеша и подкачки
      NULL,
      PostOperation_QUERY_DIRECTORY },
	{ IRP_MJ_OPERATION_END }
};  


CONST FLT_CONTEXT_REGISTRATION Contexts[] = {
    { FLT_INSTANCE_CONTEXT,
      0,
      InstanceContextCleanupCallback,
      sizeof(RULE_CONTAINER),
      'tag1',
      NULL,
      NULL,
      NULL },
    { FLT_CONTEXT_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    Contexts,                           //  Context
    Callbacks,                          //  Operation callbacks

    DriverUnload,                       //  FilterUnload

    InstanceSetup,                      //  InstanceSetup //для инициализации FLT_RELATED_OBJECTS (FiDO) руками (расширыть можно)
    InstanceQueryTeardown,	            //  InstanceQueryTeardown //для возможности открепит (FiDO) из PnP стека и удалить данные, что к нему прикреплены были. (если не определить функцию - пользовательский код не сможет откреплять FiDO с помощю FilterDetach или FltDetachVolume )
    NULL,				                //  InstanceTeardownStart //Эта процедура вызывается в начале демонтажа FiDO.
    NULL,                               //  InstanceTeardownComplete //Эта процедура вызывается посте демонтажа FiDO.

    NULL,                               //  GenerateFileName			?
    NULL,                               //  GenerateDestinationFileName	?
    NULL                                //  NormalizeNameComponent		?

};


/*************************************************************************
    Filter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    ){
    NTSTATUS status;
    UNICODE_STRING nameString, linkString;

    UNREFERENCED_PARAMETER( RegistryPath );

    //KdBreakPoint();

    DbgPrint("hide_file_filter!DriverEntry: Entered\n");

    
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CREATE_routine;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CLEANUP_routine;
    DriverObject->MajorFunction[IRP_MJ_READ] = READ_routine;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = WRITE_routine;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DEVICE_CONTROL_routine;

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &GlobalData.gFilterHandle);

    if (!NT_SUCCESS( status )) {
    	DbgPrint("hide_file_filter!DriverEntry: Failure to call FltRegisterFilter\n");
    	return status;
    }

    RtlInitUnicodeString( &nameString, CDO_PATH );
    
    status = IoCreateDevice( DriverObject,
                             0,
                             &nameString,
                             FILE_DEVICE_UNKNOWN,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &GlobalData.CDO);

    if ( !NT_SUCCESS( status ) ) {
        DbgPrint("hide_file_filter: Failure to create CDO. IoCreateDevice failed with status 0x%x. \n", status );
        goto DriverEntry_err_1;
    }

	/*
    При работе с IRP необходимо указать диспетчеру ввода/вывода (IOManager), каким способом будет происходить управление буферами, существуют три вида:
	— буферизованный ввод-вывод (buffered I/O);
	— прямой ввод-вывод (direct I/O);
	— ввод-вывод без управления (neither I/O).
	Установка одного из видов управления осуществляется сразу после создания объекта устройства, то есть после вызова функции IoCreateDevice:
	
	PDEVICE_OBJECT fdo;
	status = IoCreateDevice(..,..,..,..,..,..,&fdo);
	fdo->Flags |= DO_BUFFERED_IO;
	или:
	fdo->Flags |= DO_DIRECT_IO;
	или:
	fdo->Flags |= 0;

	смотри 	https://msdn.microsoft.com/en-us/library/windows/hardware/ff543147(v=vs.85).aspx
			https://msdn.microsoft.com/en-us/library/windows/hardware/ff540663(v=vs.85).aspx
			https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx
	*/

	GlobalData.CDO->Flags |= DO_BUFFERED_IO; //работать только с pIrp->AssociatedIrp.SystemBuffer
	//GlobalData.CDO->Flags |= 0;//работать только с pIrp->AssociatedIrp.SystemBuffer и pIrp->UserBuffer

    RtlInitUnicodeString( &linkString, CDO_SYMBOLIC_LINK_PATH );

    status = IoCreateSymbolicLink( &linkString, &nameString );
    if (status == STATUS_OBJECT_NAME_COLLISION){
    	IoDeleteSymbolicLink(&linkString);
    	status = IoCreateSymbolicLink( &linkString, &nameString );
    }
    if ( !NT_SUCCESS( status ) ) {
        DbgPrint("hide_file_filter: Failure to create CreateSymbolicLink for CDO. IoCreateSymbolicLink failed with status 0x%x. \n", status );
        goto DriverEntry_err_2;
    }

    DbgPrint("CDO_NAME is %wZ \n", &nameString);

    {
    	OBJECT_ATTRIBUTES oa;
    	UNICODE_STRING name;
    	IO_STATUS_BLOCK IoStatusBlock;
    	RtlInitUnicodeString(&name, FILE_PATH);
    	InitializeObjectAttributes(&oa, &name, OBJ_CASE_INSENSITIVE /*| OBJ_KERNEL_HANDLE*/, NULL, NULL);
        status = ZwCreateFile(&GlobalData.driver_directory, GENERIC_READ | GENERIC_WRITE , &oa, &IoStatusBlock,
        						NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF,
        						FILE_OPEN_FOR_BACKUP_INTENT | FILE_DIRECTORY_FILE, NULL, 0);


        if (!NT_SUCCESS( status )) {
			DbgPrint("hide_file_filter!DriverEntry: Failure to init driver_directory\n");
			goto DriverEntry_err_3;
        } else {
        	status = init_Global_container();

        	if (!NT_SUCCESS( status )) {
    			DbgPrint("hide_file_filter!DriverEntry: Failure to call init_Global_container\n");
    			goto DriverEntry_err_4;
    		} else {
    			status = init_Globa_CDO_DATA();

        		if (!NT_SUCCESS( status )) {
    				DbgPrint("hide_file_filter!DriverEntry: Failure to call init_Globa_CDO_DATA\n");
    				goto DriverEntry_err_5;
    			} else {
    				status = PsSetCreateProcessNotifyRoutine(PROCESS_NOTIFY_ROUTINE, FALSE);

    				if (!NT_SUCCESS( status )) {
    					DbgPrint("hide_file_filter!DriverEntry: Failure to call PsSetCreateProcessNotifyRoutine\n");
    					goto DriverEntry_err_6;
    				} else {
			    	    status = FltStartFiltering( GlobalData.gFilterHandle );

		    	    	if (!NT_SUCCESS( status )) {
							DbgPrint("hide_file_filter!DriverEntry: Failure to call FltStartFiltering");
							goto DriverEntry_err_7;
		    		    } else {
		        			DbgPrint("hide_file_filter!DriverEntry: GOOD");
		        			GlobalData.make_filtering = TRUE;
		    	    	}
		    		}
	    		}
	    	}
        }
    }

    return status;

    DriverEntry_err_7:
    	PsSetCreateProcessNotifyRoutine(PROCESS_NOTIFY_ROUTINE, TRUE);
    DriverEntry_err_6:
    	Free_Globa_CDO_DATA();
    DriverEntry_err_5:
    	clear_RULE_CONTAINER(&GlobalData.container);
    DriverEntry_err_4:
    	ZwClose(GlobalData.driver_directory);
    DriverEntry_err_3:
    	IoDeleteSymbolicLink(&linkString);
    DriverEntry_err_2:
    	IoDeleteDevice(GlobalData.CDO);
    DriverEntry_err_1:
    	FltUnregisterFilter( GlobalData.gFilterHandle );

    return status;
}

NTSTATUS
DriverUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    ){
	UNICODE_STRING linkString;
	//KdBreakPoint();
    UNREFERENCED_PARAMETER( Flags );
    PAGED_CODE();
	

	DbgPrint("hide_file_filter!DriverUnload: Entered");

	RtlInitUnicodeString( &linkString, CDO_SYMBOLIC_LINK_PATH );


    
    PsSetCreateProcessNotifyRoutine(PROCESS_NOTIFY_ROUTINE, TRUE);
    save_rules(&GlobalData.container);
    Free_Globa_CDO_DATA();
    clear_RULE_CONTAINER(&GlobalData.container);
	IoDeleteSymbolicLink(&linkString);
    IoDeleteDevice(GlobalData.CDO);
    FltUnregisterFilter( GlobalData.gFilterHandle );
    ZwClose(GlobalData.driver_directory);

    //KdBreakPoint();
    return STATUS_SUCCESS;
}

NTSTATUS
init_volume_parameters_in_RULE_CONTAINER (__in PCFLT_RELATED_OBJECTS FltObjects, __out PRULE_CONTAINER container){
    NTSTATUS status = STATUS_SUCCESS;
    ULONG BufferSizeNeeded = 0;
    PAGED_CODE();

    //KdBreakPoint();

    status = FltGetVolumeGuidName(FltObjects->Volume, &container->GUID, &BufferSizeNeeded);
    if (status == STATUS_BUFFER_TOO_SMALL){
    	ClearUNIC_STR(&container->GUID);
    	if (!(container->GUID.Buffer = GetMemory(BufferSizeNeeded))) return STATUS_MEMORY_NOT_ALLOCATED;
    	container->GUID.MaximumLength = (USHORT)BufferSizeNeeded;
    	container->GUID.Length = 0;
    	status = FltGetVolumeGuidName(FltObjects->Volume, &container->GUID, NULL);
    }

    if (!NT_SUCCESS( status )) {
        DbgPrint("hide_file_filter!%s: FltGetVolumeGuidName returned 0x%08x!\n", __FUNCTION__, status);
        goto init_volume_parameters_in_RULE_CONTAINER_err_1;
    }
    REMOVE_SLASHES_AND_QUESTION_MARKS(&container->GUID);
    TO_UPPERCASE(&container->GUID);
    
    //KdBreakPoint();
    
    status = FltGetVolumeName(FltObjects->Volume, &container->root_folder.data.key_, &BufferSizeNeeded);
    if (status == STATUS_BUFFER_TOO_SMALL){
    	ClearUNIC_STR(&container->root_folder.data.key_);
    	if (!(container->root_folder.data.key_.Buffer = GetMemory(BufferSizeNeeded))) return STATUS_MEMORY_NOT_ALLOCATED;
    	container->root_folder.data.key_.MaximumLength = (USHORT)BufferSizeNeeded;
    	container->root_folder.data.key_.Length = 0;
    	status = FltGetVolumeName(FltObjects->Volume, &container->root_folder.data.key_, NULL);
    }

    if (!NT_SUCCESS( status )) {
        DbgPrint("hide_file_filter!%s: FltGetVolumeGuidName returned 0x%08x!\n", __FUNCTION__, status);
        goto init_volume_parameters_in_RULE_CONTAINER_err_1;
    }

    REMOVE_SLASHES_AND_QUESTION_MARKS(&container->root_folder.data.key_);
    TO_UPPERCASE(&container->root_folder.data.key_);

    return status;

    init_volume_parameters_in_RULE_CONTAINER_err_1:
    ClearUNIC_STR(&container->GUID);
    return status;
}

NTSTATUS
InstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    ){

    NTSTATUS status = STATUS_SUCCESS;
    PRULE_CONTAINER Context = NULL;
    RULE_CONTAINER __Context;

	//KdBreakPoint();
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    DbgPrint("hide_file_filter!InstanceSetup: Entered\n");

    //KdBreakPoint();
    status = init_RULE_CONTAINER(&__Context);
    if (!NT_SUCCESS( status )) goto InstanceSetup_err_1;

    status = init_volume_parameters_in_RULE_CONTAINER(FltObjects, &__Context);
    if (!NT_SUCCESS( status )) goto InstanceSetup_err_2;

    if (VolumeFilesystemType == FLT_FSTYPE_FAT) __Context.path_is_correct = path_is_correct_VFAT;
    else 										__Context.path_is_correct = path_is_correct_NTFS;

    status = fill_RULE_CONTAINER(&__Context);
    if (!NT_SUCCESS( status )) goto InstanceSetup_err_2;

    status = FltAllocateContext(GlobalData.gFilterHandle, //FltObjects->Instance
                                FLT_INSTANCE_CONTEXT,
                                sizeof(RULE_CONTAINER),
                                NonPagedPool,
                                &Context);

    if (NT_SUCCESS( status ) && Context != NULL) RtlZeroMemory(Context, sizeof(RULE_CONTAINER));
   	else goto InstanceSetup_err_2;

    status = FltSetInstanceContext (FltObjects->Instance,
                                    FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                    Context,
                                    NULL);
    
    //FltReleaseContext(Context); //FltReferenceContext(Context);
    
    if (!NT_SUCCESS( status )) goto InstanceSetup_err_3;
    
    ACTIONS_PRE_WRITING(&GlobalData.CDO_data.local_rules.var_for_sync);
    RtlCopyMemory(Context, &__Context, sizeof(RULE_CONTAINER));
    if (!MY_FLT_INSTANCE_CONTEXT_insert_element(Context, &GlobalData.CDO_data.local_rules.data)){
    	status = STATUS_MEMORY_NOT_ALLOCATED;
    	RtlZeroMemory(Context, sizeof(RULE_CONTAINER));
    	goto InstanceSetup_err_4;
    }
    ACTIONS_POST_WRITING(&GlobalData.CDO_data.local_rules.var_for_sync);

    #ifdef UNIT_TEST
        //KdBreakPoint();
        if (Unit_test_2(Context)){
           DbgPrint("hide_file_filter!init: Unit_test2 passed.\n");
        } else {
            DbgPrint("hide_file_filter!init: Unit_test2 WAS NOT PASSED!!!!!!!!!\n");
        }
    #endif

    #ifdef DBG
    if (VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {
    	DbgPrint("FILE_DEVICE_NETWORK_FILE_SYSTEM\n");
       //return STATUS_FLT_DO_NOT_ATTACH;
    }

    if (VolumeDeviceType == FILE_DEVICE_CD_ROM_FILE_SYSTEM) {
    	DbgPrint("FILE_DEVICE_CD_ROM_FILE_SYSTEM\n");
    }

	if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM) {
    	DbgPrint("FILE_DEVICE_DISK_FILE_SYSTEM\n");
    }
    #endif

    return STATUS_SUCCESS;

    InstanceSetup_err_4:
        FltReleaseContext(Context);
    InstanceSetup_err_3:
        FltReleaseContext(Context);
    InstanceSetup_err_2:
        clear_RULE_CONTAINER(&__Context);
    InstanceSetup_err_1:
        return STATUS_FLT_DO_NOT_ATTACH;
}

NTSTATUS
InstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    ){//запрос на снятие экземпляра (FiDO) (Instance - экземпляр, Teardown - срыв (снятие))
	PRULE_CONTAINER Context = NULL;
	KdBreakPoint();
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    DbgPrint("hide_file_filter!InstanceQueryTeardown: Entered\n");
    PAGED_CODE();

    FltGetInstanceContext (FltObjects->Instance, &Context);

    ACTIONS_PRE_WRITING(&GlobalData.CDO_data.local_rules.var_for_sync);
    MY_FLT_INSTANCE_CONTEXT_delete_element(Context, &GlobalData.CDO_data.local_rules.data);
    ACTIONS_POST_WRITING(&GlobalData.CDO_data.local_rules.var_for_sync);

    FltReleaseContext(Context);

    return STATUS_SUCCESS;
}
VOID InstanceContextCleanupCallback ( __in PRULE_CONTAINER container, __in FLT_CONTEXT_TYPE ContextType )
{
    UNREFERENCED_PARAMETER( ContextType );
    PAGED_CODE();
    //KdBreakPoint();
    ASSERT( ContextType == FLT_INSTANCE_CONTEXT );

    if (container->GUID.Buffer != NULL){
    	save_rules(container);
    }

    clear_RULE_CONTAINER(container);
}


//______________________________________________________________________________________________________________________________________________________
//______________________________________________________________________________________________________________________________________________________
//фильтрация

//проверяет infoClass на соответствие предполагаемым и записывает в PFileName имя файла (не надо потом освобождать память по укзателю FileName.Buffer) (по указателю PFileName уже должна существовать структура UNICODE_STRING)
BOOLEAN _Get_file_name(__in FILE_INFORMATION_CLASS infoClass, __in PVOID DirectoryBuffer, __inout PUNICODE_STRING PFileName){
	ASSERT(PFileName != NULL);
	if (DirectoryBuffer == NULL) return FALSE;
	#define GET_FILE_NAME(infoClass)                                                       \
		PFileName->Length = (USHORT)(((infoClass*)DirectoryBuffer)->FileNameLength);       \
		PFileName->MaximumLength = PFileName->Length;                                      \
        PFileName->Buffer = ((infoClass*)DirectoryBuffer)->FileName;                       \
        return TRUE;
    switch(infoClass)
    {
    case FileBothDirectoryInformation:
        GET_FILE_NAME(FILE_BOTH_DIR_INFORMATION);
 
    case FileDirectoryInformation:
       	GET_FILE_NAME(FILE_DIRECTORY_INFORMATION);
 
    case FileFullDirectoryInformation:
       	GET_FILE_NAME(FILE_FULL_DIR_INFORMATION);
 
    case FileIdBothDirectoryInformation:
       	GET_FILE_NAME(FILE_ID_BOTH_DIR_INFORMATION);
 
    case FileIdFullDirectoryInformation:
       	GET_FILE_NAME(FILE_ID_FULL_DIR_INFORMATION);
 
    case FileNamesInformation:
       	GET_FILE_NAME(FILE_NAMES_INFORMATION);
 
    default:
        return FALSE;
    }
}
USHORT _Get_sizeof_INFORMATION_struct(FILE_INFORMATION_CLASS infoClass){
        switch(infoClass)
        {
        case FileBothDirectoryInformation:
        	return sizeof(FILE_BOTH_DIR_INFORMATION);
 
        case FileDirectoryInformation:
        	return sizeof(FILE_DIRECTORY_INFORMATION);
 
        case FileFullDirectoryInformation:
        	return sizeof(FILE_FULL_DIR_INFORMATION);
 
        case FileIdBothDirectoryInformation:
        	return sizeof(FILE_ID_BOTH_DIR_INFORMATION);
 
        case FileIdFullDirectoryInformation:
        	return sizeof(FILE_ID_FULL_DIR_INFORMATION);
 
        case FileNamesInformation:
        	return sizeof(FILE_NAMES_INFORMATION);
 
        default:
            return 0;
        }
}
USHORT _get_max_file_name_len(FILE_INFORMATION_CLASS infoClass, PVOID currentEntry){
    ULONG res = 0;
 #define ___FOR_MAX_FILE_NAME(currentEntry)                                                 \
    res = (currentEntry)->FileNameLength;                                                   \
    while((currentEntry)->NextEntryOffset){                                                 \
        ((char*)currentEntry) = ((char*)(currentEntry)) + (currentEntry)->NextEntryOffset;  \
        if (res < (currentEntry)->FileNameLength){                                          \
            res = (currentEntry)->FileNameLength;                                           \
        }                                                                                   \
    }                                                                                       \
    return (USHORT)res;

        switch(infoClass)
        {
        case FileBothDirectoryInformation:
            ___FOR_MAX_FILE_NAME((PFILE_BOTH_DIR_INFORMATION)currentEntry);
 
        case FileDirectoryInformation:
            ___FOR_MAX_FILE_NAME((PFILE_DIRECTORY_INFORMATION)currentEntry);
             
        case FileFullDirectoryInformation:
            ___FOR_MAX_FILE_NAME((PFILE_FULL_DIR_INFORMATION)currentEntry);
 
        case FileIdBothDirectoryInformation:
            ___FOR_MAX_FILE_NAME((PFILE_ID_BOTH_DIR_INFORMATION)currentEntry);
 
        case FileIdFullDirectoryInformation:
            ___FOR_MAX_FILE_NAME((PFILE_ID_FULL_DIR_INFORMATION)currentEntry);
 
        case FileNamesInformation:
            ___FOR_MAX_FILE_NAME((PFILE_NAMES_INFORMATION)currentEntry);
 
        default:
            return 0;
        }
}


FLT_POSTOP_CALLBACK_STATUS
PostOperation_QUERY_DIRECTORY (__inout PFLT_CALLBACK_DATA Data, __in PCFLT_RELATED_OBJECTS FltObjects, __in_opt PVOID CompletionContext, __in FLT_POST_OPERATION_FLAGS Flags){
    //KdBreakPoint();

    UNREFERENCED_PARAMETER( Flags );
    
    DbgPrint("hide_file_filter!PostOperation_QUERY_DIRECTORY: Entered");

    if (Data->RequestorMode == KernelMode){
    	DbgPrint("hide_file_filter!PostOperation_QUERY_DIRECTORY: KernelMode");
    	return FLT_POSTOP_FINISHED_PROCESSING;
    }
    #ifdef DBG
    else DbgPrint("hide_file_filter!PostOperation_QUERY_DIRECTORY: UserMode");
    #endif

    if(!NT_SUCCESS(Data->IoStatus.Status)){
	    return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if(Data->Iopb->MinorFunction != IRP_MN_QUERY_DIRECTORY){
    	return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer == NULL){
    	return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if(KeGetCurrentIrql() != PASSIVE_LEVEL){
    	return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if(!GlobalData.make_filtering){
    	return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if(is_preferred_sync((PID)FltGetRequestorProcessId(Data))){
    	return FLT_POSTOP_FINISHED_PROCESSING;
	}

	/* FltGetRequestorProcessId  https://msdn.microsoft.com/en-us/library/windows/hardware/ff543118(v=vs.85).aspx */
	
	{
		UNICODE_STRING path_without_volume;
		PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

		NTSTATUS status = FltGetFileNameInformation( Data,	FLT_FILE_NAME_NORMALIZED /*FLT_FILE_NAME_OPENED*/ | 
		/*FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY*/ FLT_FILE_NAME_QUERY_DEFAULT /*FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP*/, &nameInfo );

		#ifdef DBG
		if (status == STATUS_FLT_INVALID_NAME_REQUEST){
			DbgPrint("STATUS_FLT_INVALID_NAME_REQUEST");
		}
		if (status == STATUS_INSUFFICIENT_RESOURCES){
			DbgPrint("STATUS_INSUFFICIENT_RESOURCES");
		}
		if (status == STATUS_INVALID_PARAMETER){
			DbgPrint("STATUS_INVALID_PARAMETER");
		}
		if (status == STATUS_FLT_NAME_CACHE_MISS){
			DbgPrint("STATUS_FLT_NAME_CACHE_MISS");
		}
		if (status == STATUS_ACCESS_DENIED){
			DbgPrint("STATUS_ACCESS_DENIED");
		}
		#endif

		if (!NT_SUCCESS(status)){
			return FLT_POSTOP_FINISHED_PROCESSING;
		}

		path_without_volume.Buffer = (WCHAR*)((char*)nameInfo->Name.Buffer + nameInfo->Volume.Length + sizeof(L'\\'));
		path_without_volume.Length = nameInfo->Name.Length - nameInfo->Volume.Length - sizeof(L'\\');
		path_without_volume.MaximumLength = path_without_volume.Length;

		DbgPrint("________________\n");
		DbgPrint("%wZ\n", &nameInfo->Volume);
		DbgPrint("%wZ\n", &path_without_volume);
		DbgPrint("________________\n");

		//KdBreakPoint();
		{
			UNICODE_STRING fileName;
			PVOID currentEntry = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer, previousEntry = NULL;
			FILE_INFORMATION_CLASS infoClass = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
			USHORT sizeof_INFORMATION_struct = _Get_sizeof_INFORMATION_struct(infoClass);
            USHORT max_file_name_len = _get_max_file_name_len(infoClass, currentEntry);
            UNICODE_STRING full_fileName;//полное имя файла (без имени диска)
            PRULE_CONTAINER Context = NULL;

            status = FltGetInstanceContext (FltObjects->Instance, &Context);
            if (!NT_SUCCESS( status ) || Context == NULL) goto PostOperation_QUERY_DIRECTORY_err_1;

            full_fileName.MaximumLength = path_without_volume.MaximumLength + max_file_name_len + sizeof(L'\\');
            full_fileName.Buffer = GetMemory(full_fileName.MaximumLength);
            if (full_fileName.Buffer){
                RtlCopyMemory(full_fileName.Buffer, path_without_volume.Buffer, path_without_volume.Length);
                full_fileName.Buffer[path_without_volume.Length / sizeof(WCHAR)] = L'\\';
			    while (_Get_file_name( infoClass, currentEntry, &fileName)){
                    RtlCopyMemory((char*)full_fileName.Buffer + path_without_volume.Length + sizeof(L'\\'), fileName.Buffer, fileName.Length);//получили полное имя файла (без имени диска)
                    full_fileName.Length = path_without_volume.Length + fileName.Length + sizeof(L'\\');
			    	
                    if(Must_be_hidden_sync(&path_without_volume, &fileName, &full_fileName, &GlobalData.container)
                     || Must_be_hidden_sync(&path_without_volume, &fileName, &full_fileName, Context)){
			    		DbgPrint("%wZ | hiding.\n", &fileName);
			    		{//"убираем" файл из списка файлов (память не трогаем)
			    			//KdBreakPoint();
							if(!previousEntry){ // first entry
    		            		if(!*((ULONG*)currentEntry)){
		                        	//      у нас единственная запись и она должна быть удалена
    		                    	//      завершаем со статусом STATUS_NO_MORE_FILES
	        		                currentEntry = NULL;

		        	                Data->IoStatus.Status = STATUS_NO_MORE_FILES;
 		
				                } else { // это первая и не последняя запись
				                	//переписываем байты
				                	PVOID nextEntry = (char*)currentEntry + *((ULONG*)currentEntry);
 									char *ptr1 = (char*)currentEntry + sizeof(ULONG) /*поле NextEntryOffset пропускаем*/, *ptr2 = (char*)nextEntry + sizeof(ULONG) /*поле NextEntryOffset пропускаем*/, *end2;
 									UNICODE_STRING next_fileName;
 									*((ULONG*)currentEntry) += *((ULONG*)nextEntry); /*поправляем значение поля NextEntryOffset*/
 									_Get_file_name(infoClass, nextEntry, &next_fileName);//просто, чтоб узнать длину строки в nextEntry
 									end2 = ptr2 + sizeof_INFORMATION_struct + next_fileName.Length
 									- sizeof(ULONG) /*поле NextEntryOffset пропускаем*/
 									- sizeof(WCHAR) /*это количество байт вхлдит в  как первый символ (смотри описание структур таких как FILE_BOTH_DIR_INFORMATION)*/
 									;

            			            while (ptr2 != end2){
            			            	*ptr1++ = *ptr2++;
            			            }
		                		}
					        } else { // not first entry
			        	        ULONG shift = *((ULONG*)currentEntry);
		    	    	        if (shift){
		        		        	*((ULONG*)previousEntry) += shift;
		        		        	//currentEntry = ((char*)currentEntry) + shift;
		        	    	    	(char*)currentEntry += shift;
		        	        	} else {
			        	        	*((ULONG*)previousEntry) = 0;
			        	        	currentEntry = NULL;
			        	        }
					        }
        				}
			    	} else {
			    		DbgPrint("%wZ\n", &fileName);
			    		if (*((ULONG*)currentEntry)){
			    			previousEntry = currentEntry;
			    			(char*)currentEntry += *((ULONG*)currentEntry);
			    		} else {
			    			currentEntry = NULL;
			    			//break;
			    		}
			    	}
	    	    }
            }

            FltReleaseContext(Context);
            FreeMemory(full_fileName.Buffer);
			goto PostOperation_QUERY_DIRECTORY_OK_1;

            PostOperation_QUERY_DIRECTORY_err_1:;

            PostOperation_QUERY_DIRECTORY_OK_1:;
		}
        
        FltReleaseFileNameInformation(nameInfo);

	}

    return FLT_POSTOP_FINISHED_PROCESSING;
}

//______________________________________________________________________________________________________________________________________________________
//______________________________________________________________________________________________________________________________________________________
/*именованное устройство

IRP_MJ_DEVICE_CONTROL      
"
	а для именованого девайса (или как то так) нужно будет 
	сreate/close - пустышки, просто завершаются успешно, что бы user mode мог открыть девайс.
	ну и device control - что бы user mode мог посылать запросы девайсу
"

*/

/*

https://msdn.microsoft.com/en-us/library/windows/hardware/ff559935(v=vs.85).aspx

https://msdn.microsoft.com/en-us/library/windows/hardware/ff548630(v=vs.85).aspx

"а преобразования, см. ZwQuerySymbolicLinkObject"
*/

NTSTATUS
CREATE_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    ){
	NTSTATUS status = STATUS_SUCCESS;
	PGUI_CONTEXT GUI;

    UNREFERENCED_PARAMETER( pDeviceObject );
    //KdBreakPoint();

    DbgPrint("hide_file_filter!%s: Entered!\n", __FUNCTION__);

    GUI = ExAllocatePool(PagedPool, sizeof(GUI_CONTEXT));
    if (!GUI){
    	//если возвращать плохой статус, то нужно что-то писать в Irp (Irp->IoStatus), чтоб вызываемой программе система вернула INVALID_HANDLE_VALUE?
    	status = STATUS_MEMORY_NOT_ALLOCATED;
    	goto CREATE_routine_err_1;
    }
    GUI->pid = PsGetCurrentProcessId();
    GUI->count = 0;
    GUI->messages.first = NULL;
    GUI->messages.last = NULL;
	
	KeInitializeMutex(&GUI->mutex_for_messages, 0);
	KeInitializeEvent(&GUI->new_message, SynchronizationEvent/*NotificationEvent*/, FALSE);

	{//вставляем GUI в контейнер для GUI
		PVOID ptr;
		ACTIONS_PRE_WRITING(&GlobalData.CDO_data.GUIs.var_for_sync);
		ptr = PGUI_CONTEXT_insert_element(GUI, &GlobalData.CDO_data.GUIs.data);
		ACTIONS_POST_WRITING(&GlobalData.CDO_data.GUIs.var_for_sync);
		if (!ptr){
			status = STATUS_MEMORY_NOT_ALLOCATED;
			goto CREATE_routine_err_2;
		}
	}

	
	
	pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//Irp->IoStatus = FILE_OPENED; //(это нужно?)
    return STATUS_SUCCESS;

    CREATE_routine_err_2:
    FreeMemory(GUI);
    CREATE_routine_err_1:

    pIrp->IoStatus.Status = status;
    return status;
}

// https://msdn.microsoft.com/en-us/library/windows/hardware/ff550718(v=vs.85).aspx
NTSTATUS
CLEANUP_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    ){
	HANDLE pid;
    UNREFERENCED_PARAMETER( pDeviceObject );
    UNREFERENCED_PARAMETER( pIrp );

    //KdBreakPoint();
    DbgPrint("hide_file_filter!%s: Entered!\n", __FUNCTION__);

    pid = PsGetCurrentProcessId();

	{//удаляем контекст GUI из контейнера для GUI
		ACTIONS_PRE_WRITING(&GlobalData.CDO_data.GUIs.var_for_sync);
		PGUI_CONTEXT_delete_element((PGUI_CONTEXT)&pid, &GlobalData.CDO_data.GUIs.data);
		ACTIONS_POST_WRITING(&GlobalData.CDO_data.GUIs.var_for_sync);
	}
	// ZwQueryDirectoryFile чтоб удалить файлы в папке temp https://msdn.microsoft.com/en-us/library/windows/hardware/ff567047(v=vs.85).aspx
	// ZwDeleteFile 										https://msdn.microsoft.com/en-us/library/windows/hardware/ff566435(v=vs.85).aspx

	pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}





NTSTATUS get_container(PUNICODE_STRING volume_name, PRULE_CONTAINER *container){
	NTSTATUS status = STATUS_SUCCESS;
	PNODE(PRULE_CONTAINER) pnode;
	if (!(pnode = MY_FLT_INSTANCE_CONTEXT_finde((PRULE_CONTAINER)volume_name, &GlobalData.CDO_data.local_rules.data))){
	   	HANDLE LinkHandle;
    	OBJECT_ATTRIBUTES oa;
	   	ULONG ReturnedLength;
	   	UNICODE_STRING real_volume_name;
   		InitializeObjectAttributes(&oa, volume_name, OBJ_CASE_INSENSITIVE, NULL, NULL);
   		status = ZwOpenSymbolicLinkObject(&LinkHandle, GENERIC_READ, &oa);
    	if (!NT_SUCCESS(status)){
    		status = IO_ERR_INVALID_VOLUME_NAME;
	   		goto get_container_err_1;
    	}
    	RtlZeroMemory(&real_volume_name, sizeof(real_volume_name));
    	status = ZwQuerySymbolicLinkObject(LinkHandle, &real_volume_name, &ReturnedLength);
	   	if (status == STATUS_BUFFER_TOO_SMALL){
    		if (!(real_volume_name.Buffer = GetMemory(ReturnedLength))){
    			status = STATUS_MEMORY_NOT_ALLOCATED;
    			goto get_container_err_2;
    		}
	   		real_volume_name.MaximumLength = (USHORT)ReturnedLength;
    		real_volume_name.Length = 0;
    		status = ZwQuerySymbolicLinkObject(LinkHandle, &real_volume_name, NULL);
    	}
    	if (!NT_SUCCESS(status)) goto get_container_err_3;

    	REMOVE_SLASHES_AND_QUESTION_MARKS(&real_volume_name);

    	if (!(pnode = MY_FLT_INSTANCE_CONTEXT_finde((PRULE_CONTAINER)&real_volume_name, &GlobalData.CDO_data.local_rules.data))){
    		status = IO_ERR_INVALID_VOLUME_NAME;
    		goto get_container_err_3;
    	}

    	ClearUNIC_STR(&real_volume_name);
    	ZwClose(LinkHandle);
    	goto get_container_OK_1;
	
		get_container_err_3:
   		ClearUNIC_STR(&real_volume_name);
   		get_container_err_2:
   		ZwClose(LinkHandle);
   		goto get_container_err_1;
   	}

   	get_container_OK_1:
   	*container = pnode->key_;
   	return STATUS_SUCCESS;

   	get_container_err_1:
   	return status;
}

//   http://blagin.ru/sozdanie-drajverov-chast-4-obmen-dannymi/
/*

	для GUI http://www.outsidethebox.ms/12317/#sel= 
			https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724454(v=vs.85).aspx

	//пример
	#include<Windows.h>
	int main(){
		char ptr[100];
		unsigned long length = GetWindowsDirectory(ptr, sizeof(ptr));
		if (length == 0 || length > 70) return 1;
		.........
		strcpy_s(ptr + length, sizeof(ptr) - length, "\\temp\\somefile");
		.........
	}
*/
NTSTATUS
DEVICE_CONTROL_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    ){
	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pIrp);
    ULONG ControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;
  
    ULONG InputLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG OutputLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ULONG AnswerLength = 0;
    NTSTATUS status = STATUS_SUCCESS;

    if (OutputLength > MAX_CMD_LENGTH){
    	status = IO_ERR_BUFFER_TOO_BIG;
    	goto DEVICE_CONTROL_routine_err_1;
    }

    UNREFERENCED_PARAMETER( pDeviceObject );
    
    //KdBreakPoint();

    DbgPrint("hide_file_filter!%s: Entered!\n", __FUNCTION__);
  

    #if DBG
        DbgPrint("IoControlCode IOCTL_FROM_CORE: %d\n",ControlCode);
        DbgPrint("TestControl(InputLength): %d\n",InputLength);
        DbgPrint("TestControl(OutputLength): %d\n",OutputLength);
        DbgPrint("TestControl(pIrp->AssociatedIrp.SystemBuffer): %S\n",pIrp->AssociatedIrp.SystemBuffer);
    #endif

    if (ControlCode == D_3_CTL){
        AnswerLength = sizeof(GlobalData.make_filtering);
    	if (OutputLength < AnswerLength){
	       	status = STATUS_BUFFER_TOO_SMALL;
    	   	goto DEVICE_CONTROL_routine_err_1;
    	}
    	RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &GlobalData.make_filtering, AnswerLength);
    } else {
    	UNICODE_STRING path;
  		UNICODE_STRING volume_name;
   		USHORT i;

    	//путь к парке так <имя диска:>\......\     (в конце стоит слеш)
   		//для глобальных правил имя диска <имя диска> = *:
		//KdBreakPoint();
   		volume_name.Buffer = pIrp->AssociatedIrp.SystemBuffer;

   		for (i = 0; i < InputLength / sizeof(WCHAR); i++){
    		if (volume_name.Buffer[i] == SEPARATOR) break;
   		}

   		if (i == InputLength / sizeof(WCHAR)){
    		status = IO_ERR_NO_SEPARATOR;
   			goto DEVICE_CONTROL_routine_err_1;
   		}

    	volume_name.MaximumLength = (i + 1) * sizeof(WCHAR);
   		volume_name.Length = volume_name.MaximumLength;

   		path.Buffer = (WCHAR*)((char*)volume_name.Buffer + volume_name.MaximumLength);
    	path.MaximumLength = (USHORT)(InputLength - volume_name.MaximumLength);
   		path.Length = path.MaximumLength;

   		ACTIONS_PRE_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
   		{
   			PRULE_CONTAINER container = NULL;
   			status = get_container(&volume_name, &container);
   			if (!NT_SUCCESS(status)) goto DEVICE_CONTROL_routine_err_2_1;
    		
    		ACTIONS_PRE_READING(&container->root_folder.var_for_sync);
    		if (ControlCode == D_2_CTL){
    			BOOLEAN will_be_hiding;
    			UNICODE_STRING fileName, folder;

    			AnswerLength = sizeof(will_be_hiding);
    			if (OutputLength < AnswerLength){
	       			status = STATUS_BUFFER_TOO_SMALL;
    	   			goto DEVICE_CONTROL_routine_err_2_2;
    			}
    			if(!container->path_is_correct(&path, _NAME_)){
    				status = IO_ERR_INVALID_PATH;
   					goto DEVICE_CONTROL_routine_err_2_2;
   				}

   				//if (GlobalData.make_filtering){
	   				fileName.Buffer = (WCHAR*)((char*)path.Buffer + path.Length - sizeof(WCHAR));
   					while (*fileName.Buffer != L'\\') fileName.Buffer--;
   					fileName.Buffer++;
   					fileName.Length = path.Length - (USHORT)((char*)fileName.Buffer - (char*)path.Buffer);
   					fileName.MaximumLength = fileName.Length;

	   				folder.Buffer = path.Buffer;
   					folder.Length = path.Length - fileName.Length;
   					folder.MaximumLength = folder.Length;

   					will_be_hiding = Must_be_hidden(&folder, &fileName, &path, &container->root_folder.data);
   				//} else will_be_hiding = FALSE;

   				RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &will_be_hiding, AnswerLength);
    		} else {
    			PFOLDER pfolder;
    			KdBreakPoint();
    			if(!container->path_is_correct(&path, _FOLDER_)){
    				status = IO_ERR_INVALID_PATH;
   					goto DEVICE_CONTROL_routine_err_2_2;
   				}

    			if (!(pfolder = finde_rule(&path, &container->root_folder.data, NULL))){
    				status = IO_ERR_INVALID_PATH;
    				goto DEVICE_CONTROL_routine_err_2_2;
    			}
    			if (ControlCode == D_1_1_FOLDER_CTL || ControlCode == D_1_1_NAME_CTL || ControlCode == D_1_1_MASK_CTL){
    				SIZE_T size = 0;
        			AnswerLength = sizeof(size);
    	    		if (OutputLength < AnswerLength){
	    	    		status = STATUS_BUFFER_TOO_SMALL;
    		    		goto DEVICE_CONTROL_routine_err_2_2;
    		    	}
    		    	if (ControlCode == D_1_1_FOLDER_CTL) PFOLDER_get_pnode_where_condition_is_true(&size, &pfolder->subfolders, sum_size_FOLDER_names_in_tree);
    		    	else if (ControlCode == D_1_1_NAME_CTL) UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->hiding_names, sum_size_UNIC_STR_records_in_tree);
    		    	else UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->masks, sum_size_UNIC_STR_records_in_tree);
    		    	RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, &size, AnswerLength);
    			} else if (ControlCode == D_1_2_FOLDER_CTL || ControlCode == D_1_2_NAME_CTL || ControlCode == D_1_2_MASK_CTL){
    				SIZE_T size = 0;
    				WCHAR* Buffer = pIrp->AssociatedIrp.SystemBuffer;
    		    	if (ControlCode == D_1_2_FOLDER_CTL) PFOLDER_get_pnode_where_condition_is_true(&size, &pfolder->subfolders, sum_size_FOLDER_names_in_tree);
    		    	else if (ControlCode == D_1_2_NAME_CTL) UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->hiding_names, sum_size_UNIC_STR_records_in_tree);
    		    	else UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->masks, sum_size_UNIC_STR_records_in_tree);

    		    	AnswerLength = size;
    	    		if (OutputLength < AnswerLength){
	    	    		status = STATUS_BUFFER_TOO_SMALL;
    		    		goto DEVICE_CONTROL_routine_err_2_2;
    		    	}

    		    	if (ControlCode == D_1_2_FOLDER_CTL) PFOLDER_get_pnode_where_condition_is_true(&Buffer, &pfolder->subfolders, write_FOLDER_names_to_buffer);
    		    	else if (ControlCode == D_1_2_NAME_CTL) UNIC_STR_get_pnode_where_condition_is_true(&Buffer, &pfolder->hiding_names, write_UNIC_STR_names_to_buffer);
    		    	else UNIC_STR_get_pnode_where_condition_is_true(&Buffer, &pfolder->masks, write_UNIC_STR_names_to_buffer);
    			} else if (ControlCode == D_1_3_FOLDER_CTL || ControlCode == D_1_3_NAME_CTL || ControlCode == D_1_3_MASK_CTL){
    				//имя файла строится так <pid><count><EXPANSION>
    				UNICODE_STRING full_fileName, fileName, tmp;
    				WCHAR full_fileName_Buffer[
    					(sizeof(TEMP_PATH) - sizeof(L'\0'))
    					+ 2*sizeof(WCHAR)*NUMBER_OF_SYMBOLS_FOR_32_BIT_NUMBER_IN_16_BASE
    					+ (sizeof(EXPANSION) - sizeof(L'\0'))
    				];
    				HANDLE pid = PsGetCurrentProcessId();
    				HANDLE hFile = NULL;
    				OBJECT_ATTRIBUTES oa;
    				IO_STATUS_BLOCK IoStatusBlock;
    				ULONG count;

    				fileName.Buffer = (WCHAR*)((char*)full_fileName_Buffer + (sizeof(TEMP_PATH) - sizeof(L'\0')));
    				fileName.Length = sizeof(full_fileName_Buffer) - (sizeof(TEMP_PATH) - sizeof(L'\0'));
    				fileName.MaximumLength = fileName.Length;

    				AnswerLength = fileName.Length;
    	    		if (OutputLength < AnswerLength){
	    	    		status = STATUS_BUFFER_TOO_SMALL;
    		    		goto DEVICE_CONTROL_routine_err_2_2;
    		    	}

    				full_fileName.Buffer = full_fileName_Buffer;
    				full_fileName.MaximumLength = sizeof(full_fileName_Buffer);
    				full_fileName.Length = full_fileName.MaximumLength;

    				RtlCopyMemory(full_fileName.Buffer, TEMP_PATH, sizeof(TEMP_PATH) - sizeof(L'\0'));

    				//KdBreakPoint();

    				ACTIONS_PRE_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    				{
						PNODE(PGUI_CONTEXT) pnode = PGUI_CONTEXT_finde((PGUI_CONTEXT)&pid, &GlobalData.CDO_data.GUIs.data);
						RtlCopyMemory(&count, &pnode->key_->count, sizeof(LONG));
						InterlockedIncrement(&pnode->key_->count);
    		    	}
    		    	ACTIONS_POST_READING(&GlobalData.CDO_data.GUIs.var_for_sync);

    		    	tmp.Buffer = fileName.Buffer;
    		    	tmp.MaximumLength = sizeof(WCHAR)*NUMBER_OF_SYMBOLS_FOR_32_BIT_NUMBER_IN_16_BASE;
    		    	tmp.Length = 0;
    		    	status = RtlIntegerToUnicodeString( (ULONG)pid /*в pid 32-битное значение*/, 16, &tmp);
    		    	KdBreakPoint();//посмотреть какой получился tmp
    		    	if (!NT_SUCCESS(status)) goto DEVICE_CONTROL_routine_err_2_2;

    		    	tmp.Buffer = fileName.Buffer + NUMBER_OF_SYMBOLS_FOR_32_BIT_NUMBER_IN_16_BASE;
    		    	tmp.Length = 0;
    		    	status = RtlIntegerToUnicodeString(count, 16, &tmp);
    		    	KdBreakPoint();//посмотреть какой получился tmp
    		    	if (!NT_SUCCESS(status)) goto DEVICE_CONTROL_routine_err_2_2;

    		    	RtlCopyMemory(fileName.Buffer + 2*NUMBER_OF_SYMBOLS_FOR_32_BIT_NUMBER_IN_16_BASE, EXPANSION, sizeof(EXPANSION) - sizeof(L'\0'));

    		    	InitializeObjectAttributes(&oa, &full_fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    				status = ZwCreateFile(&hFile, GENERIC_WRITE, &oa, &IoStatusBlock, NULL, 0, FILE_ATTRIBUTE_NORMAL, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT |  0, NULL, 0);
    				if (!NT_SUCCESS(status)) goto DEVICE_CONTROL_routine_err_2_2;

    		    	{
    		    		INFO_FOR_write_to_file info = {hFile, STATUS_SUCCESS, NULL, 0};
    		    		SIZE_T size = 0;

    		    		if (ControlCode == D_1_3_FOLDER_CTL) PFOLDER_get_pnode_where_condition_is_true(&size, &pfolder->subfolders, max_size_FOLDER_names_in_tree);
    		    		else if (ControlCode == D_1_3_NAME_CTL) UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->hiding_names, max_size_UNIC_STR_records_in_tree);
    		    		else UNIC_STR_get_pnode_where_condition_is_true(&size, &pfolder->masks, max_size_UNIC_STR_records_in_tree);

    		    		if (size){
    		    			info.Buffer = GetMemory(size + 2*sizeof(SEPARATOR));
    		    			if (info.Buffer){
    		    				info.Buffer[0] = SEPARATOR;
	    		    			if (ControlCode == D_1_3_FOLDER_CTL) PFOLDER_get_pnode_where_condition_is_true(&info, &pfolder->subfolders, write_FOLDER_names_to_file);
    			    			else if (ControlCode == D_1_3_NAME_CTL) UNIC_STR_get_pnode_where_condition_is_true(&info, &pfolder->hiding_names, write_UNIC_STR_names_to_file);
    			    			else UNIC_STR_get_pnode_where_condition_is_true(&info, &pfolder->masks, write_UNIC_STR_names_to_file);
    		    			} else info.status = STATUS_MEMORY_NOT_ALLOCATED;
    		    		}

    		    		status = info.status;
    		    		FreeMemory(info.Buffer);
    		    	}

    		    	ZwClose(hFile);

    		    	if (!NT_SUCCESS(status)){
    		    		ZwDeleteFile(&oa);
    		    		goto DEVICE_CONTROL_routine_err_2_2;
    		    	}

    		    	RtlCopyMemory(pIrp->AssociatedIrp.SystemBuffer, fileName.Buffer, AnswerLength);
    			} else {
    				status = IO_ERR_INVALID_CTL;
	        		goto DEVICE_CONTROL_routine_err_2_2;
    			}
	    	}
	    	ACTIONS_POST_READING(&container->root_folder.var_for_sync);
	    	goto DEVICE_CONTROL_routine_OK_2_1;

	    	DEVICE_CONTROL_routine_err_2_2:
    		ACTIONS_POST_READING(&container->root_folder.var_for_sync);
    		goto DEVICE_CONTROL_routine_err_2_1;
    	}
    	DEVICE_CONTROL_routine_OK_2_1:
    	ACTIONS_POST_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
    	goto DEVICE_CONTROL_routine_OK_1;

    	DEVICE_CONTROL_routine_err_2_1:
    	ACTIONS_POST_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
    	goto DEVICE_CONTROL_routine_err_1;
	}

    DEVICE_CONTROL_routine_OK_1:
    pIrp->IoStatus.Status      = STATUS_SUCCESS;
    pIrp->IoStatus.Information = AnswerLength;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

	DEVICE_CONTROL_routine_err_1:
    pIrp->IoStatus.Status = status;
    if (status == STATUS_BUFFER_TOO_SMALL) pIrp->IoStatus.Information = AnswerLength;//должен быть такой длины
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}


#define __________FOR_WRITE_routine(_param, __ACTION__, WRITE_routine_err, just_for_me)\
				if (anonym) __ACTION__(_param);\
    			else {\
    				UNICODE_STRING message;\
    				message.MaximumLength = (USHORT)message_size;\
    				message.Length = message.MaximumLength;\
    				message.Buffer = ExAllocatePool(PagedPool, message.MaximumLength);\
	    			if (!message.Buffer){\
    					status = STATUS_MEMORY_NOT_ALLOCATED;\
    					goto WRITE_routine_err;\
    				}\
    				RtlCopyMemory(message.Buffer, Buffer, message.Length);\
    				status = make_action_and_sand_messages(&message, _param, __ACTION__, just_for_me);\
    				ClearUNIC_STR(&message);\
    			}

NTSTATUS
WRITE_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    ){
    PWCHAR Buffer = NULL;
    ULONG ulSize = 0;
    ULONG message_size = 0;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS status = STATUS_SUCCESS;
    int cmd;
    BOOLEAN anonym;
    
    UNREFERENCED_PARAMETER( pDeviceObject );
    KdBreakPoint();
    DbgPrint("hide_file_filter!%s: Entered!\n", __FUNCTION__);

   	ulSize  = IrpStack->Parameters.Write.Length;
    Buffer = pIrp->AssociatedIrp.SystemBuffer;//pIrp->AssociatedIrp.SystemBuffer;
    message_size = ulSize;
	#if DBG
       	DbgPrint("Run TestWrite");
       	DbgPrint("ulSize:  %d", ulSize);
       	DbgPrint("pBuffer: %S", Buffer);
	#endif
    
    if (ulSize < sizeof(int)){
    	status = STATUS_BUFFER_TOO_SMALL;
    	goto WRITE_routine_err_1;
    }

    cmd = *(int*)Buffer;
    if (anonym = cmd & ANONYMOUSLY) cmd ^= ANONYMOUSLY;

    if (cmd == ADD_RULE || cmd == REMOVE_RULE){
    	UNICODE_STRING path;
  		UNICODE_STRING volume_name;
   		USHORT i;

   		volume_name.Buffer = (WCHAR*)((char*)Buffer + sizeof(cmd));
		message_size = ulSize;
   		for (i = 0; i < ulSize / sizeof(WCHAR); i++){
    		if (volume_name.Buffer[i] == SEPARATOR) break;
   		}

   		if (i == ulSize / sizeof(WCHAR)){
    		status = IO_ERR_NO_SEPARATOR;
   			goto WRITE_routine_err_1;
   		}

    	volume_name.MaximumLength = (i + 1) * sizeof(WCHAR);
   		volume_name.Length = volume_name.MaximumLength;

   		path.Buffer = (WCHAR*)((char*)volume_name.Buffer + volume_name.MaximumLength);
    	path.MaximumLength = (USHORT)(ulSize - volume_name.MaximumLength - sizeof(cmd));
   		path.Length = path.MaximumLength;

   		ACTIONS_PRE_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
   		{
   			PRULE_CONTAINER container = NULL;
   			status = get_container(&volume_name, &container);
   			if (!NT_SUCCESS(status)) goto WRITE_routine_err_2_1;
    		
    		ACTIONS_PRE_WRITING(&container->root_folder.var_for_sync);
    		{
    			//KdBreakPoint();//нужно узнать, можно ли менять входной буффер
    			if (container->path_is_correct(&path, _FOLDER_) || container->path_is_correct(&path, _RULE_)){
    				PVOID pnode = finde_rule(&path, &container->root_folder.data, NULL);
    				ABOUT_RULE param;
    				param.add = (cmd == ADD_RULE);
    				RtlCopyMemory(&param.path, &path, sizeof(UNICODE_STRING));
    				param.folder = &container->root_folder.data;
    				if (pnode && !param.add || !pnode && param.add){
    					if (param.add) TO_UPPERCASE(&path);
    					__________FOR_WRITE_routine(&param, ACTION_about_rule, WRITE_routine_err_2_2, FALSE);
    				} else {status = THERE_WAS_THIS_RULE;}
    			} else {
    				status = IO_ERR_INVALID_PATH;
    				goto WRITE_routine_err_2_2;
    			}
    		}
	    	ACTIONS_POST_WRITING(&container->root_folder.var_for_sync);
	    	goto WRITE_routine_OK_2_1;

	    	WRITE_routine_err_2_2:
    		ACTIONS_POST_WRITING(&container->root_folder.var_for_sync);
    		goto WRITE_routine_err_2_1;
    	}
    	WRITE_routine_OK_2_1:
    	ACTIONS_POST_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
    	goto WRITE_routine_OK_1;

    	WRITE_routine_err_2_1:
    	ACTIONS_POST_READING(&GlobalData.CDO_data.local_rules.var_for_sync);
    	goto WRITE_routine_err_1;

    } else if (cmd == ADD_PRIVILEGE_PID || cmd == REMOVE_PRIVILEGE_PID) {
    	if(KeGetCurrentIrql() > APC_LEVEL){
    		status = IO_ERR_IRQL_TOO_BIG;
    		goto WRITE_routine_err_1;
		}
    	{
    		PEPROCESS ptr;
    		ULONG pid;
    		message_size = sizeof(cmd) + sizeof(pid);
    		if (ulSize < message_size){
    			status = STATUS_BUFFER_TOO_SMALL;
    			goto WRITE_routine_err_1;
    		}
    		RtlCopyMemory(&pid, (char*)Buffer + sizeof(cmd), sizeof(pid));
    		status = PsLookupProcessByProcessId((HANDLE)pid, &ptr);
    		if (NT_SUCCESS(status)){
    			ABOUT_PID param = {cmd == ADD_PRIVILEGE_PID, (HANDLE)pid};
    			PNODE(PID) pnode;
    			ObDereferenceObject(ptr);

    			ACTIONS_PRE_WRITING(&GlobalData.CDO_data.preferred.var_for_sync);
    			pnode = PID_finde(param.pid, &GlobalData.CDO_data.preferred.data);
    			if (pnode && !param.add || !pnode && param.add){
    				__________FOR_WRITE_routine(&param, ACTION_about_pid, WRITE_routine_err_3_1, FALSE);
    			} else {status = THERE_WAS_THIS_RULE;}
    			ACTIONS_POST_WRITING(&GlobalData.CDO_data.preferred.var_for_sync);
    			goto WRITE_routine_OK_1;

    			WRITE_routine_err_3_1:
    			ACTIONS_POST_WRITING(&GlobalData.CDO_data.preferred.var_for_sync);
    			goto WRITE_routine_err_1;
	    	} else goto WRITE_routine_err_1;
    	}
    } else if (cmd == MAKE_FILTERING || cmd == DO_NOT_MAKE_FILTERING) {
    	if (GlobalData.make_filtering != (cmd == MAKE_FILTERING)){
    		message_size = sizeof(cmd);
    		__________FOR_WRITE_routine(NULL, ACTION_change_make_filtering, WRITE_routine_err_1, FALSE);
    	}
    } else if (cmd == SEND_MESSAGE_TO_ME || cmd == SEND_MESSAGE_TO_ALL) {
    	if (anonym){
    		status = IO_ERR_INVALID_CMD;
    		goto WRITE_routine_err_1;
    	}if (cmd == SEND_MESSAGE_TO_ALL){
    		__________FOR_WRITE_routine(NULL, ACTION_return_success, WRITE_routine_err_1, FALSE);
    	} else {//cmd == SEND_MESSAGE_TO_ME
    		__________FOR_WRITE_routine(NULL, ACTION_return_success, WRITE_routine_err_1, TRUE);
    	}

    } else {
    	status = IO_ERR_INVALID_CMD;
    	goto WRITE_routine_err_1;
    }

    WRITE_routine_OK_1:
    if (!NT_SUCCESS(status)) goto WRITE_routine_err_1;
    pIrp->IoStatus.Status      = STATUS_SUCCESS;
    pIrp->IoStatus.Information = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

	WRITE_routine_err_1:
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
READ_routine (
    __in PDEVICE_OBJECT pDeviceObject,
    __in PIRP pIrp
    ){

	PWCHAR Buffer = NULL;
    ULONG OutputLength = 0, AnswerLength = 0;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(pIrp);
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( pDeviceObject );
    KdBreakPoint();
    DbgPrint("hide_file_filter!%s: Entered!\n", __FUNCTION__);

    OutputLength = IrpStack->Parameters.Read.Length;
    Buffer = pIrp->AssociatedIrp.SystemBuffer;
  
	#if DBG
       	DbgPrint("Run TestRead");
       	DbgPrint("OutputLength:  %d", OutputLength);
       	DbgPrint("pBuffer: %S", Buffer);
	#endif
	
	ACTIONS_PRE_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    {
    	HANDLE pid = PsGetCurrentProcessId();
        PNODE(PGUI_CONTEXT) pnode_GUI = PGUI_CONTEXT_finde((PGUI_CONTEXT)&pid, &GlobalData.CDO_data.GUIs.data);
        PGUI_CONTEXT GUI;
        ASSERT(pnode_GUI);
        GUI = pnode_GUI->key_;
        KeWaitForSingleObject(&GUI->new_message, UserRequest /*Executive*/, KernelMode, FALSE, NULL);
        KeWaitForSingleObject(&GUI->mutex_for_messages, UserRequest /*Executive*/, KernelMode, FALSE, NULL);

        ASSERT(GUI->messages.first);
        AnswerLength = GUI->messages.first->data->data.Length;
        if (OutputLength < AnswerLength){
        	status = STATUS_BUFFER_TOO_SMALL;
        	goto READ_routine_err_3;
        }

        RtlCopyMemory(Buffer, GUI->messages.first->data->data.Buffer, AnswerLength);
        Free_PREF_COUNT(GUI->messages.first->data, ClearUNIC_STR);
        if (GUI->messages.first->next){
        	GUI->messages.first = (PVOID)GUI->messages.first->next;
        	KeSetEvent(&GUI->new_message, IO_NO_INCREMENT, FALSE);
        }
        else GUI->messages.first = (PVOID)(GUI->messages.last = NULL);

        KeReleaseMutex(&GUI->mutex_for_messages, FALSE); 
        goto READ_routine_OK_2;
        
        READ_routine_err_3:
        KeSetEvent(&GUI->new_message, IO_NO_INCREMENT, FALSE);
        KeReleaseMutex(&GUI->mutex_for_messages, FALSE); 
        goto READ_routine_err_2;
    }
    READ_routine_OK_2:
    ACTIONS_POST_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    goto READ_routine_OK_1;
    
    READ_routine_err_2:
    ACTIONS_POST_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    goto READ_routine_err_1;

    READ_routine_OK_1:
    pIrp->IoStatus.Status      = STATUS_SUCCESS;
    pIrp->IoStatus.Information = AnswerLength;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

    READ_routine_err_1:
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
}