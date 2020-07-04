#ifndef __PROCESSES_H__
#define __PROCESSES_H__

#include <Windows.h>
#include <Psapi.h>
#include <wchar.h>

#pragma comment (lib, "psapi.lib")

#define WINDOW_STYLE WS_OVERLAPPEDWINDOW
#define STATIC_STYLE (WS_CHILD | WS_VISIBLE | WS_THICKFRAME)
#define PROCESS_BUTTON_STYLE (WS_CHILD | WS_VISIBLE)
#define PROCESS_NAME_EDIT_STYLE (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER)
#define LIBRARY_NAME_EDIT_STYLE (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER)
#define FUNCTION_NAME_EDIT_STYLE (WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER)
#define INJECT_BUTTON_STYLE (WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER)
#define FILE_ACCESS_ATTRUBUTES FILE_READ_DATA
#define PROCESS_ACCESS_FLAGS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION)
#define PROTECT_FLAGS PAGE_READWRITE
#define REMOTE_THREAD_STACK_SIZE 4096
#define MAX_PROCESSES_COUNT 256
#define MAX_MODULES_COUNT 256
#define PROCESSES_BUFFER_SIZE 4096
#define PROCESS_INFO_BUFFER_LENGTH 16384
#define UPDATE_RATE 1
#define BUFFER_SIZE PROCESS_INFO_BUFFER_LENGTH

#define LOAD_LIBRARY_STRING "LoadLibraryW"

#define MAIN_CLAS_NAME L"MainClass"
#define CAPTION L"DLL-injecter"
#define EDIT_CLASS_NAME L"EDIT"
#define STATIC_CLASS_NAME L"STATIC"
#define BUTTON_CLASS_NAME L"BUTTON"
#define INJECT_BUTTON_TEXT L"INJECT!"
#define EDIT_FONT_NAME L"CONSOLAS"
#define BUTTON_FONT_NAME L"CONSOLAS"
#define ERROR_STRING L"Error!"
#define KERNEL32_STRING L"Kernel32.DLL"
#define STATIC_FONT_NAME L"Courier new"
#define SUCCESS_STRING L"Successfully injected!"
#define INCORRECT_INPUT_STRING L"Incorrect input!"
#define PROCESS_NAME_EDIT_TEXT L"Enter process name"
#define LIBRARY_NAME_EDIT_TEXT L"Enter library name"
#define FUNCTION_NAME_EDIT_TEXT L"Enter function name"
#define ENTER_PROCESS_NAME_STRING L"Enter process name!"
#define ENTER_LIBRARY_NAME_STRING L"Enter library name!"
#define ENTER_FUNCTION_NAME_STRING L"Enter funciton name!"
#define WRONG_PROCESS_NAME_STRING L"Incorrect process name!"
#define WRONG_LIBRARY_NAME_STRING L"Incorrect library name!"
#define WRONG_FUNCTION_NAME_STRING L"Incorrect function name!"
#define LIBRARY_ANALYZE_FAIL_STRING L"Failed to analyze library from this process!"
#define LIBRARY_LOADING_FAIL_STRING L"Failed to load library to the selected process!"
#define FAIL_ALLOCATIONG_MEMORY_STRING L"Failed to allocate memory in selected process!"
#define FAIL_COPYING_MEMORY_STRING L"Failed to copy file name from your process to selected process!"
#define FAIL_STARTING_REMOTE_PROCEDURE_STRING L"Failed to start remote thread on selected function in selected remote process!"

#define Append(destination, source) wcscat_s ((wchar_t *)(destination), (int)(BUFFER_SIZE), (wchar_t *)(source));

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Init(HINSTANCE hInstance);
void Release();
void GetProcessesList();
void ShowInfo(int index);
void MoveWindows();
void GetProcessModulesInfo(int index, wchar_t *result);
HANDLE GetProcessByName(wchar_t *processName);
BOOL Inject(wchar_t *fileName, wchar_t *processName, char *functionName);
void Error(const wchar_t *text, const wchar_t *caption);
void Info(const wchar_t *text, const wchar_t *caption);
void GetStringFromInt(int value, wchar_t *result);

#endif // __PROCESSES_H__