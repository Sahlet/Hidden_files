// exe_test.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "..\UnitTest\shared_h.h"
#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[]){

	cout << "tree_test_1\t" << tree_test_1() << endl;
	cout << "CDO_test_1 \t" << CDO_test_1() << endl;
	
	system("pause");
	
	return 0;
}