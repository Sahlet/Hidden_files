#ifdef DBG

//stuectures_and_routines_test
BOOLEAN Unit_test_1(__inout PRULE_CONTAINER container){
	NTSTATUS status;
    UNICODE_STRING test_rule, test_path, test_Name, test_full_Name;
    //KdBreakPoint();
    test_rule.Buffer = L"*1";
    test_rule.Length = 4;

    test_path.Buffer = L"some_path";
    test_path.Length = 18;

    test_Name.Buffer = L"1";
    test_Name.Length = 2;

    test_full_Name.Buffer = L"some_path" L"\\" L"1";
    test_full_Name.Length = 22;

    status = add_rule_sync(&test_rule, container);
    if (NT_SUCCESS(status)){
    	if (! Must_be_hidden_sync(&test_path, &test_Name, &test_full_Name, container)) return FALSE;
        
        delete_rule_sync(&test_rule, container);
        test_rule.Buffer = L"*2";
        status = add_rule_sync(&test_rule, container);
        if (NT_SUCCESS(status)){
        	if (Must_be_hidden_sync(&test_path, &test_Name, &test_full_Name, container)) return FALSE;
    	}
    }

    return NT_SUCCESS(status);
}

//stuectures_and_routines_test
BOOLEAN Unit_test_2(__inout PRULE_CONTAINER container){
	NTSTATUS status;
    UNICODE_STRING test_rule, test_path, test_Name, test_full_Name;
    //KdBreakPoint();
    test_rule.Buffer = L"*2";
    test_rule.Length = 4;

    test_path.Buffer = L"some_path";
    test_path.Length = 18;

    test_Name.Buffer = L"1";
    test_Name.Length = 2;

    test_full_Name.Buffer = L"some_path" L"\\" L"1";
    test_full_Name.Length = 22;

    status = add_rule_sync(&test_rule, container);
    if (NT_SUCCESS(status)){
    	if (Must_be_hidden_sync(&test_path, &test_Name, &test_full_Name, container)) return FALSE;
        
        delete_rule_sync(&test_rule, container);
        test_rule.Buffer = L"*1";
        status = add_rule_sync(&test_rule, container);
        if (NT_SUCCESS(status)){
        	if (!Must_be_hidden_sync(&test_path, &test_Name, &test_full_Name, container)) return FALSE;
    	}
    }

    return NT_SUCCESS(status);
}

#endif