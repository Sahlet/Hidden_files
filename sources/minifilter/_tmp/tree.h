/*
	tree.h
		определяет функции оперирования бинарными сбалансированными деревьями
*/
#ifndef MY_TREE
	#define MY_TREE
	#include <ntdef.h>
	//весы 
    //REVIEW_REC: можно использовать emun или константы, но не макросы
	#define _L 0	//(L - высота левого на 1 больше)
	#define _M 1	//(M - равновесие)
	#define _R 2	//(R - высота правого на 1 больше)

	//узел бинарного дерева
	#define NODE(KEY_TYPE)																															\
	struct {																																		\
		KEY_TYPE key_;																																\
		PVOID left_;																																\
		PVOID right_;																																\
		unsigned char balance_;																														\
	}

	#define PNODE(KEY_TYPE) NODE(KEY_TYPE)*
	#define TREE(KEY_TYPE) NODE(KEY_TYPE)**
	#define _PNODE(KEY_TYPE) VOID*
	#define _TREE(KEY_TYPE) VOID**

    //REVIEW: тут бы еще проверить tree != NULL перед разименованием
	#define EMPTY(tree) (*tree) ? FALSE : TRUE
	/*при неудаче вернет NULL*/
	static VOID** get_empty_tree(PVOID(GetMemory)(SIZE_T)){
		VOID** ptr = (VOID**)GetMemory(sizeof(PVOID));
		if (ptr){
			*ptr = NULL;
		}
		return ptr;
	}

	#define BALANCED_TREE_ROUTINES_DECLARATION(Word, KEY_TYPE)																						\
	/*возвращает NULL или указатель на уезл с ключом (key)*/																						\
	extern _PNODE(KEY_TYPE) Word ## finde(KEY_TYPE key, _TREE(KEY_TYPE) tree);																		\
	extern _PNODE(KEY_TYPE) Word ## insert_element(KEY_TYPE key, _TREE(KEY_TYPE) tree);																\
	extern void Word ## delete_element(KEY_TYPE key, _TREE(KEY_TYPE) tree);																			\
	/*очистить дерево*/																																\
	extern void Word ## clear_tree(_TREE(KEY_TYPE) tree);																							\
	/*удалить дерево*/																																\
	extern void Word ## delete_tree(_TREE(KEY_TYPE) tree);																							\
	/*возвращает NULL или указатель на уезл, ключ которого удовлетворет condition (param передается в condition)*/									\
	extern _PNODE(KEY_TYPE) Word ## get_pnode_where_condition_is_true(PVOID param, _TREE(KEY_TYPE) tree, BOOLEAN (condition)(PVOID, KEY_TYPE*));

	//REVIEW_REC: лучше typedef-ы сделать на все эти функции
	//GetMemory 	type is " PVOID(*)(SIZE_T) "
	//FreeMemory 	type is " VOID (*)(VOID*) "
	//ClearKey 		type is " VOID (*)(KEY_TYPE*) "
	//CompareKey	type is " LONG (*)(KEY_TYPE*, KEY_TYPE*) "
	#define BALANCED_TREE_ROUTINES_DEFINITION(Word, KEY_TYPE, GetMemory, FreeMemory, ClearKey, CompareKey)											\
	/*при неудаче вернет NULL*/																														\
	PNODE(KEY_TYPE) Word ## _______get_ptr_to_node(KEY_TYPE key) {																					\
		PNODE(KEY_TYPE) ptr = GetMemory(sizeof(NODE(KEY_TYPE)));																					\
		if (ptr){																																	\
			ptr->key_ = key;																														\
			ptr->left_ = NULL;																														\
			ptr->right_ = NULL;																														\
			ptr->balance_ = _M;																														\
		}																																			\
		return (PVOID)ptr;																															\
	}																																				\
	/*восcтановка баланса при перевесе слева*/																										\
	void Word ## _______LL_LR(_TREE(KEY_TYPE) tree, BOOLEAN* h){																					\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		if (!(*h)) return;																															\
		switch (ptr->balance_){																														\
			case _L:{																																\
				PNODE(KEY_TYPE) tmp1;																												\
				PNODE(KEY_TYPE) tmp2;																												\
				tmp1 = ptr->left_;																													\
				if (tmp1->balance_ == _L){	/*LL*/																									\
					ptr->left_ = tmp1->right_;																										\
					tmp1->right_ = ptr;																												\
					ptr->balance_ = _M;																												\
					ptr = (PVOID)tmp1;																												\
				} else {					/*LR*/																									\
					tmp2 = tmp1->right_;																											\
					tmp1->right_ = tmp2->left_;																										\
					tmp2->left_ = tmp1;																												\
					ptr->left_ = tmp2->right_;																										\
					tmp2->right_ = ptr;																												\
					if (tmp2->balance_ == _L) ptr->balance_ = _R; else ptr->balance_ = _M;															\
					if (tmp2->balance_ == _R) tmp1->balance_ = _L; else tmp1->balance_ = _M;														\
					ptr = (PVOID)tmp2;																												\
				}																																	\
				ptr->balance_ = _M;																													\
				(*tree) = (PVOID)ptr;																												\
				(*h) = FALSE;																														\
			}																																		\
			break;																																	\
			case _M:																																\
				ptr->balance_ = _L;																													\
				break;																																\
			case _R:																																\
				ptr->balance_ = _M;																													\
				(*h) = FALSE;																														\
		}																																			\
	}																																				\
	/*восcтановка баланса при перевесе справа*/																										\
	void Word ## _______RR_RL(_TREE(KEY_TYPE) tree, BOOLEAN* h){																					\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		if (!(*h)) return;																															\
		switch (ptr->balance_){																														\
			case _L:																																\
				ptr->balance_ = _M;																													\
				(*h) = FALSE;																														\
				break;																																\
			case _M:																																\
				ptr->balance_ = _R;																													\
				break;																																\
			case _R:{																																\
				PNODE(KEY_TYPE) tmp1;																												\
				PNODE(KEY_TYPE) tmp2;																												\
				tmp1 = ptr->right_;																													\
				if (tmp1->balance_ == _R){	/*RR*/																									\
					ptr->right_ = tmp1->left_;																										\
					tmp1->left_ = ptr;																												\
					ptr->balance_ = _M;																												\
					ptr = (PVOID)tmp1;																												\
				} else {					/*RL*/																									\
					tmp2 = tmp1->left_;																												\
					tmp1->left_ = tmp2->right_;																										\
					tmp2->right_ = tmp1;																											\
					ptr->right_ = tmp2->left_;																										\
					tmp2->left_ = ptr;																												\
					if (tmp2->balance_ == _R) ptr->balance_ = _L; else ptr->balance_ = _M;															\
					if (tmp2->balance_ == _L) tmp1->balance_ = _R; else tmp1->balance_ = _M;														\
					ptr = (PVOID)tmp2;																												\
				}																																	\
				ptr->balance_ = _M;																													\
				(*tree) = (PVOID)ptr;																												\
				(*h) = FALSE;																														\
			}																																		\
		}																																			\
	}																																				\
	/*функция ничего не делает, если такой ключ уже есть. Параметры (ключ, дерево, возвращает TRUE при потребности балансировки)*/					\
	/*вернет NULL при неудаче или указатель на узел который подошел по ключу или который пришлось вставить в дерево*/								\
	_PNODE(KEY_TYPE) Word ## _______insert_element_(KEY_TYPE key, _TREE(KEY_TYPE) tree, BOOLEAN* h){												\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		if (EMPTY(tree)){																															\
			(*tree) = (PVOID)Word ## _______get_ptr_to_node(key);																					\
			ptr = (PVOID)(*tree);																													\
			if (ptr){																																\
				*h = TRUE;																															\
			} else {																																\
				*h = FALSE;																															\
			}																																		\
		} else {																																	\
			LONG compare_res = CompareKey((PVOID)&key, (PVOID)&ptr->key_);																			\
			if (compare_res < 0){																													\
				ptr = (PVOID)Word ## _______insert_element_(key, (PVOID)&ptr->left_, h);															\
				Word ## _______LL_LR((PVOID)tree, (PVOID)h);																						\
			} else if (compare_res > 0){																											\
				ptr = (PVOID)Word ## _______insert_element_(key, (PVOID)&ptr->right_, h);															\
				Word ## _______RR_RL((PVOID)tree, (PVOID)h);																						\
			} else {																																\
				(*h) = FALSE;																														\
			}																																		\
		}																																			\
		return (PVOID)ptr;																															\
	}																																				\
	/*ищет справа нул, копирует поля, удаляет узел у которого справа нул*/																			\
	void Word ## _______del_(_TREE(KEY_TYPE) tree, _TREE(KEY_TYPE) rtree, BOOLEAN *h){																\
		if ((*(TREE(KEY_TYPE))rtree)->right_ != NULL){																								\
			Word ## _______del_(tree, (*(TREE(KEY_TYPE))rtree)->right_, h);																			\
			Word ## _______LL_LR((PVOID)rtree, h);																									\
		} else {																																	\
			PVOID tmp = (*rtree);																													\
			ClearKey(&(*(TREE(KEY_TYPE))tree)->key_);																								\
			(*(TREE(KEY_TYPE))tree)->key_ = (*(TREE(KEY_TYPE))rtree)->key_;																			\
			(*(TREE(KEY_TYPE))rtree) = (*(TREE(KEY_TYPE))rtree)->left_;																				\
			(*h) = TRUE;																															\
			FreeMemory(tmp);																														\
		}																																			\
	}																																				\
	/*(ключ, указатель на дерево, возвращает ТРУ при потребности балансировки)*/																	\
	void Word ## _______delete_element_(KEY_TYPE key, _TREE(KEY_TYPE) tree, BOOLEAN* h){ 															\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		if (EMPTY(tree)){																															\
			(*h) = FALSE;																															\
		} else {																																	\
			LONG compare_res = CompareKey((PVOID)&key, (PVOID)&ptr->key_);																			\
			if (compare_res > 0){																													\
				Word ## _______delete_element_(key, (PVOID)&ptr->right_, h);																		\
				Word ## _______LL_LR((PVOID)tree, h);																								\
			} else if (compare_res < 0){																											\
				Word ## _______delete_element_(key, (PVOID)&ptr->left_, h);																			\
				Word ## _______RR_RL((PVOID)tree, h);																								\
			} else {																																\
				if (ptr->right_ == NULL){																											\
					(*tree) = ptr->left_;																											\
					ClearKey(&ptr->key_);																											\
					FreeMemory((PVOID)ptr);																											\
					(*h) = TRUE;																													\
				} else if (ptr->left_ == NULL){																										\
					(*tree) = ptr->right_;																											\
					ClearKey(&ptr->key_);																											\
					FreeMemory((PVOID)ptr);																											\
					(*h) = TRUE;																													\
				} else {																															\
					Word ## _______del_((PVOID)tree, (PVOID)&ptr->left_, h);																		\
					Word ## _______RR_RL((PVOID)tree, h);																							\
				}																																	\
			}																																		\
		}																																			\
	}																																				\
	/*возвращает NULL или указатель на уезл с ключом (key)*/																						\
	_PNODE(KEY_TYPE) Word ## finde(KEY_TYPE key, _TREE(KEY_TYPE) tree){																				\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		LONG compare_res;																															\
		KEY_TYPE* pkey = &key;																														\
		while (ptr){																																\
			compare_res = CompareKey(pkey, &ptr->key_);																								\
			if (compare_res < 0){																													\
				ptr = ptr->left_;																													\
			} else if (compare_res > 0){																											\
				ptr = ptr->right_;																													\
			} else {																																\
				return (PVOID)ptr;																													\
			}																																		\
		}																																			\
		return NULL;																																\
	}																																				\
	_PNODE(KEY_TYPE) Word ## insert_element(KEY_TYPE key, _TREE(KEY_TYPE) tree){																	\
		BOOLEAN b;																																	\
		return (PVOID)Word ## _______insert_element_(key, (PVOID)tree, &b);																			\
	}																																				\
	void Word ## delete_element(KEY_TYPE key, _TREE(KEY_TYPE) tree){																				\
		BOOLEAN b;																																	\
		Word ## _______delete_element_(key, (PVOID)tree, &b);																						\
	}																																				\
	/*очистить дерево*/																																\
	void Word ## clear_tree(_TREE(KEY_TYPE) tree){																									\
		PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																										\
		if (EMPTY(tree)) return;																													\
		ClearKey(&ptr->key_);																														\
		Word ## clear_tree((PVOID)&ptr->right_);																									\
		Word ## clear_tree((PVOID)&ptr->left_);																										\
		FreeMemory(ptr);																															\
		(*tree) = NULL;																																\
	}																																				\
	/*удалить дерево*/																																\
	void Word ## delete_tree(_TREE(KEY_TYPE) tree){																									\
		Word ## clear_tree((PVOID)tree);																											\
		FreeMemory(tree);																															\
	}																																				\
	/*возвращает NULL или указатель на уезл, ключ которого удовлетворет condition (param передается в condition)*/									\
	_PNODE(KEY_TYPE) Word ## get_pnode_where_condition_is_true(PVOID param, _TREE(KEY_TYPE) tree, BOOLEAN (condition)(PVOID, KEY_TYPE*)){			\
		if (EMPTY(tree)) return NULL;																												\
		{																																			\
			PNODE(KEY_TYPE) ptr = (PVOID)(*tree);																									\
			if (condition(param, &ptr->key_)) return (PVOID)ptr;																					\
			if (ptr	= (PVOID)Word ## get_pnode_where_condition_is_true(param, (PVOID)&ptr->left_, condition)) return (PVOID)ptr;					\
			ptr = (PVOID)(*tree);																													\
			if (ptr	= (PVOID)Word ## get_pnode_where_condition_is_true(param, (PVOID)&ptr->right_, condition)) return (PVOID)ptr;					\
		}																																			\
		return NULL;																																\
	}																																				\
	//the end of BALANCED_TREE_ROUTINES_DEFINITION
#endif