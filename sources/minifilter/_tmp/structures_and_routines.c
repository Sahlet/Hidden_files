/*
	structures_and_routines.c
*/
#include "structures_and_routines.h"

PVOID GetMemory(SIZE_T size){
    __try{
        return ExAllocatePool(NonPagedPool, size);
    }
    __except(EXCEPTION_EXECUTE_HANDLER){
        __try{
            return ExAllocatePool(PagedPool, size);
        }
        __except(EXCEPTION_EXECUTE_HANDLER){
            return NULL;
        }
    }
}
void FreeMemory(PVOID ptr){
    if (ptr) ExFreePool(ptr);
}
void ClearUNIC_STR(PUNICODE_STRING str){
    FreeMemory(str->Buffer);
    RtlZeroMemory(str, sizeof(UNICODE_STRING));
}
LONG CompareUNIC_STR(PUNICODE_STRING left, PUNICODE_STRING right){
    return RtlCompareUnicodeString(left, right, TRUE);
}

BALANCED_TREE_ROUTINES_DEFINITION(UNIC_STR_, UNICODE_STRING, GetMemory, FreeMemory, ClearUNIC_STR, CompareUNIC_STR)


//очищает папку (указатель на папку не освобождает и имя папки не трогает)
void clear_folder(__inout PFOLDER pfolder);

FreePFOLDER(PFOLDER pfolder){
    if (pfolder){
        ClearUNIC_STR(&pfolder->key_);
        clear_folder(pfolder);
        FreeMemory(pfolder);
    }
}

#define ClearPFOLDER(ptr) FreePFOLDER(*ptr)

LONG ComparePFOLDER(PFOLDER* left, PFOLDER* right){
    return RtlCompareUnicodeString(&(*left)->key_, &(*right)->key_, TRUE);
}

BALANCED_TREE_ROUTINES_DEFINITION(PFOLDER_, PFOLDER, GetMemory, FreeMemory, ClearPFOLDER, ComparePFOLDER)

//очищает папку (указатель на папку не освобождает и имя папки не трогает)
void clear_folder(__inout PFOLDER pfolder){
    UNIC_STR_clear_tree((PVOID)&pfolder->masks);
    UNIC_STR_clear_tree((PVOID)&pfolder->hiding_names);
    PFOLDER_clear_tree((PVOID)&pfolder->subfolders);
}

#define _____WRONG_CHAR_FOR_ALL_SYSTEMS(ch) (ch == L'\t' || ch == L'\n' || ch == L'"' || ch == L'*' || ch == L':' || *ptr == L'/' || ch == L'\\' || ch == L'?' || ch == L'|' || ch == L'\0')
#define _____WRONG_CHAR_FOR_NTFS(ch) (_____WRONG_CHAR_FOR_ALL_SYSTEMS(ch) || ch == L'<' || ch == L'>')
#define _____WRONG_CHAR_FOR_VFAT(ch) (_____WRONG_CHAR_FOR_ALL_SYSTEMS(ch) || ch == L'=' || ch == L';' || ch == L'[' || ch == L']' || ch == L',' || ch == L'^')
#define _____WITHOUT_STAR_AND_QUESTION_MARK(ch, _____WRONG_CHAR) (ch != L'*' && ch != L'*' && _____WRONG_CHAR(ch))
#define _____WRONG_CHAR_FOR_NTFS_WITHOUT_STAR_AND_QUESTION_MARK(ch) _____WITHOUT_STAR_AND_QUESTION_MARK(ch, _____WRONG_CHAR_FOR_NTFS)
#define _____WRONG_CHAR_FOR_VFAT_WITHOUT_STAR_AND_QUESTION_MARK(ch) _____WITHOUT_STAR_AND_QUESTION_MARK(ch, _____WRONG_CHAR_FOR_VFAT)

#define _____CHECK(punic_rule, _____WRONG_CHAR)\
            {\
                WCHAR* ptr = *punic_rule->Buffer != L'\\' ? punic_rule->Buffer : punic_rule->Buffer + 1, *end = (WCHAR*)((char*)punic_rule->Buffer + punic_rule->Length);\
                ULONG Length_of_cour_name = 0;\
                while(ptr != end){\
                    if (*ptr == L'\\'){\
                        if (Length_of_cour_name == 0) return FALSE;\
                        if (*(ptr - 1) == L' ' || (ptr + 1 != end && *(ptr + 1) == L' ')) return FALSE;\
                        Length_of_cour_name = 0;\
                    } else {\
                        if (_____WRONG_CHAR(*ptr)) return FALSE;\
                        Length_of_cour_name++;\
                    }\
                    ptr++;\
                }\
            }

//не проверяет на верхний регистр
//чтоб вернуло тру, надо чтоб все имена в пути начинались не с пробела и заканчивались не пробелом,
//и чтоб в rule не было недопустичых символов, между бекслешами есть хоть один не пустой символ и сам путь начинался с бекслеша
//если type == _FOLDER_, вернет true, если rule будет оканчиваться на \ и в rule не будет * и ?
//если type == _RULE_, вернет true, если rule не будет оканчиваться на \ и в rule могут присутствовать * и ?
//если type == _NAME_, вернет true, если rule не будет оканчиваться на \ и в rule не будет * и ?
BOOLEAN path_is_correct_NTFS(__in PUNICODE_STRING rule, __in int type){
    if (rule->Length == 0 || (rule->Buffer[0] != L'\\')) return FALSE;
    if(type == _FOLDER_){
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) != L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_NTFS);
    } else if(type == _RULE_) {
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) == L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_NTFS_WITHOUT_STAR_AND_QUESTION_MARK);
    } else if(type == _NAME_) {
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) == L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_NTFS);
    } else return FALSE;    
    return TRUE;
}
BOOLEAN path_is_correct_VFAT(__in PUNICODE_STRING rule, __in int type){
    if (rule->Length == 0 || (rule->Buffer[0] != L'\\')) return FALSE;
    if(type == _FOLDER_){
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) != L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_VFAT);
    } else if(type == _RULE_) {
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) == L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_VFAT_WITHOUT_STAR_AND_QUESTION_MARK);
    } else if(type == _NAME_) {
        if(*(WCHAR*)((char*)rule->Buffer + rule->Length - sizeof(WCHAR)) == L'\\') return FALSE;
        _____CHECK(rule, _____WRONG_CHAR_FOR_VFAT);
    } else return FALSE;
    return TRUE;
}
BOOLEAN path_is_correct_ALL(__in PUNICODE_STRING rule, __in int type){
    return path_is_correct_NTFS(rule, type) && path_is_correct_VFAT(rule, type);
}

//задает текущее имя
void get_current_name(__in PUNICODE_STRING path, __out PUNICODE_STRING cur_name){
	if (path->Length != 0){
    	cur_name->Buffer = *path->Buffer != L'\\' ? path->Buffer : path->Buffer + 1;
    	{
	        WCHAR* ptr = cur_name->Buffer, *end = (WCHAR*)((char*)path->Buffer + path->Length);
    	    while(ptr != end){
        	    if (*ptr == L'\\') break;
        	    ptr++;
	        }
    	    cur_name->MaximumLength = cur_name->Length = (USHORT)((char*)ptr - (char*)cur_name->Buffer);
    	}
    } else {
        cur_name->MaximumLength = cur_name->Length = 0;
        cur_name->Buffer = NULL;
    }   
}

//задает текущее имя или маску (если маска, то в ней все, что есть в rule без первого бекслеша (если он есть))
void get_current_rule(__in PUNICODE_STRING rule, __out PUNICODE_STRING cur_rule, BOOLEAN* cur_rule_is_mask){
    cur_rule->Buffer = *rule->Buffer != L'\\' ? rule->Buffer : rule->Buffer + 1;
    *cur_rule_is_mask = FALSE;
    {
        WCHAR* ptr = cur_rule->Buffer, *end = (WCHAR*)((char*)rule->Buffer + rule->Length);
        while(ptr != end){
            if (*ptr == L'\\') break;
            else if (*ptr == L'*' || *ptr == L'?'){
                *cur_rule_is_mask = TRUE;
                ptr = end;
                break;
            }
            ptr++;
        }
        cur_rule->MaximumLength = cur_rule->Length = (USHORT)((char*)ptr - (char*)cur_rule->Buffer);
    }
}



NTSTATUS add_rule(__in PUNICODE_STRING rule, __inout PFOLDER folder){
    UNICODE_STRING cur_rule, cur_rule_copy;
    BOOLEAN cur_rule_is_mask;
    NTSTATUS status = STATUS_SUCCESS;
    
    get_current_rule(rule, &cur_rule, &cur_rule_is_mask);
    if (cur_rule.Length == 0) return THERE_WAS_THIS_RULE | IT_WAS_FOLDER;

    cur_rule_copy.MaximumLength = cur_rule.Length;
    cur_rule_copy.Length = cur_rule.Length;
    if(!(cur_rule_copy.Buffer = GetMemory(cur_rule.Length))) return STATUS_MEMORY_NOT_ALLOCATED;
    RtlCopyMemory(cur_rule_copy.Buffer, cur_rule.Buffer, cur_rule.Length);

    if (cur_rule_is_mask) {
        //cur_rule - маска (в ней есть * или ?)
        PNODE(UNICODE_STRING) ptr = (PVOID)UNIC_STR_insert_element(cur_rule_copy, (PVOID)&folder->masks);
        status = IT_WAS_MASK;
        if (!ptr){
            status = STATUS_MEMORY_NOT_ALLOCATED;
            goto add_rule_err_1;
        } else if (ptr->key_.Buffer == cur_rule_copy.Buffer){
            //мы добавили правило, которого не было
            status |= THERE_WAS_NOT_THIS_RULE;
        } else {
            status |= THERE_WAS_THIS_RULE;
            ClearUNIC_STR(&cur_rule_copy);
        }
    } else {
        ULONG shift = (cur_rule.Length + (ULONG)(((char*)cur_rule.Buffer - (char*)rule->Buffer)));
        if (shift == rule->Length) {
            //cur_rule - последнее имя в rule без * и ?
            PNODE(UNICODE_STRING) ptr = (PVOID)UNIC_STR_insert_element(cur_rule_copy, (PVOID)&folder->hiding_names);
            status = IT_WAS_NAME;
            if (!ptr){
                status = STATUS_MEMORY_NOT_ALLOCATED;
                goto add_rule_err_1;
            } else if (ptr->key_.Buffer == cur_rule_copy.Buffer){
                //мы добавили правило, которого не было
                status = THERE_WAS_NOT_THIS_RULE;
            } else {
                status = THERE_WAS_THIS_RULE;
                ClearUNIC_STR(&cur_rule_copy);
            }
        } else {
            //cur_rule - промежуточное имя в rule без * и ?
            PNODE(PFOLDER) ptr;
            PFOLDER subfolder = GetMemory(sizeof(FOLDER));
            if (!subfolder){
                status = STATUS_MEMORY_NOT_ALLOCATED;
                goto add_rule_err_1;
            }

            INIT_FOLDER_BY_EMPTY_VALE((*subfolder));
            RtlCopyMemory(&(subfolder->key_), &cur_rule_copy, sizeof(UNICODE_STRING));
            RtlZeroMemory(&cur_rule_copy, sizeof(UNICODE_STRING));
            
            ptr = (PVOID)PFOLDER_insert_element(subfolder, (PVOID)&folder->subfolders);
            if (!ptr){
                FreePFOLDER(subfolder);
                return STATUS_MEMORY_NOT_ALLOCATED;
            } else {
                UNICODE_STRING rest;//остаток
                if ((PVOID)(ptr->key_) == subfolder){
                    //мы добавили правило, которого не было
                    status = THERE_WAS_NOT_THIS_RULE;
                } else {
                    FreePFOLDER(subfolder);
                    subfolder = NULL;
                    status = THERE_WAS_THIS_RULE;
                }
                rest.Length = (USHORT)(rule->Length - shift);
                rest.MaximumLength = rest.Length;
                rest.Buffer = (WCHAR*)((char*)rule->Buffer + shift);
                if (rest.Length == 2 && rest.Buffer[0] == L'\\'){
                    status |= IT_WAS_FOLDER;
                } else {
                    status = add_rule(&rest, ptr->key_);
                    if (!NT_SUCCESS(status) && subfolder) PFOLDER_delete_element(subfolder, (PVOID)&folder->subfolders);
                }
            }
        }
    }
    
    return status;

    add_rule_err_1:
    ClearUNIC_STR(&cur_rule_copy);
    return status;
}
NTSTATUS delete_rule(__in PUNICODE_STRING rule, __inout PFOLDER folder){
    UNICODE_STRING cur_rule;
    BOOLEAN cur_rule_is_mask;
    NTSTATUS status = STATUS_SUCCESS;
    get_current_rule(rule, &cur_rule, &cur_rule_is_mask);

    if (cur_rule.Length == 0) return YOU_CAN_NOT_DELETE_THE_RULE;

    if (cur_rule_is_mask) {
        //cur_rule - маска (в ней есть * или ?)
        status = IT_WAS_MASK;
        if (UNIC_STR_finde(cur_rule, (PVOID)&folder->masks)){
            UNIC_STR_delete_element(cur_rule, (PVOID)&folder->masks);
            status |= THERE_WAS_THIS_RULE;
        } else status |= THERE_WAS_NOT_THIS_RULE;
    } else {
        ULONG shift = (cur_rule.Length + (ULONG)(((char*)cur_rule.Buffer - (char*)rule->Buffer)));
        if (shift == rule->Length){
            //cur_rule - последнее имя в rule без * и ?
            status = IT_WAS_NAME;
            if (UNIC_STR_finde(cur_rule, (PVOID)&folder->hiding_names)){
                UNIC_STR_delete_element(cur_rule, (PVOID)&folder->hiding_names);
                status |= THERE_WAS_THIS_RULE;
            } else status |= THERE_WAS_NOT_THIS_RULE;
        } else {
            //cur_rule - промежуточное имя в rule без * и ?
            PNODE(PFOLDER) ptr = (PVOID)PFOLDER_finde((PVOID)&cur_rule, (PVOID)&folder->subfolders);
            if (ptr){
                UNICODE_STRING rest;//остаток
                rest.Length = (USHORT)(rule->Length - shift);
                rest.MaximumLength = rest.Length;
                rest.Buffer = (WCHAR*)((char*)rule->Buffer + shift);
                if (rest.Length == 2 && rest.Buffer[0] == L'\\'){
                    PFOLDER_delete_element((PVOID)&cur_rule, (PVOID)&folder->subfolders);
                    status = IT_WAS_FOLDER | THERE_WAS_THIS_RULE;
                } else status = delete_rule(&rest, ptr->key_);
            } else status = IT_WAS_FOLDER | THERE_WAS_NOT_THIS_RULE;
        }
    }
    return status;
}

BOOLEAN condition_for_mascs(PVOID fileName, PUNICODE_STRING mask){
	return FsRtlIsNameInExpression(mask, fileName, TRUE, NULL);
}
BOOLEAN _______Must_be_hidden(__in PUNICODE_STRING path, __in PUNICODE_STRING fileName, __in PUNICODE_STRING full_fileName, __in PFOLDER folder){
	UNICODE_STRING cur_name;
    get_current_name(path, &cur_name);
    if (cur_name.Length) {
    	USHORT shift = (USHORT)((cur_name.Length + ((char*)cur_name.Buffer - (char*)path->Buffer)));
        PNODE(PFOLDER) ptr = (PVOID)PFOLDER_finde((PVOID)&cur_name, (PVOID)&folder->subfolders);
        if (ptr){
            UNICODE_STRING rest_path, rest_full_file_name;
    		rest_path.Length = path->Length - shift;
    		rest_path.MaximumLength = rest_path.Length;
    		rest_path.Buffer = (WCHAR*)((char*)path->Buffer + shift);
    		rest_full_file_name.Length = full_fileName->Length - shift;
    		rest_full_file_name.MaximumLength = rest_full_file_name.Length;
    		rest_full_file_name.Buffer = (WCHAR*)((char*)full_fileName->Buffer + shift);
    		if (_______Must_be_hidden(&rest_path, fileName, &rest_full_file_name, ptr->key_)) return TRUE;
        }
        if(UNIC_STR_finde(cur_name, (PVOID)&folder->hiding_names)) return TRUE;
        if(UNIC_STR_get_pnode_where_condition_is_true(full_fileName, (PVOID)&folder->masks, condition_for_mascs)) return TRUE;
        if(UNIC_STR_get_pnode_where_condition_is_true((PVOID)&cur_name, (PVOID)&folder->masks, condition_for_mascs)) return TRUE;
    } else {
        if(UNIC_STR_finde(*fileName, (PVOID)&folder->hiding_names)) return TRUE;
        if(UNIC_STR_get_pnode_where_condition_is_true(fileName, (PVOID)&folder->masks, condition_for_mascs)) return TRUE;
    }
    return FALSE;
}

//отвечает скрывать файл или нет
BOOLEAN Must_be_hidden(__in PUNICODE_STRING path, __in PUNICODE_STRING fileName, __in PUNICODE_STRING full_fileName, __in PFOLDER folder){
	ASSERT(/*Context_into != NULL &&*/ path != NULL && fileName != NULL);
	if (fileName->Buffer[0] == L'.'){
		if (fileName->Length == 1 || (fileName->Buffer[1] == L'.' && fileName->Length == 2)) return FALSE;
	}
	return _______Must_be_hidden(path, fileName, full_fileName, folder);
}

PVOID finde_rule(__in PUNICODE_STRING rule, __in PFOLDER folder, __out_opt int* type_){
    UNICODE_STRING cur_rule;
    BOOLEAN cur_rule_is_mask;
    PVOID ptr = NULL;
    int type;
    
    get_current_rule(rule, &cur_rule, &cur_rule_is_mask);
    if (cur_rule.Length == 0){
        type = __PFOLDER__;
        ptr = folder;
    } else if (cur_rule_is_mask) {
        //cur_rule - маска (в ней есть * или ?)
        if (ptr = UNIC_STR_finde(cur_rule, (PVOID)&folder->masks)) type = PNODE_UNICODE_STRING_MASK;
        else type = NOT_EXISTING_PATH;
    } else {
        ULONG shift = (cur_rule.Length + (ULONG)(((char*)cur_rule.Buffer - (char*)rule->Buffer)));
        if (shift == rule->Length){
            //cur_rule - последнее имя в rule без * и ?
            if (ptr = UNIC_STR_finde(cur_rule, (PVOID)&folder->hiding_names))  type = PNODE_UNICODE_STRING_NAME;
            else type = NOT_EXISTING_PATH;
        } else {
            //cur_rule - промежуточное имя в rule без * и ?
            if (ptr = PFOLDER_finde((PVOID)&cur_rule, (PVOID)&folder->subfolders)){
                UNICODE_STRING rest;//остаток
                rest.Length = (USHORT)(rule->Length - shift);
                rest.MaximumLength = rest.Length;
                rest.Buffer = (WCHAR*)((char*)rule->Buffer + shift);
                ptr = finde_rule(&rest, ((PNODE(PFOLDER))ptr)->key_, &type);
            } else type = NOT_EXISTING_PATH;
        }
    }
    if (type_) *type_ = type;
    return ptr;
}



//________________________________________________________________________________________________________________
//________________________________________________________________________________________________________________


NTSTATUS init_RULE_CONTAINER(__out PRULE_CONTAINER container){
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    INIT_FOLDER_BY_EMPTY_VALE(container->root_folder.data);
    container->GUID.Buffer = NULL;
    container->GUID.Length = 0;
    container->GUID.MaximumLength = 0;

    container->path_is_correct = NULL;

    return ExInitializeResourceLite(&container->root_folder.var_for_sync);
}

void clear_RULE_CONTAINER(__out PRULE_CONTAINER container){
    clear_folder(&container->root_folder.data);
    ClearUNIC_STR(&container->root_folder.data.key_);
    ClearUNIC_STR(&container->GUID);

    ExDeleteResourceLite(&container->root_folder.var_for_sync);
}

//заполняет дерево правил правилами из файла
NTSTATUS fill_RULE_CONTAINER(__inout PRULE_CONTAINER container){
    //файл заполнен правилами так     SEPARATOR<правило1>SEPARATOR SEPARATOR<правило2>SEPARATOR ... SEPARATOR<правило_последнее>SEPARATOR
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE hFile;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR *Buffer, *ptr, *end;
    UNICODE_STRING rule;
    const SIZE_T buf_size = MAX_RULE_LENGTH + 2*sizeof(SEPARATOR);
    BOOLEAN deleted = FALSE;
    LARGE_INTEGER shift;

    //KdBreakPoint();
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    RtlZeroMemory(&shift, sizeof(shift));

    //https://msdn.microsoft.com/en-us/library/windows/hardware/ff547804(v=vs.85).aspx
    //https://msdn.microsoft.com/ru-ru/library/windows/hardware/ff566424(v=vs.85).aspx
    InitializeObjectAttributes(&oa, &container->GUID, OBJ_CASE_INSENSITIVE /*| OBJ_KERNEL_HANDLE*/, GlobalData.driver_directory, NULL);
    status = ZwCreateFile(&hFile, GENERIC_READ /*| GENERIC_WRITE */ /* | SYNCHRONIZE*/, &oa, &IoStatusBlock,
        NULL, 0, FILE_ATTRIBUTE_NORMAL, FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT |  0, NULL, 0);

    if (!NT_SUCCESS(status)) return status;

    Buffer = ExAllocatePool(PagedPool, buf_size);
    if (!Buffer){
        status = STATUS_MEMORY_NOT_ALLOCATED;
        goto fill_RULE_CONTAINER_err_1;
    }

    ACTIONS_PRE_WRITING(&container->root_folder.var_for_sync);
    clear_folder(&container->root_folder.data);

    do{
        status = ZwReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock, Buffer, buf_size, &shift, NULL);
        if (status == STATUS_END_OF_FILE){
            status = STATUS_SUCCESS;
            break;
        }
        if(!NT_SUCCESS(status)) goto fill_RULE_CONTAINER_err_2;
        rule.Buffer = ptr = Buffer; end = (WCHAR*)((char*)Buffer + IoStatusBlock.Information);
        while(ptr < end){
            while(ptr < end && *ptr != SEPARATOR){ptr++;}
            if (ptr < end){
                rule.Buffer = ++ptr;
                while(ptr < end && *ptr != SEPARATOR){ptr++;}
                if (ptr < end){//добавляем правило
                    rule.MaximumLength = (USHORT)((char*)ptr - (char*)rule.Buffer);
                    rule.Length = rule.MaximumLength;
                    if (rule.MaximumLength == 0) goto fill_RULE_CONTAINER_err_3; //файл испорчен (правило без символов)
                    status = add_rule(&rule, &container->root_folder.data);
                    if(!NT_SUCCESS(status)) goto fill_RULE_CONTAINER_err_2;
                    ptr++;
                    rule.Buffer = ptr;
                } else --rule.Buffer;
            }
        }
        if (rule.Buffer == Buffer && IoStatusBlock.Information == buf_size) goto fill_RULE_CONTAINER_err_3; //файл испорчен (длина некоторого правила в нем превышает допустимую)
        //KdBreakPoint();
        shift.LowPart = (ULONG)((char*)rule.Buffer - (char*)Buffer);
    } while (IoStatusBlock.Information == buf_size);

    goto fill_RULE_CONTAINER_OK_1;

    fill_RULE_CONTAINER_err_3: //файл был кем-то испорчен (что-то в нем неправильно).
    status = STATUS_SUCCESS;
    ZwClose(hFile);
    ZwDeleteFile(&oa);
    deleted = TRUE;

    fill_RULE_CONTAINER_err_2:
    clear_folder(&container->root_folder.data);

    fill_RULE_CONTAINER_OK_1:
    ACTIONS_POST_WRITING(&container->root_folder.var_for_sync);
    FreeMemory(Buffer);

    fill_RULE_CONTAINER_err_1:
    if (!deleted) ZwClose(hFile);

    return status;
}

typedef struct _INFO_FOR_SAVING_RULES{
    NTSTATUS status;
    HANDLE hFile;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING rule;
}INFO_FOR_SAVING_RULES, *PINFO_FOR_SAVING_RULES;

BOOLEAN ____write_rule_to_file_func(__inout PINFO_FOR_SAVING_RULES info, __in PUNICODE_STRING str){
    //KdBreakPoint();
    RtlCopyMemory((char*)info->rule.Buffer + info->rule.Length, str->Buffer, str->Length);
    info->rule.Length += str->Length;
    info->rule.Buffer[info->rule.Length / sizeof(WCHAR) /*>> 1*/] = SEPARATOR;

    info->status = ZwWriteFile(info->hFile, NULL, NULL, NULL, &info->IoStatusBlock, info->rule.Buffer, info->rule.Length + sizeof(SEPARATOR), NULL, NULL);
    if (!NT_SUCCESS(info->status)) return TRUE;

    info->rule.Length -= str->Length;
    return FALSE;
}
BOOLEAN recursion_for_write_rule_to_file_func(__inout PINFO_FOR_SAVING_RULES info, __in PFOLDER* ppfolder){
    RtlCopyMemory((char*)info->rule.Buffer + info->rule.Length, (*ppfolder)->key_.Buffer, (*ppfolder)->key_.Length);
    info->rule.Length += (*ppfolder)->key_.Length;
    info->rule.Buffer[info->rule.Length / sizeof(WCHAR)] = L'\\';
    info->rule.Length += sizeof(L'\\');

    UNIC_STR_get_pnode_where_condition_is_true(info, &(*ppfolder)->hiding_names, ____write_rule_to_file_func);
    if (!NT_SUCCESS(info->status)) return TRUE;
    UNIC_STR_get_pnode_where_condition_is_true(info, &(*ppfolder)->masks, ____write_rule_to_file_func);
    if (!NT_SUCCESS(info->status)) return TRUE;
    PFOLDER_get_pnode_where_condition_is_true(info, &(*ppfolder)->subfolders, recursion_for_write_rule_to_file_func);
    if (!NT_SUCCESS(info->status)) return TRUE;

    info->rule.Length -= ((*ppfolder)->key_.Length + sizeof(L'\\'));
    return FALSE;
}
NTSTATUS write_rule_to_file_func(__inout PINFO_FOR_SAVING_RULES info, __in PFOLDER root_folder){
    UNIC_STR_get_pnode_where_condition_is_true(info, &root_folder->hiding_names, ____write_rule_to_file_func);
    if (!NT_SUCCESS(info->status)) return info->status;
    UNIC_STR_get_pnode_where_condition_is_true(info, &root_folder->masks, ____write_rule_to_file_func);
    if (!NT_SUCCESS(info->status)) return info->status;
    PFOLDER_get_pnode_where_condition_is_true(info, &root_folder->subfolders, recursion_for_write_rule_to_file_func);

    return info->status;
}
NTSTATUS save_rules(__in PRULE_CONTAINER container){
    INFO_FOR_SAVING_RULES info;
    OBJECT_ATTRIBUTES oa;

    info.status = STATUS_SUCCESS;

    //KdBreakPoint();
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    //https://msdn.microsoft.com/en-us/library/windows/hardware/ff547804(v=vs.85).aspx
    //https://msdn.microsoft.com/ru-ru/library/windows/hardware/ff566424(v=vs.85).aspx
    InitializeObjectAttributes(&oa, &container->GUID, OBJ_CASE_INSENSITIVE /*| OBJ_KERNEL_HANDLE*/, GlobalData.driver_directory, NULL);
    info.status = ZwCreateFile(&info.hFile, /*GENERIC_READ |*/ GENERIC_WRITE  /* | SYNCHRONIZE */, &oa, &info.IoStatusBlock,
        NULL, 0, FILE_ATTRIBUTE_NORMAL, FILE_OVERWRITE_IF,
        FILE_SYNCHRONOUS_IO_NONALERT | 0, NULL, 0);

    if (!NT_SUCCESS(info.status)) return info.status;

    info.rule.MaximumLength = MAX_RULE_LENGTH + 2*sizeof(WCHAR) + 2*sizeof(SEPARATOR);
    info.rule.Length = 0;
    info.rule.Buffer = ExAllocatePool(PagedPool, info.rule.MaximumLength);
    if (!info.rule.Buffer){
        info.status = STATUS_MEMORY_NOT_ALLOCATED;
        goto save_rules_err_1;
    }
    info.rule.Length = 2*sizeof(WCHAR) + sizeof(SEPARATOR);
    info.rule.Buffer[0] = L'\r';
    info.rule.Buffer[1] = L'\n';
    info.rule.Buffer[2] = SEPARATOR;

    ACTIONS_PRE_READING(&container->root_folder.var_for_sync);
        info.status = write_rule_to_file_func(&info, &container->root_folder.data);
    ACTIONS_POST_READING(&container->root_folder.var_for_sync);
    

    FreeMemory(info.rule.Buffer);

    save_rules_err_1:
    ZwClose(info.hFile);

    return info.status;
}

NTSTATUS add_rule_sync(__in PUNICODE_STRING rule, __inout PRULE_CONTAINER container){
	NTSTATUS status;
	ACTIONS_PRE_WRITING(&container->root_folder.var_for_sync);
	status = add_rule(rule, &container->root_folder.data);
	ACTIONS_POST_WRITING(&container->root_folder.var_for_sync);
	return status;
}
NTSTATUS delete_rule_sync(__in PUNICODE_STRING rule, __inout PRULE_CONTAINER container){
    NTSTATUS status;
	ACTIONS_PRE_WRITING(&container->root_folder.var_for_sync);
	status = delete_rule(rule, &container->root_folder.data);
	ACTIONS_POST_WRITING(&container->root_folder.var_for_sync);
    return status;   
}

BOOLEAN Must_be_hidden_sync(__in PUNICODE_STRING path, __in PUNICODE_STRING fileName, __in PUNICODE_STRING full_fileName, __in PRULE_CONTAINER container){
	BOOLEAN res;
	ACTIONS_PRE_READING(&container->root_folder.var_for_sync);
	res = Must_be_hidden(path, fileName, full_fileName, &container->root_folder.data);
	ACTIONS_POST_READING(&container->root_folder.var_for_sync);
	return res;
}

//________________________________________________________________________________________________________________
//________________________________________________________________________________________________________________
//структыры и функции для CDO

LONG CompareMY_FLT_INSTANCE_CONTEXT(MY_FLT_INSTANCE_CONTEXT* left, MY_FLT_INSTANCE_CONTEXT* right){
    return RtlCompareUnicodeString(&(*left)->root_folder.data.key_, &(*right)->root_folder.data.key_, TRUE);
}

VOID FreeMY_FLT_INSTANCE_CONTEXT(MY_FLT_INSTANCE_CONTEXT container){
    if (container->root_folder.data.key_.Buffer != GlobalData.container.root_folder.data.key_.Buffer){//чтоб не наткнулис на GlobalData.container (так надо)
        FltReleaseContext(container);
    }
}

#define ClearMY_FLT_INSTANCE_CONTEXT(pptr) FreeMY_FLT_INSTANCE_CONTEXT(*pptr)

BALANCED_TREE_ROUTINES_DEFINITION(MY_FLT_INSTANCE_CONTEXT_, MY_FLT_INSTANCE_CONTEXT, GetMemory, FreeMemory, ClearMY_FLT_INSTANCE_CONTEXT, CompareMY_FLT_INSTANCE_CONTEXT)

LONG ComparePGUI_CONTEXT(PGUI_CONTEXT* left, PGUI_CONTEXT* right){
    return ((LONG)((*left)->pid)) - ((LONG)((*right)->pid));
}

VOID Clear_QUEUE_of_messages(PVOID /*PQUEUE(UNICODE_STRING)*/ messages){
    PQUEUE_NODE(PREF_COUNT(UNICODE_STRING)) head = (PVOID)((PQUEUE(UNICODE_STRING))messages)->first;
    PQUEUE_NODE(PREF_COUNT(UNICODE_STRING)) tmp;
    while(head){
        tmp = head->next;
        Free_PREF_COUNT(head->data, ClearUNIC_STR);
        FreeMemory(head);
        head = (PVOID)tmp;
    }
    RtlZeroMemory(messages, sizeof(QUEUE(UNICODE_STRING)));
}

VOID FreePGUI_CONTEXT(PGUI_CONTEXT ptr){
    Clear_QUEUE_of_messages(&ptr->messages);
    FreeMemory(ptr);
}

#define ClearPGUI_CONTEXT(pptr) FreePGUI_CONTEXT(*pptr)

BALANCED_TREE_ROUTINES_DEFINITION(PGUI_CONTEXT_, PGUI_CONTEXT, GetMemory, FreeMemory, ClearPGUI_CONTEXT, ComparePGUI_CONTEXT)

LONG ComparePID(PID* left, PID* right){
    return ((LONG)(*left)) - ((LONG)(*right));
}
#define ClearPID(pptr)
BALANCED_TREE_ROUTINES_DEFINITION(PID_, PID, GetMemory, FreeMemory, ClearPID, ComparePID)


NTSTATUS init_Global_container(){
    NTSTATUS status = init_RULE_CONTAINER(&GlobalData.container);

    if (!NT_SUCCESS(status)) return status;
    //KdBreakPoint();//посмотри на значеия status и GlobalData.container.GUID
    GlobalData.container.GUID.MaximumLength = sizeof(_____GlobalData_container_GUID) - sizeof(L'\0');
    GlobalData.container.GUID.Length = 0;
    if(!(GlobalData.container.GUID.Buffer = GetMemory(GlobalData.container.GUID.MaximumLength))) return STATUS_MEMORY_NOT_ALLOCATED;
    /*status = */RtlAppendUnicodeToString(&GlobalData.container.GUID, _____GlobalData_container_GUID);

    GlobalData.container.root_folder.data.key_.MaximumLength = sizeof(_____GlobalData_container_key_) - sizeof(L'\0');
    GlobalData.container.root_folder.data.key_.Length = 0;
    if(!(GlobalData.container.root_folder.data.key_.Buffer = GetMemory(GlobalData.container.root_folder.data.key_.MaximumLength))) goto init_Global_container_err_1;
    RtlAppendUnicodeToString(&GlobalData.container.root_folder.data.key_, _____GlobalData_container_key_);

    GlobalData.container.path_is_correct = path_is_correct_ALL;

    status = fill_RULE_CONTAINER(&GlobalData.container);
    if (!NT_SUCCESS( status )) goto init_Global_container_err_2;

    #ifdef UNIT_TEST
    //KdBreakPoint();
    if (Unit_test_1(&GlobalData.container)){
        DbgPrint("hide_file_filter!init: Unit_test1 passed.\n");
    } else {
        DbgPrint("hide_file_filter!init: Unit_test1 WAS NOT PASSED!!!!!!!!!\n");
    }
    #endif

    return status;

    init_Global_container_err_2:
    ClearUNIC_STR(&GlobalData.container.root_folder.data.key_);

    init_Global_container_err_1:
    ClearUNIC_STR(&GlobalData.container.GUID);

    return status;
}
NTSTATUS init_Globa_CDO_DATA(){
    NTSTATUS status = STATUS_SUCCESS;
    
    status = ExInitializeResourceLite(&GlobalData.CDO_data.local_rules.var_for_sync);
    if (!NT_SUCCESS(status)) return status;
    status = ExInitializeResourceLite(&GlobalData.CDO_data.GUIs.var_for_sync);
    if (!NT_SUCCESS(status)) goto init_Globa_CDO_DATA_err_1;
    status = ExInitializeResourceLite(&GlobalData.CDO_data.preferred.var_for_sync);
    if (!NT_SUCCESS(status)) goto init_Globa_CDO_DATA_err_2;

    GlobalData.CDO_data.local_rules.data = NULL;
    GlobalData.CDO_data.GUIs.data = NULL;
    GlobalData.CDO_data.preferred.data = NULL;

    //добавляем GlobalData.container, чтоб не писать отдельный код для него при обработке запросов.
    if (!MY_FLT_INSTANCE_CONTEXT_insert_element(&GlobalData.container, &GlobalData.CDO_data.local_rules.data)){
        status = STATUS_MEMORY_NOT_ALLOCATED;
        goto init_Globa_CDO_DATA_err_3;
    }

    goto init_Globa_CDO_DATA_OK_1;
    
    init_Globa_CDO_DATA_err_3:
    ExDeleteResourceLite(&GlobalData.CDO_data.preferred.var_for_sync);
    init_Globa_CDO_DATA_err_2:
    ExDeleteResourceLite(&GlobalData.CDO_data.GUIs.var_for_sync);
    init_Globa_CDO_DATA_err_1:
    ExDeleteResourceLite(&GlobalData.CDO_data.local_rules.var_for_sync);

    init_Globa_CDO_DATA_OK_1:
    return status;
}
VOID Free_Globa_CDO_DATA(){
    ExDeleteResourceLite(&GlobalData.CDO_data.preferred.var_for_sync);
    ExDeleteResourceLite(&GlobalData.CDO_data.GUIs.var_for_sync);
    ExDeleteResourceLite(&GlobalData.CDO_data.local_rules.var_for_sync);

    PID_clear_tree(&GlobalData.CDO_data.preferred.data);
    PGUI_CONTEXT_clear_tree(&GlobalData.CDO_data.GUIs.data);
    MY_FLT_INSTANCE_CONTEXT_clear_tree(&GlobalData.CDO_data.local_rules.data);
}

BOOLEAN is_preferred_sync(PID pid){
    BOOLEAN res = FALSE;
    ACTIONS_PRE_READING(&GlobalData.CDO_data.preferred.var_for_sync);
    res = (BOOLEAN)PID_finde(pid, &GlobalData.CDO_data.preferred.data);
    ACTIONS_POST_READING(&GlobalData.CDO_data.preferred.var_for_sync);
    return res;
}
VOID PROCESS_NOTIFY_ROUTINE ( IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create ){
    //может проверяться не только удаление процесса, а и создание, чтоб, если прислали фейковый идентификатор, это не повлияло на фильтрацию файлов для создаваемого процесса
    if (!Create /*удаление?*/){
        ACTIONS_PRE_WRITING(&GlobalData.CDO_data.preferred.var_for_sync);
        PID_delete_element(ProcessId, &GlobalData.CDO_data.preferred.data);
        ACTIONS_POST_WRITING(&GlobalData.CDO_data.preferred.var_for_sync);
    }
}

BOOLEAN sum_size_UNIC_STR_records_in_tree(PSIZE_T size, PUNICODE_STRING str){
    *size += 2*sizeof(SEPARATOR) + str->Length;
    return FALSE;
}
BOOLEAN sum_size_FOLDER_names_in_tree(PSIZE_T size, PFOLDER* ppfolder){
    return sum_size_UNIC_STR_records_in_tree(size, &(*ppfolder)->key_);
}

BOOLEAN write_UNIC_STR_names_to_buffer(WCHAR** pBuffer, PUNICODE_STRING str){
    (*pBuffer)[0] = SEPARATOR;
    RtlCopyMemory(*pBuffer + 1, str->Buffer, str->Length);
    ((char*)(*pBuffer)) += sizeof(SEPARATOR) + str->Length;
    (*pBuffer)[0] = SEPARATOR;
    ((char*)(*pBuffer)) += sizeof(SEPARATOR);
    return FALSE;
}
BOOLEAN write_FOLDER_names_to_buffer(WCHAR** pBuffer, PFOLDER* ppfolder){
    return write_UNIC_STR_names_to_buffer(pBuffer, &(*ppfolder)->key_);
}

BOOLEAN max_size_UNIC_STR_records_in_tree(PSIZE_T size, PUNICODE_STRING str){
    if (*size < str->Length) *size = str->Length;
    return FALSE;
}
BOOLEAN max_size_FOLDER_names_in_tree(PSIZE_T size, PFOLDER* ppfolder){
    return max_size_UNIC_STR_records_in_tree(size, &(*ppfolder)->key_);
}

BOOLEAN write_UNIC_STR_names_to_file(PINFO_FOR_write_to_file info, PUNICODE_STRING str){
    //info->Buffer[0] = SEPARATOR;
    RtlCopyMemory(info->Buffer + 1, str->Buffer, str->Length);
    *(WCHAR*)(((char*)info->Buffer) + sizeof(SEPARATOR) + str->Length) = SEPARATOR;

    info->status = ZwWriteFile(info->hFile, NULL, NULL, NULL, &info->IoStatusBlock, info->Buffer, str->Length + 2*sizeof(SEPARATOR), NULL, NULL);
    if (!NT_SUCCESS(info->status)) return TRUE;
    return FALSE;
}
BOOLEAN write_FOLDER_names_to_file(PINFO_FOR_write_to_file info, PFOLDER* ppfolder){
    return write_UNIC_STR_names_to_file(info, &(*ppfolder)->key_);
}

BOOLEAN how_many_nodes(PULONG n, PVOID ptr){
    (*n)++;
    return FALSE;
}





NTSTATUS ACTION_change_make_filtering(PVOID ptr){
    GlobalData.make_filtering = !GlobalData.make_filtering;
    return STATUS_SUCCESS;
}

NTSTATUS ACTION_about_pid(PABOUT_PID param){
    if (param->add){
        if (!PID_insert_element(param->pid, &GlobalData.CDO_data.preferred.data)){
            return STATUS_MEMORY_NOT_ALLOCATED;
        }
    }
    else PID_delete_element(param->pid, &GlobalData.CDO_data.preferred.data);
    return STATUS_SUCCESS;
}

NTSTATUS ACTION_about_rule(PABOUT_RULE param){
    if (param->add) return add_rule(&param->path, param->folder);
    else return delete_rule(&param->path, param->folder);
}

NTSTATUS ACTION_return_success(PVOID param){return STATUS_SUCCESS;}






BOOLEAN insert_message_from_queue_to_GUI_CONTEXT(PQUEUE(PREF_COUNT(UNICODE_STRING)) pqueue, PGUI_CONTEXT* ppGUI){
    PQUEUE_NODE(PREF_COUNT(UNICODE_STRING)) node = (PVOID)pqueue->first;
    ASSERT(node);
    KeWaitForSingleObject(&(*ppGUI)->mutex_for_messages, UserRequest /*Executive*/, KernelMode, FALSE, NULL);
    if ((*ppGUI)->messages.first){
        (*ppGUI)->messages.last->next = (PVOID)node;
        (*ppGUI)->messages.last = (PVOID)node;
    } else (*ppGUI)->messages.first = (PVOID)((*ppGUI)->messages.last = (PVOID)node);
    if (pqueue->first->next){
        pqueue->first = (PVOID)pqueue->first->next;
        node->next = NULL;
    } else pqueue->first = (PVOID)(pqueue->last = NULL);
    KeSetEvent(&(*ppGUI)->new_message, IO_NO_INCREMENT, FALSE);
    KeReleaseMutex(&(*ppGUI)->mutex_for_messages, FALSE); 
    return FALSE;
}

NTSTATUS make_action_and_sand_messages(PUNICODE_STRING _message, PVOID param, NTSTATUS (*ACTION)(PVOID param), BOOLEAN just_for_me){
    NTSTATUS status = STATUS_SUCCESS;
    PREF_COUNT(UNICODE_STRING) message = ExAllocatePool(PagedPool, sizeof(REF_COUNT(UNICODE_STRING)));
    KdBreakPoint();
    if (!message){
        status = STATUS_MEMORY_NOT_ALLOCATED;
        goto make_action_and_sand_messages_err_1;
    }
    RtlZeroMemory(message, sizeof(REF_COUNT(UNICODE_STRING)));
    message->refs = 1;
    RtlCopyMemory(&message->data, _message, sizeof(UNICODE_STRING));
    RtlZeroMemory(_message, sizeof(UNICODE_STRING));

    ACTIONS_PRE_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    {
        QUEUE(PREF_COUNT(UNICODE_STRING)) queue;//для того, чтоб создать все узлы очереди для сообщений и, если не получится создать все, иметь возможность удалить все существующие узлы
        PQUEUE_NODE(PREF_COUNT(UNICODE_STRING)) node = NULL;
        RtlZeroMemory(&queue, sizeof(queue));
        if (just_for_me){
            HANDLE pid = PsGetCurrentProcessId();
            PNODE(PGUI_CONTEXT) pnode_GUI = PGUI_CONTEXT_finde((PGUI_CONTEXT)&pid, &GlobalData.CDO_data.GUIs.data);
            ASSERT(pnode_GUI);
            if (node = ExAllocatePool(PagedPool, sizeof(QUEUE_NODE(PREF_COUNT(UNICODE_STRING))))){
                node->data = (PVOID)message;
                node->next = NULL;
                message->refs++;
                queue.first = (PVOID)(queue.last = (PVOID)node);
            } else {
                status = STATUS_MEMORY_NOT_ALLOCATED;
                goto make_action_and_sand_messages_err_2;
            }
            status = ACTION(param);
            if (!NT_SUCCESS(status)) goto make_action_and_sand_messages_err_2;
        
            insert_message_from_queue_to_GUI_CONTEXT((PVOID)&queue, &pnode_GUI->key_);
            goto make_action_and_sand_messages_OK_1;
        } else {
            ULONG n = 0, i;
            PGUI_CONTEXT_get_pnode_where_condition_is_true(&n, &GlobalData.CDO_data.GUIs.data, how_many_nodes);
            for(i = 0; i < n; i++){
                if (node = ExAllocatePool(PagedPool, sizeof(QUEUE_NODE(PREF_COUNT(UNICODE_STRING))))){
                    node->data = (PVOID)message;
                    node->next = NULL;
                    message->refs++;
                    if (queue.first){
                        queue.last->next = node;
                        queue.last = (PVOID)node;
                    } else queue.first = (PVOID)(queue.last = (PVOID)node);
                } else {
                    status = STATUS_MEMORY_NOT_ALLOCATED;
                    goto make_action_and_sand_messages_err_2;
                }
            }
            status = ACTION(param);
            if (!NT_SUCCESS(status)) goto make_action_and_sand_messages_err_2;
        
            PGUI_CONTEXT_get_pnode_where_condition_is_true(&queue, &GlobalData.CDO_data.GUIs.data, insert_message_from_queue_to_GUI_CONTEXT);
            goto make_action_and_sand_messages_OK_1;
        }

        make_action_and_sand_messages_err_2:
        Clear_QUEUE_of_messages(&queue);
    }

    make_action_and_sand_messages_OK_1:
    ACTIONS_POST_READING(&GlobalData.CDO_data.GUIs.var_for_sync);
    Free_PREF_COUNT(message, ClearUNIC_STR);

    make_action_and_sand_messages_err_1:
    return status;
}