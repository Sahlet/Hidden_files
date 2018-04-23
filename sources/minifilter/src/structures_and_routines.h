/*
	structures_and_routines.h
		для каждой папки есть дерево, в котором хранятся подпапки,
		дерево, в котором храняться правила скрытия для текущей папки без звездочек и вопросов,
		дерево, в котором храняться правила скрытия для текущей папки со звездочками и вопросами.
*/
#ifndef STRUCTURES_AND_ROUTINES

#define STRUCTURES_AND_ROUTINES

#include <fltKernel.h>
//#include <Wdm.h>
//#include <ntdef.h>
//#include <Ntifs.h>
//#include <dontuse.h>
//#include <suppress.h>
#include "shared_defs.h"
#include "tree.h"

//папка
//здесь папкой обзывается структура с именем папки, что выступает в роли ключа,
// деревм с подпапками,
// деревом с именями, что нужно скрыть
// и деревом с масками, по которым нужно скрывать
typedef struct _FOLDER{
	UNICODE_STRING key_;//имя
	PNODE(PVOID) subfolders;	//PNODE(PFOLDER) subfolders_tree;
	PNODE(UNICODE_STRING) hiding_names;
	PNODE(UNICODE_STRING) masks;
} FOLDER, *PFOLDER;

//REVIEW_REC: RtlZeroMemory не проще ли? 
#define INIT_FOLDER_BY_EMPTY_VALE(folder) RtlZeroMemory(&folder, sizeof(FOLDER));


//меняет все символы в нижнем регистре на символы в верхнем регистре
#define TO_UPPERCASE(PUNICODE_STRING_str) RtlUpcaseUnicodeString(PUNICODE_STRING_str, PUNICODE_STRING_str, FALSE);

#define REMOVE_SLASHES_AND_QUESTION_MARKS(punic_str)\
    {\
    	USHORT i;\
    	for(i = 0; i < (punic_str)->Length / sizeof(WCHAR); i++){\
    		if (\
    			(punic_str)->Buffer[i] == L'\\' 	||\
    			(punic_str)->Buffer[i] == L'?'\
    		   ){\
    			(punic_str)->Buffer[i] = L'_';\
    		}\
    	}\
    }


extern PVOID GetMemory(SIZE_T size);
extern void FreeMemory(PVOID ptr);
extern void ClearUNIC_STR(PUNICODE_STRING str);
extern LONG CompareUNIC_STR(PUNICODE_STRING left, PUNICODE_STRING right);


//не проверяет на верхний регистр
//чтоб вернуло тру, надо чтоб все имена в пути начинались не с пробела и заканчивались не пробелом, и чтоб в rule не было недопустичых символов, между бекслешами есть хоть один не пустой символ
//если folder == true, вернет true, если rule будет оканчиваться на \ и в rule не будет * и ?
//если folder == false, вернет true, если rule не будет оканчиваться на \ и в rule могут присутствовать * и ?
enum type_of_path {_FOLDER_, _NAME_, _RULE_};
extern BOOLEAN path_is_correct_NTFS(__in PUNICODE_STRING rule, __in int type);
extern BOOLEAN path_is_correct_VFAT(__in PUNICODE_STRING rule, __in int type);
extern BOOLEAN path_is_correct_ALL(__in PUNICODE_STRING rule, __in int type);

//________________________________________________________________________________________________________________
//________________________________________________________________________________________________________________
//структыры и функции для обеспечения синхронного доступа
	#define REF_COUNT(TYPE)																															\
	struct {																																		\
		TYPE data;																																	\
		LONG refs;	/*InterlockedDecrement; InterlockedIncrement;*/																					\
	}
	#define PREF_COUNT(TYPE) REF_COUNT(TYPE)*

	//not function
	#define Free_PREF_COUNT(ptr, Clear_for_TYPE)																									\
		if (!InterlockedDecrement(&(ptr)->refs)){																									\
        	Clear_for_TYPE(&(ptr)->data);																											\
        	FreeMemory(ptr);																														\
    	}

	#define RW_SYNC(TYPE)																															\
	struct {																																		\
		TYPE data;																																	\
		ERESOURCE var_for_sync;																														\
	}

	#define W_SYNC(TYPE)																															\
	struct {																																		\
		TYPE data;																																	\
		KMUTEX mut;																																	\
	}

typedef struct _RULE_CONTAINER{
	RW_SYNC(FOLDER) root_folder;
	UNICODE_STRING GUID;
	BOOLEAN (*path_is_correct)(__in PUNICODE_STRING rule, __in int type);
} RULE_CONTAINER, *PRULE_CONTAINER;


//действия перед записыванием в контейнер
#define ACTIONS_PRE_WRITING(ptr_eresource)																			\
{																													\
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);                                                                        \
	KeEnterCriticalRegion();                                                                                        \
    ExAcquireResourceExclusiveLite(ptr_eresource, TRUE);                                                            \
}
//действия после записывания в контейнер
#define ACTIONS_POST_WRITING(ptr_eresource)                                                                         \
{																													\
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);                                                                        \
    ExReleaseResourceLite(ptr_eresource);																			\
	KeLeaveCriticalRegion();                                                                                        \
}


//действия перед чтением в контейнер
#define ACTIONS_PRE_READING(ptr_eresource)                                                                          \
{                                                                                                                   \
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);                                                                        \
    KeEnterCriticalRegion();                                                                                        \
    ExAcquireResourceSharedLite(ptr_eresource, TRUE);                                                               \
}
//действия после чтения в контейнер
#define ACTIONS_POST_READING(ptr_eresource)                                                                         \
{                                                                                                                   \
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);                                                                        \
    ExReleaseResourceLite(ptr_eresource);																			\
    KeLeaveCriticalRegion();                                                                                        \
}

extern NTSTATUS init_RULE_CONTAINER(__out PRULE_CONTAINER container);
extern void clear_RULE_CONTAINER(__out PRULE_CONTAINER container);

extern NTSTATUS fill_RULE_CONTAINER(__inout PRULE_CONTAINER container);
extern NTSTATUS save_rules(__in PRULE_CONTAINER container);


extern NTSTATUS add_rule_sync(__in PUNICODE_STRING rule, __inout PRULE_CONTAINER container);

#define YOU_CAN_NOT_DELETE_THE_RULE 			0xFFFFFF05
extern NTSTATUS delete_rule_sync(__in PUNICODE_STRING rule, __inout PRULE_CONTAINER container);

//отвечает скрывать файл или нет
extern BOOLEAN Must_be_hidden(__in PUNICODE_STRING path, __in PUNICODE_STRING fileName, __in PUNICODE_STRING full_fileName, __in PFOLDER folder);
extern BOOLEAN Must_be_hidden_sync(__in PUNICODE_STRING path, __in PUNICODE_STRING fileName, __in PUNICODE_STRING full_fileName, __in PRULE_CONTAINER container);

enum type_of_res {NOT_EXISTING_PATH, PNODE_UNICODE_STRING_MASK, PNODE_UNICODE_STRING_NAME, __PFOLDER__};
extern PVOID finde_rule(__in PUNICODE_STRING rule, __in PFOLDER folder, __out_opt int* type);

//________________________________________________________________________________________________________________
//________________________________________________________________________________________________________________
//структыры и функции для CDO

	#define QUEUE_NODE(TYPE)																														\
	struct {																																		\
		TYPE data;																																	\
		PVOID next;																																	\
	}
	#define PQUEUE_NODE(TYPE) QUEUE_NODE(TYPE)*
	#define QUEUE(TYPE)																																\
	struct {																																		\
		PQUEUE_NODE(TYPE) first;																													\
		PQUEUE_NODE(TYPE) last;																														\
	}
	#define PQUEUE(TYPE) QUEUE(TYPE)*


	typedef struct _GUI_CONTEXT{
		HANDLE pid;
		LONG count;//для создания имен файлов
		QUEUE(PREF_COUNT(UNICODE_STRING)) messages;
		KMUTEX mutex_for_messages;
		KEVENT new_message;
	} GUI_CONTEXT, *PGUI_CONTEXT;

	extern VOID Clear_QUEUE_of_messages(PVOID /*PQUEUE(UNICODE_STRING)*/ messages);

	#define MY_FLT_INSTANCE_CONTEXT PRULE_CONTAINER

	typedef HANDLE PID;
	typedef struct _CDO_DATA{
		RW_SYNC(PNODE(MY_FLT_INSTANCE_CONTEXT)) local_rules;
		RW_SYNC(PNODE(PGUI_CONTEXT)) GUIs;
		RW_SYNC(PNODE(PID)) preferred;
	} CDO_DATA, *PCDO_DATA;

	typedef struct _HIDE_FILES_FILTER_DATA{

		PFLT_FILTER gFilterHandle;
    	PDEVICE_OBJECT CDO;

    	RULE_CONTAINER container;
    	CDO_DATA CDO_data;
    	BOOLEAN make_filtering;

    	HANDLE driver_directory;//ZwCreateDirectoryObject
	} HIDE_FILES_FILTER_DATA, *P_HIDE_FILES_FILTER_DATA;

	extern HIDE_FILES_FILTER_DATA GlobalData;

	extern VOID Free_Globa_CDO_DATA();
	extern NTSTATUS init_Globa_CDO_DATA();
	extern NTSTATUS init_Global_container();
	extern NTSTATUS init_Global_container ();
	extern BOOLEAN is_preferred_sync(PID pid);
	extern VOID PROCESS_NOTIFY_ROUTINE ( IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create );

	BALANCED_TREE_ROUTINES_DECLARATION(UNIC_STR_, UNICODE_STRING);
	BALANCED_TREE_ROUTINES_DECLARATION(PFOLDER_, PFOLDER);
	BALANCED_TREE_ROUTINES_DECLARATION(MY_FLT_INSTANCE_CONTEXT_, MY_FLT_INSTANCE_CONTEXT);
	BALANCED_TREE_ROUTINES_DECLARATION(PGUI_CONTEXT_, PGUI_CONTEXT);
	BALANCED_TREE_ROUTINES_DECLARATION(PID_, PID);

	extern BOOLEAN sum_size_FOLDER_names_in_tree(PSIZE_T size, PFOLDER* ppfolder);
	extern BOOLEAN sum_size_UNIC_STR_records_in_tree(PSIZE_T size, PUNICODE_STRING str);
	
	extern BOOLEAN write_FOLDER_names_to_buffer(WCHAR** pBuffer, PFOLDER* ppfolder);
	extern BOOLEAN write_UNIC_STR_names_to_buffer(WCHAR** pBuffer, PUNICODE_STRING str);

	extern BOOLEAN max_size_FOLDER_names_in_tree(PSIZE_T size, PFOLDER* ppfolder);
	extern BOOLEAN max_size_UNIC_STR_records_in_tree(PSIZE_T size, PUNICODE_STRING str);

	typedef struct _INFO_FOR_write_to_file{
		HANDLE hFile;
		NTSTATUS status;
		PWCHAR Buffer;
		IO_STATUS_BLOCK IoStatusBlock;
	} INFO_FOR_write_to_file, *PINFO_FOR_write_to_file;
	extern BOOLEAN write_FOLDER_names_to_file(PINFO_FOR_write_to_file info, PFOLDER* ppfolder);
	extern BOOLEAN write_UNIC_STR_names_to_file(PINFO_FOR_write_to_file info, PUNICODE_STRING str);

	extern BOOLEAN how_many_nodes(PULONG n, PVOID);
	extern NTSTATUS make_action_and_sand_messages(PUNICODE_STRING _message, PVOID param, NTSTATUS (*ACTION)(PVOID param), BOOLEAN just_for_me);

	extern NTSTATUS ACTION_change_make_filtering(PVOID);

	typedef struct _ABOUT_PID{
		BOOLEAN add;
		HANDLE pid;
	}ABOUT_PID, *PABOUT_PID;
	extern NTSTATUS ACTION_about_pid(PABOUT_PID param);

	typedef struct _ABOUT_RULE{
		BOOLEAN add;
		UNICODE_STRING path;
		PFOLDER folder;
	}ABOUT_RULE, *PABOUT_RULE;
	extern NTSTATUS ACTION_about_rule(PABOUT_RULE param);	
	extern NTSTATUS ACTION_return_success(PVOID param);

#endif