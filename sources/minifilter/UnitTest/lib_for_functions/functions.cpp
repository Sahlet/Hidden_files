#include "stdafx.h"
#include <Windows.h>
#include <winioctl.h>
#include "..\..\src\shared_defs.h"
#include "..\UnitTest\shared_h.h"
#include <list>
#include "tree.h"
using namespace std;

bool tree_test_1_func(PVOID list_, KEY_TYPE* Pkey){
	((list<int>*)list_)->push_back(*Pkey);
	return false;
}
bool tree_test_1(){
			int tmp[] = {2, 1, 3/**/, 2, 3/**/, 3, 2, -2, 6, 5, 7};
			list<int> list1(tmp, tmp + sizeof(tmp) / sizeof(int)), list2;
			_TREE tree = get_empty_tree(malloc);
			get_pnode_where_conition_is_true(&list2, tree, tree_test_1_func);
			insert_element(1, tree);
			insert_element(1, tree);
			insert_element(2, tree);
			insert_element(3, tree);
			get_pnode_where_conition_is_true(&list2, tree, tree_test_1_func);
			delete_element(1, tree);
			get_pnode_where_conition_is_true(&list2, tree, tree_test_1_func);
			insert_element(7, tree);
			insert_element(6, tree);
			insert_element(-2, tree);
			insert_element(5, tree);
			get_pnode_where_conition_is_true(&list2, tree, tree_test_1_func);
			delete_tree(tree);
			return list1 == list2;
}


struct HANDLE_guard{
	HANDLE h;
	HANDLE_guard(HANDLE h);
	HANDLE_guard();
	~HANDLE_guard();
	operator HANDLE(){
		return h;
	}
};

bool success = true;
HANDLE_guard::HANDLE_guard(HANDLE h) : h(h){}
HANDLE_guard::HANDLE_guard(){
	h = CreateFile(	(LPCTSTR)( L"\\\\.\\" CDO_SYMBOLIC_LINK_NAME),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
	if (h == INVALID_HANDLE_VALUE){ success = false; }
}
HANDLE_guard::~HANDLE_guard(){
	if (h != INVALID_HANDLE_VALUE){
		CloseHandle(h);
	}
}

HANDLE_guard hCDO;

bool CDO_test_1(){
	if (!success) return false;
	DWORD dwReturnBytes;
		char inBuff [32000];
	char outBuff[32000];

	#define IN_1 L"\\??\\C:\\"
	for(int i = 0; i < sizeof(IN_1); i++) inBuff[i] = ((char*)IN_1)[i];
	if (!((bool)DeviceIoControl(hCDO, D_1_1_FOLDER_CTL,
					IN_1,sizeof(IN_1) - sizeof(L'\0'),//in
					outBuff,sizeof(outBuff),//out
					&dwReturnBytes,NULL))) return false;

	
	#define IN_2 L"*:\\Folder\\*.myf"

	int cmd = ADD_RULE;
	*(int*)inBuff = cmd;
	for(int i = sizeof(cmd); i < sizeof(IN_2) + sizeof(cmd); i++) inBuff[i] = ((char*)IN_2)[i - sizeof(cmd)];
	if (!(success = (bool)WriteFile(hCDO, inBuff,
				sizeof(cmd) + sizeof(IN_2) - sizeof(L'\0'),
				&dwReturnBytes,NULL))) return false;


	#define IN_3 L"*:\\Folder\\"
	for(int i = 0; i < sizeof(IN_3); i++) inBuff[i] = ((char*)IN_3)[i];
	if (!(success = (bool)DeviceIoControl(hCDO, D_1_1_MASK_CTL,
					inBuff,sizeof(IN_3) - sizeof(L'\0'),
					outBuff,sizeof(outBuff),
					&dwReturnBytes,NULL))) return false;
	if (dwReturnBytes != sizeof(size_t)) return false;
	if (*((size_t*)outBuff) < sizeof(L":*.myf:") - sizeof(L'\0')) return false;

	return success;
}