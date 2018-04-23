#pragma once
using namespace std;

	#define VOID void
	#define PVOID void*
	#define SIZE_T size_t
	#define BOOLEAN bool
	//#define FALSE false
	//#define TRUE true
	#define LONG long


	#define KEY_TYPE int
	#define GetMemory(arg) malloc(arg);
	#define FreeMemory(arg) free(arg)
	#define ClearKey(arg)
	#define CompareKey(arg1, arg2) *(arg1) - *(arg2)

	#define _L 0	//(L - высота левого на 1 больше)
	#define _M 1	//(M - равновесие)
	#define _R 2	//(R - высота правого на 1 больше)

	//узел бинарного дерева
	struct NODE{																														
		KEY_TYPE key_;																													
		PVOID left_;																													
		PVOID right_;																													
		unsigned char balance_ : 2;																											
	};

	#define PNODE NODE*
	#define TREE NODE**
	#define _PNODE VOID*
	#define _TREE VOID**

	#define EMPTY(tree) (*tree) ? FALSE : TRUE
	/*при неудаче вернет NULL*/
	_TREE get_empty_tree(PVOID(GetMemory)(SIZE_T)){
		VOID** ptr = (VOID**)GetMemory(sizeof(PVOID));
		if (ptr){
			*ptr = NULL;
		}
		return ptr;
	}

	//GetMemory 	type is " PVOID(*)(SIZE_T) "
	//FreeMemory 	type is " VOID (*)(VOID*) "
	//ClearKey 		type is " VOID (*)(KEY_TYPE*) "
	//CompareKey	type is " LONG (*)(KEY_TYPE*, KEY_TYPE*) "
	#define BALANCED_TREE_ROUTINES_DEFINITION(Word, KEY_TYPE, GetMemory, FreeMemory, ClearKey, CompareKey)								/*\*/
	/*при неудаче вернет NULL*/																											/*\*/
	_PNODE /*Word ## */_______get_ptr_to_node(KEY_TYPE key) {																				/*\*/
		PNODE ptr = (PNODE)GetMemory(sizeof(NODE));																		/*\*/
		if (ptr){																														/*\*/
			ptr->key_ = key;																											/*\*/
			ptr->left_ = NULL;																											/*\*/
			ptr->right_ = NULL;																											/*\*/
			ptr->balance_ = _M;																											/*\*/
		}																																/*\*/
		return ptr;																														/*\*/
	}																																	/*\*/
	/*восcтановка баланса при перевесе слева*/																							/*\*/
	void /*Word ## */_______LL_LR(_TREE tree, BOOLEAN* h){																				/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		if (!(*h)) return;																												/*\*/
		switch (ptr->balance_){																											/*\*/
			case _L:{																													/*\*/
				PNODE tmp1;																									/*\*/
				PNODE tmp2;																									/*\*/
				tmp1 = (PNODE)ptr->left_;																										/*\*/
				if (tmp1->balance_ == _L){	/*LL*/																						/*\*/
					ptr->left_ = tmp1->right_;																							/*\*/
					tmp1->right_ = ptr;																									/*\*/
					ptr->balance_ = _M;																									/*\*/
					ptr = tmp1;																											/*\*/
				} else {					/*LR*/																						/*\*/
					tmp2 = (PNODE)tmp1->right_;																								/*\*/
					tmp1->right_ = tmp2->left_;																							/*\*/
					tmp2->left_ = tmp1;																									/*\*/
					ptr->left_ = tmp2->right_;																							/*\*/
					tmp2->right_ = ptr;																									/*\*/
					if (tmp2->balance_ == _L) ptr->balance_ = _R; else ptr->balance_ = _M;													/*\*/
					if (tmp2->balance_ == _R) tmp1->balance_ = _L; else tmp1->balance_ = _M;												/*\*/
					ptr = tmp2;																											/*\*/
				}																														/*\*/
				ptr->balance_ = _M;																										/*\*/
				(*tree) = ptr;																											/*\*/
				(*h) = FALSE;																											/*\*/
			}																															/*\*/
			break;																														/*\*/
			case _M:																														/*\*/
				ptr->balance_ = _L;																										/*\*/
				break;																													/*\*/
			case _R:																														/*\*/
				ptr->balance_ = _M;																										/*\*/
				(*h) = FALSE;																											/*\*/
		}																																/*\*/
	}																																	/*\*/
	/*восcтановка баланса при перевесе справа*/																							/*\*/
	void /*Word ## */_______RR_RL(_TREE tree, BOOLEAN* h){																				/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		if (!(*h)) return;																												/*\*/
		switch (ptr->balance_){																											/*\*/
			case _L:																														/*\*/
				ptr->balance_ = _M;																										/*\*/
				(*h) = FALSE;																											/*\*/
				break;																													/*\*/
			case _M:																														/*\*/
				ptr->balance_ = _R;																										/*\*/
				break;																													/*\*/
			case _R:{																													/*\*/
				PNODE tmp1;																									/*\*/
				PNODE tmp2;																									/*\*/
				tmp1 = (PNODE)ptr->right_;																										/*\*/
				if (tmp1->balance_ == _R){	/*RR*/																						/*\*/
					ptr->right_ = tmp1->left_;																							/*\*/
					tmp1->left_ = ptr;																									/*\*/
					ptr->balance_ = _M;																									/*\*/
					ptr = tmp1;																											/*\*/
				} else {					/*RL*/																						/*\*/
					tmp2 = (PNODE)tmp1->left_;																									/*\*/
					tmp1->left_ = tmp2->right_;																							/*\*/
					tmp2->right_ = tmp1;																								/*\*/
					ptr->right_ = tmp2->left_;																							/*\*/
					tmp2->left_ = ptr;																									/*\*/
					if (tmp2->balance_ == _R) ptr->balance_ = _L; else ptr->balance_ = _M;													/*\*/
					if (tmp2->balance_ == _L) tmp1->balance_ = _R; else tmp1->balance_ = _M;												/*\*/
					ptr = tmp2;																											/*\*/
				}																														/*\*/
				ptr->balance_ = _M;																										/*\*/
				(*tree) = ptr;																											/*\*/
				(*h) = FALSE;																											/*\*/
			}																															/*\*/
		}																																/*\*/
	}																																	/*\*/
	/*функция ничего не делает, если такой ключ уже есть. Параметры (ключ, дерево, возвращает TRUE при потребности балансировки)*/		/*\*/
	/*вернет NULL при неудаче или указатель на узел который подошел по ключу или который пришлось вставить в дерево*/					/*\*/
	_PNODE /*Word ## */_______insert_element_(KEY_TYPE key, _TREE tree, BOOLEAN* h){											/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		if (EMPTY(tree)){																												/*\*/
			(*tree) = /*Word ## */_______get_ptr_to_node(key);																						/*\*/
			ptr = (PNODE)(*tree);																												/*\*/
			if (ptr){																													/*\*/
				*h = TRUE;																												/*\*/
			} else {																													/*\*/
				*h = FALSE;																												/*\*/
			}																															/*\*/
		} else {																														/*\*/
			LONG compare_res = CompareKey(&key, &ptr->key_);																			/*\*/
			if (compare_res < 0){																										/*\*/
				ptr = (PNODE)/*Word ## */_______insert_element_(key, &ptr->left_, h);																		/*\*/
				/*Word ## */_______LL_LR(tree, h);																									/*\*/
			} else if (compare_res > 0){																								/*\*/
				ptr = (PNODE)/*Word ## */_______insert_element_(key, &ptr->right_, h);																		/*\*/
				/*Word ## */_______RR_RL(tree, h);																									/*\*/
			} else {																													/*\*/
				*h = FALSE;																												/*\*/
			}																															/*\*/
		}																																/*\*/
		return ptr;																														/*\*/
	}																																	/*\*/
	/*ищет справа нул, копирует поля, удаляет узел у которого справа нул*/																/*\*/
	void /*Word ## */_______del_(_TREE tree, _TREE rtree, BOOLEAN *h){															/*\*/
		if ((*(TREE)rtree)->right_ != NULL){																					/*\*/
			/*Word ## */_______del_(tree, (_TREE)(*(TREE)rtree)->right_, h);																		/*\*/
			/*Word ## */_______LL_LR(rtree, h);																										/*\*/
		} else {																														/*\*/
			PVOID tmp = (*rtree);																										/*\*/
			ClearKey(&(*(TREE)tree)->key_);																					/*\*/
			(*(TREE)tree)->key_ = (*(TREE)rtree)->key_;																/*\*/
			(*rtree) = (*(TREE)rtree)->left_;																					/*\*/
			(*h) = TRUE;																												/*\*/
			FreeMemory(tmp);																											/*\*/
		}																																/*\*/
	}																																	/*\*/
	/*(ключ, указатель на дерево, возвращает ТРУ при потребности балансировки)*/														/*\*/
	void /*Word ## */_______delete_element_(KEY_TYPE key, _TREE tree, BOOLEAN* h){ 														/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		if (EMPTY(tree)){																												/*\*/
			(*h) = FALSE;																												/*\*/
		} else {																														/*\*/
			LONG compare_res = CompareKey(&key, &ptr->key_);																			/*\*/
			if (compare_res > 0){																										/*\*/
				/*Word ## */_______delete_element_(key, &ptr->right_, h);																			/*\*/
				/*Word ## */_______LL_LR(tree, h);																									/*\*/
			} else if (compare_res < 0){																								/*\*/
				/*Word ## */_______delete_element_(key, &ptr->left_, h);																			/*\*/
				/*Word ## */_______RR_RL(tree, h);																									/*\*/
			} else {																													/*\*/
				if (ptr->right_ == NULL){																								/*\*/
					(*tree) = ptr->left_;																								/*\*/
					ClearKey(&ptr->key_);																								/*\*/
					FreeMemory(ptr);																									/*\*/
					(*h) = TRUE;																										/*\*/
				} else if (ptr->left_ == NULL){																							/*\*/
					(*tree) = ptr->right_;																								/*\*/
					ClearKey(&ptr->key_);																								/*\*/
					FreeMemory(ptr);																									/*\*/
					(*h) = TRUE;																										/*\*/
				} else {																												/*\*/
					/*Word ## */_______del_(tree, &ptr->left_, h);																					/*\*/
					/*Word ## */_______RR_RL(tree, h);																								/*\*/
				}																														/*\*/
			}																															/*\*/
		}																																/*\*/
	}																																	/*\*/
	/*возвращает NULL или указатель на уезл с ключом (key)*/																			/*\*/
	_PNODE /*Word ## */finde(KEY_TYPE key, _TREE tree){																			/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		LONG compare_res;																												/*\*/
		KEY_TYPE* pkey = &key;																											/*\*/
		while (ptr){																													/*\*/
			compare_res = CompareKey(pkey, &ptr->key_);																					/*\*/
			if (compare_res < 0){																										/*\*/
				ptr = (PNODE)ptr->left_;																										/*\*/
			} else if (compare_res > 0){																								/*\*/
				ptr = (PNODE)ptr->right_;																										/*\*/
			} else {																													/*\*/
				return ptr;																												/*\*/
			}																															/*\*/
		}																																/*\*/
		return NULL;																													/*\*/
	}																																	/*\*/
	_PNODE /*Word ## */insert_element(KEY_TYPE key, _TREE tree){																/*\*/
		BOOLEAN b;																														/*\*/
		return /*Word ## */_______insert_element_(key, tree, &b);																					/*\*/
	}																																	/*\*/
	void /*Word ## */delete_element(KEY_TYPE key, _TREE tree){																			/*\*/
		BOOLEAN b;																														/*\*/
		/*Word ## */_______delete_element_(key, tree, &b);																							/*\*/
	}																																	/*\*/
	/*очистить дерево*/																													/*\*/
	void /*Word ## */clear_tree(_TREE tree){																								/*\*/
		PNODE ptr = (PNODE)(*tree);																									/*\*/
		if (EMPTY(tree)) return;																										/*\*/
		ClearKey(&ptr->key_);																											/*\*/
		/*Word ## */clear_tree(&ptr->right_);																										/*\*/
		/*Word ## */clear_tree(&ptr->left_);																										/*\*/
		FreeMemory(ptr);																												/*\*/
	}																																	/*\*/
	/*удалить дерево*/																													/*\*/
	void /*Word ## */delete_tree(_TREE tree){																								/*\*/
		/*Word ## */clear_tree(tree);																												/*\*/
		FreeMemory(tree);																												/*\*/
	}																																	/*\*/
	/*возвращает NULL или указатель на уезл, ключ которого удовлетворет condition (param передается в condition)*/						/*\*/
	_PNODE /*Word ## */get_pnode_where_conition_is_true(PVOID param, _TREE tree, BOOLEAN (condition)(PVOID, KEY_TYPE*)){		/*\*/
		if (EMPTY(tree)) return NULL;																									/*\*/
		{																																/*\*/
			PNODE ptr = (PNODE)(*tree);																								/*\*/
			if (condition(param, &ptr->key_)) return ptr;																				/*\*/
			if (ptr	= (PNODE)/*Word ## */get_pnode_where_conition_is_true(param, &ptr->left_, condition)) return ptr;										/*\*/
			ptr = (PNODE)(*tree);																										/*\*/
			if (ptr	= (PNODE)/*Word ## */get_pnode_where_conition_is_true(param, &ptr->right_, condition)) return ptr;										/*\*/
		}																																/*\*/
		return NULL;																													/*\*/
	}																																	/*\*/

