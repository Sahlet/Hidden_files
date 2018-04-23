#ifndef _SHAREAD_DEFS_H
#define _SHAREAD_DEFS_H

#define CDO_PATH               L"\\FileSystem\\Filters\\Hide_files_CDO"
#define CDO_SYMBOLIC_LINK_PATH L"\\??\\Hide_files_CDO"
#define CDO_SYMBOLIC_LINK_NAME L"Hide_files_CDO"


#define _____GlobalData_container_key_ L"*:"
#define _____GlobalData_container_GUID L"GlobalRules"

/*
DeviceIoControl
D1. Получить правила для конкретной папки.
	D1.1 получить суммарный размер памяти для (имен подпапок)\масок\правил в папке.
	D1.2 запольнит буффер (именами подпапок)\масоками\правилами (не хватило памяти - проблемы того, кто предоставил буффер!).
	D1.3 получить (имена подпапок)\масоки\правила в файле (драйвер создает файл в папке systemroot\temp и возвращает имя (без пути) в ответ).
D2. команда - вопрос: этот путь будет скрываться или нет? (параметр команды - путь)
D3. команда - вопрос: фильтр скрывает или нет?


WriteFile
W1. дать/удалить привилегию процессу по PID
W2. Добавить правило
W3. Удалить правило
W4. Команда фильтровать/не фильтровать
W5. Прислать сообщение мне
W6. Прислать сообщение всем
//////////////////////////////////////////////////////////////////////////////////////////////////////
////для команд W1 - W4 можно указывать бит, который определяет, выполнять операцию аонимно или нет.
//////////////////////////////////////////////////////////////////////////////////////////////////////


ReadFile
R1. выполнелась одна из W команд (пользователь читает команду, что прислал WriteFile)




путь приходит в таком виде \\??\\C:\\some_path\\...\\file
если глобальное правило, то      *:\\some_path\\...\\file
*/

#define D_1_1_FOLDER_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_2_FOLDER_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_3_FOLDER_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_1_NAME_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_2_NAME_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x822, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_3_NAME_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x823, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_1_MASK_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x831, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_2_MASK_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x832, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_1_3_MASK_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x833, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_2_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define D_3_CTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IO_ERR_NO_SEPARATOR 						0xFFFFFF00
#define IO_ERR_INVALID_PATH 						0xFFFFFF01
#define IO_ERR_INVALID_VOLUME_NAME 					0xFFFFFF02
#define IO_ERR_INVALID_CMD 							0xFFFFFF03
#define IO_ERR_INVALID_CTL 							IO_ERR_INVALID_CMD
#define IO_ERR_BUFFER_TOO_BIG						0xFFFFFF04
#define IO_ERR_YOU_CAN_NOT_DELETE_THE_RULE 			0xFFFFFF05
#define IO_ERR_IRQL_TOO_BIG							0xFFFFFF06

#define THERE_WAS_NOT_THIS_RULE 					0x7FFFFF00
#define THERE_WAS_THIS_RULE 						0x7FFFFF01
#define IT_WAS_FOLDER 								0x7FFFFF10
#define IT_WAS_NAME 								0x7FFFFF20
#define IT_WAS_MASK 								0x7FFFFF30

enum cmd {
	ANONYMOUSLY = 1,
	ADD_PRIVILEGE_PID = 2,
	REMOVE_PRIVILEGE_PID = 4,
	ADD_RULE = 6,
	REMOVE_RULE = 8,
	MAKE_FILTERING = 10,
	DO_NOT_MAKE_FILTERING = 12,
	
	SEND_MESSAGE_TO_ME = 14,
	SEND_MESSAGE_TO_ALL = 16
};

#define MAX_RULE_LENGTH 32600
#define MAX_CMD_LENGTH 32750

#define SEPARATOR L':'

#endif