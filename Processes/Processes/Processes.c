#include "Processes.h"

int buttonWidth, buttonHeight, processesCount, openedProcessesCount;
char *functionName;
RECT clientRect, injectPanelRect;
HFONT hStaticFont;
DWORD *processIds, *processIdsOld;
HANDLE processes[MAX_PROCESSES_COUNT], hSelectedProcess;
HWND buttons[MAX_PROCESSES_COUNT], hMainWindow, hStaticLeft, hStaticRight, hProcessNameEdit, hLibraryNameEdit, hFunctionNameEdit, hInjectButton;
HMODULE *processModules[MAX_PROCESSES_COUNT], hKernel32;
MODULEINFO *processModulesInfo[MAX_PROCESSES_COUNT];
wchar_t *processNames[MAX_PROCESSES_COUNT], *processName, *libraryName;

WNDCLASSEXW classEx =
{
	sizeof(WNDCLASSEX), 0, (WNDPROC)WindowProc, 0, 0, 0, 0, 0, 0, 0, MAIN_CLAS_NAME, 0
};

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	Init(hInstance);
	
	while (GetMessageW(&msg, (HWND)0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	Release();

	return 0;
}

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int source, index, i;
	HANDLE handle;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
			return 0;
		}
		break;
	case WM_COMMAND:
		source = HIWORD(lParam);
		if (source > 1) {
			handle = (HANDLE)lParam;
			if (handle == hInjectButton) {
				wmemset(processName, (wchar_t)0, MAX_PATH);
				wmemset(libraryName, (wchar_t)0, MAX_PATH);
				memset(functionName, (char)0, MAX_PATH);
				if (!GetWindowTextW(hProcessNameEdit, processName, MAX_PATH)) {
					Error(ENTER_PROCESS_NAME_STRING, INCORRECT_INPUT_STRING);
					return 0;
				}
				if (!GetWindowTextW(hLibraryNameEdit, libraryName, MAX_PATH)) {
					Error(ENTER_LIBRARY_NAME_STRING, INCORRECT_INPUT_STRING);
					return 0;
				}
				if (!GetWindowTextA(hFunctionNameEdit, functionName, MAX_PATH)) {
					Error(ENTER_FUNCTION_NAME_STRING, INCORRECT_INPUT_STRING);
					return 0;
				}
				Inject(libraryName, processName, functionName);
			} else {
				index = -1;
				for (i = 0; i < MAX_PROCESSES_COUNT; i++) {
					if (buttons[i]) {
						if (buttons[i] == handle) {
							index = i;
							break;
						}
					} else {
						return DefWindowProcW(hWnd, uMsg, wParam, lParam);
					}
				}
				if (index != -1) {
					hSelectedProcess = processes[index];
					ShowInfo(index);
				}
			}
			return 0;
		}
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &clientRect);
		InvalidateRect(hWnd, (RECT *)NULL, FALSE);
		buttonWidth = clientRect.right / 4;
		buttonHeight = clientRect.bottom / openedProcessesCount;
		MoveWindows();
		return 0;
	case WM_TIMER:
		GetProcessesList();
		return 0;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &clientRect, (HBRUSH)0);
		EndPaint(hWnd, &ps);
		return 0;
	}
	
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void Init(HINSTANCE hInstance)
{
	classEx.hInstance = hInstance;
	classEx.hCursor = LoadCursorW((HINSTANCE)0, IDC_ARROW);
	RegisterClassExW(&classEx);

	hMainWindow = CreateWindowExW(0, MAIN_CLAS_NAME, CAPTION, WINDOW_STYLE,
		0, 0, 1600, 900, (HWND)0, (HMENU)0, (HINSTANCE)0, (LPVOID)0);
	GetProcessesList();
	SetTimer(hMainWindow, (UINT_PTR)NULL, 1000 / UPDATE_RATE, (TIMERPROC)NULL);
	
	hStaticFont = CreateFontW(18, 10, 0, 0, FW_NORMAL, 0, 0, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, STATIC_FONT_NAME);

	ShowWindow(hMainWindow, SW_SHOWNORMAL);
	UpdateWindow(hMainWindow);

	hProcessNameEdit = CreateWindowExW(0, EDIT_CLASS_NAME, PROCESS_NAME_EDIT_TEXT, PROCESS_NAME_EDIT_STYLE, buttonWidth * 3, 0, buttonWidth, 50, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
	hLibraryNameEdit = CreateWindowExW(0, EDIT_CLASS_NAME, LIBRARY_NAME_EDIT_TEXT, LIBRARY_NAME_EDIT_STYLE, buttonWidth * 3, 50, buttonWidth, 50, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
	hFunctionNameEdit = CreateWindowExW(0, EDIT_CLASS_NAME, FUNCTION_NAME_EDIT_TEXT, FUNCTION_NAME_EDIT_STYLE, buttonWidth * 3, 100, buttonWidth, 50, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
	hInjectButton = CreateWindowExW(0, BUTTON_CLASS_NAME, INJECT_BUTTON_TEXT, INJECT_BUTTON_STYLE, buttonWidth * 3, 150, buttonWidth, buttonWidth, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
	
	processName = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t));
	libraryName = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t));
	functionName = (char *)calloc(MAX_PATH, sizeof(char));

	hKernel32 = GetModuleHandleW(KERNEL32_STRING);
}

void Release()
{
	int i;

	hStaticLeft = (HWND)0;
	hStaticRight = (HWND)0;

	FreeLibrary(hKernel32);

	DeleteObject(hStaticFont);
	
	free(processIds);
	processIds = (DWORD *)NULL;
	free(processIdsOld);
	processIdsOld = (DWORD *)NULL;

	for (i = 0; i < MAX_PROCESSES_COUNT; i++) {
		if (processes[i]) {
			CloseHandle(processes[i]);
			processes[i] = (HANDLE)0;
			buttons[i] = (HWND)0;
			free(processModules[i]);
			processModules[i] = (HMODULE *)NULL;
			free(processModulesInfo[i]);
			processModulesInfo[i] = (MODULEINFO *)NULL;
			free(processNames[i]);
			processNames[i] = (wchar_t *)NULL;
		} else {
			break;
		}
	}
}

void GetProcessesList()
{
	int i, j, count, index, modulesCount;
	DWORD arraySize, cbNeeded;
	BOOL arraysEquals;
	HANDLE hProcess;
	HWND hBtn;

	arraySize = (DWORD)PROCESSES_BUFFER_SIZE;
	arraysEquals = TRUE;
	
	if (!processIds) {
		if (!(processIds = (DWORD *)calloc(PROCESSES_BUFFER_SIZE, sizeof(DWORD)))) {
			return;
		}
	}
	if (!processIdsOld) {
		if (!(processIdsOld = (DWORD *)calloc(PROCESSES_BUFFER_SIZE, sizeof(DWORD)))) {
			return;
		}
	}
	if (processIds) {
		if (EnumProcesses (processIds, arraySize, &cbNeeded)) {
			count = cbNeeded / sizeof(DWORD);
			if ((count == processesCount) && processesCount) {
				for (i = 0; i < count; i++) {
					if (processIds[i] != processIdsOld[i]) {
						arraysEquals = FALSE;
						break;
					}
				}
			} else {
				arraysEquals = FALSE;
			}
			if (!arraysEquals) {
				processesCount = count;
				for (i = 0; i < processesCount; i++) {
					processIdsOld[i] = processIds[i];
				}
				if (processes[0]) {
					for (i = 0; i < MAX_PROCESSES_COUNT; i++) {
						if (processes[i]) {
							CloseHandle(processes[i]);
						}
						if (buttons[i]) {
							DestroyWindow(buttons[i]);
						}
						processes[i] = (HANDLE)0;
						buttons[i] = (HWND)0;
						hSelectedProcess = (HANDLE)0;
					}
				}
				for (i = 0, index = 0; i < count; i++) {
					if (hProcess = OpenProcess(PROCESS_ACCESS_FLAGS, 0, processIds[i])) {
						if (processModules[index]) {
							free(processModules[index]);
							processModules[index] = (HMODULE *)NULL;
						}
						if (processModulesInfo[index]) {
							free(processModulesInfo[index]);
							processModulesInfo[index] = (MODULEINFO *)NULL;
						}
						if (processModules[index] = (HMODULE *)calloc(MAX_MODULES_COUNT, sizeof(HMODULE))) {
							if (EnumProcessModules(hProcess, processModules[index], MAX_MODULES_COUNT * sizeof(HMODULE), &cbNeeded)) {
								modulesCount = cbNeeded / sizeof(HMODULE);
								if (processNames[index]) {
									free(processNames[index]);
									processNames[index] = (wchar_t *)NULL;
								}
								if (processNames[index] = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t))) {
									if (GetModuleBaseNameW(hProcess, processModules[index][0], processNames[index], MAX_PATH)) {
										if (processModulesInfo[index] = (MODULEINFO *)calloc(modulesCount, sizeof(MODULEINFO))) {
											for (j = 0; j < modulesCount; j++) {
												GetModuleInformation(hProcess, processModules[index][j], &processModulesInfo[index][j], sizeof(MODULEINFO));
											}
										}
										processes[index] = hProcess;
										index++;
										if (index == MAX_PROCESSES_COUNT) {
											break;
										}
									}
								}
							}
						} else {
							return;
						}
					}
				}
				buttonWidth = clientRect.right / 4;
				buttonHeight = clientRect.bottom / index;
				for (i = 0; i < index; i++) {
					if (hBtn = CreateWindowExW(0, BUTTON_CLASS_NAME, processNames[i], PROCESS_BUTTON_STYLE, 0, buttonHeight * i, buttonWidth, buttonHeight, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL)) {
						buttons[i] = hBtn;
					}
				}
				openedProcessesCount = index;
			}
		}
	}
}

void ShowInfo(int index)
{
	int half, quarter, bottom, length;
	wchar_t *processInfo;

	half = clientRect.right / 2;
	quarter = clientRect.right / 4;
	bottom = clientRect.bottom;
	if (!hStaticRight) {
		hStaticRight = CreateWindowExW(0, STATIC_CLASS_NAME, (wchar_t *)NULL, STATIC_STYLE, half, 0, quarter, bottom, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
		SendMessageW(hStaticRight, WM_SETFONT, (WPARAM)hStaticFont, (LPARAM)TRUE);
	}
	if (processInfo = (wchar_t *)calloc(PROCESS_INFO_BUFFER_LENGTH, sizeof(wchar_t))) {
		GetProcessModulesInfo(index, processInfo);
		if (!hStaticLeft) {
			hStaticLeft = CreateWindowExW(0, STATIC_CLASS_NAME, processInfo, STATIC_STYLE, quarter, 0, quarter, bottom, hMainWindow, (HMENU)0, (HINSTANCE)0, (LPVOID)NULL);
			SendMessageW(hStaticLeft, WM_SETFONT, (WPARAM)hStaticFont, (LPARAM)TRUE);
		} else {
			SetWindowTextW(hStaticLeft, processInfo);
		}
		length = wcslen(processInfo) / 2;
		while(processInfo[length++] != L'\n');
		length--;
		processInfo[length] = (wchar_t)0;
		SetWindowTextW(hStaticLeft, processInfo);
		processInfo[length] = L'\n';
		SetWindowTextW(hStaticRight, processInfo + length + 1);
		free(processInfo);
		processInfo = (wchar_t *)NULL;
	}
}

void MoveWindows()
{
	int i, width;

	for (i = 0; i < openedProcessesCount; i++) {
		MoveWindow(buttons[i], 0, i * buttonHeight, buttonWidth, buttonHeight, TRUE);
	}
	MoveWindow(hStaticLeft, buttonWidth, 0, buttonWidth, clientRect.bottom, TRUE);
	MoveWindow(hStaticRight, buttonWidth * 2, 0, buttonWidth, clientRect.bottom, TRUE);
	width = clientRect.right / 4;
	MoveWindow(hProcessNameEdit, width * 3, 0, width, 50, TRUE);
	MoveWindow(hLibraryNameEdit, width * 3, 50, width, 50, TRUE);
	MoveWindow(hFunctionNameEdit, width * 3, 100, width, 50, TRUE);
	MoveWindow(hInjectButton, width * 3, 150, width, width, TRUE);
}

void GetProcessModulesInfo(int index, wchar_t *result)
{
	int i;
	wchar_t *moduleName, *moduleSize, moduleNumber[] = L"000";

	Append(result, L"Process information\nProcess name: ");
	Append(result, processNames[index]);
	Append(result, L"\nModules:\n");

	for (i = 0; i < MAX_MODULES_COUNT; i++) {
		if (processModules[index][i]) {
			Append(result, moduleNumber);
			Append(result, L": ");
			if (moduleName = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t))) {
				GetModuleBaseNameW(processes[index], processModules[index][i], moduleName, MAX_PATH);
				Append(result, moduleName);
				if (moduleSize = (wchar_t *)calloc(11, sizeof(wchar_t))) {
					GetStringFromInt(processModulesInfo[index][i].SizeOfImage, moduleSize);
					Append(result, L" - ");
					Append(result, moduleSize);
					free(moduleSize);
					moduleSize = (wchar_t *)NULL;
				}
				Append(result, L" bytes\n");
				free(moduleName);
				moduleName = (wchar_t *)NULL;
			}
		} else {
			break;
		}
		if (moduleNumber[2] == L'9') {
			if (moduleNumber[1] == L'9') {
				moduleNumber[0]++;
				moduleNumber[1] = L'0';
			} else {
				moduleNumber[1]++;
			}
			moduleNumber[2] = L'0';
		} else {
			moduleNumber[2]++;
		}
	}
}

HANDLE GetProcessByName(wchar_t *processName)
{
	int count, i, index;
	wchar_t *openedProcessName;
	DWORD cbNeeded, *processIds;
	HANDLE hOpenedProcess;
	HMODULE processMainModule;

	if (!(processIds = (DWORD *)calloc(PROCESSES_BUFFER_SIZE, sizeof(DWORD)))) {
		return (HANDLE)0;
	}
	if (!EnumProcesses(processIds, PROCESSES_BUFFER_SIZE, &cbNeeded)) {
		free(processIds);
		processIds = (DWORD *)NULL;
		return (HANDLE)0;
	}
	count = cbNeeded / sizeof(DWORD);
	if (openedProcessName = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t))) {
		for (i = 0, index = 0; i < count; i++) {
			if (hOpenedProcess = OpenProcess(PROCESS_ACCESS_FLAGS, FALSE, processIds[i])) {
				if (EnumProcessModules(hOpenedProcess, &processMainModule, sizeof(HMODULE), &cbNeeded)) {
					wmemset(openedProcessName, (wchar_t)0, MAX_PATH);
					if (GetModuleBaseNameW(hOpenedProcess, processMainModule, openedProcessName, MAX_PATH)) {
						if (!wcscmp(processName, openedProcessName)) {
							free(openedProcessName);
							openedProcessName = (wchar_t *)NULL;
							free(processIds);
							processIds = (DWORD *)NULL;
							return hOpenedProcess;
						}
					}
				}
			}
		}
	} else {
		free(processIds);
		processIds = (DWORD *)NULL;
		return (HANDLE)0;
	}
	free(openedProcessName);
	openedProcessName = (wchar_t *)NULL;
	free(processIds);
	processIds = (DWORD *)NULL;

	return (HANDLE)0;
}

BOOL Inject(wchar_t *fileName, wchar_t *processName, char *functionName)
{
	DWORD remoteThreadExitCode;
	HANDLE hRemoteProcess, hLibraryFile, hRemoteThread;
	HMODULE hTempLibrary;
	void *functionAddress, *functionOffset, *fileNameRemoteAddress;

	hLibraryFile = CreateFileW(fileName, FILE_ACCESS_ATTRUBUTES, 0, (SECURITY_ATTRIBUTES *)NULL, OPEN_EXISTING, 0, (HANDLE)NULL);
	if (GetLastError()) {
		Error(WRONG_LIBRARY_NAME_STRING, INCORRECT_INPUT_STRING);
		return FALSE;
	}
	CloseHandle(hLibraryFile);
	if (!(hTempLibrary = LoadLibraryW(fileName))) {
		Error(LIBRARY_ANALYZE_FAIL_STRING, ERROR_STRING);
		return FALSE;
	}
	if (!(functionOffset = (void *)GetProcAddress(hTempLibrary, functionName))) {
		Error(WRONG_FUNCTION_NAME_STRING, INCORRECT_INPUT_STRING);
		return FALSE;
	}
	functionOffset = (void *)((long)functionOffset - (long)hTempLibrary);
	FreeLibrary(hTempLibrary);
	hTempLibrary = (HMODULE)0;
	if (!(hRemoteProcess = GetProcessByName(processName))) {
		Error(WRONG_PROCESS_NAME_STRING, INCORRECT_INPUT_STRING);
		return FALSE;
	}
	if(!(fileNameRemoteAddress = VirtualAllocEx(hRemoteProcess, (void *)NULL, wcslen(fileName) * sizeof(wchar_t), MEM_COMMIT, PROTECT_FLAGS))) {
		Error(FAIL_ALLOCATIONG_MEMORY_STRING, ERROR_STRING);
		return FALSE;
	}
	if (!(WriteProcessMemory(hRemoteProcess, fileNameRemoteAddress, fileName, wcslen(fileName) * sizeof(wchar_t), (SIZE_T *)NULL))) {
		Error(FAIL_COPYING_MEMORY_STRING, ERROR_STRING);
		return FALSE;
	}
	functionAddress = GetProcAddress(hKernel32, LOAD_LIBRARY_STRING);
	if (!(hRemoteThread = CreateRemoteThread(hRemoteProcess, (SECURITY_ATTRIBUTES *)NULL, REMOTE_THREAD_STACK_SIZE, (LPTHREAD_START_ROUTINE)functionAddress, fileNameRemoteAddress, 0, (DWORD *)NULL))) {
		Error(LIBRARY_LOADING_FAIL_STRING, ERROR_STRING);
		return FALSE;
	}
	WaitForSingleObject(hRemoteThread, INFINITE);
	GetExitCodeThread(hRemoteThread, &remoteThreadExitCode);
	CloseHandle(hRemoteThread);
	functionAddress = (void *)((long)remoteThreadExitCode + (long)functionOffset);
	if (!(hRemoteProcess = CreateRemoteThread(hRemoteProcess, (SECURITY_ATTRIBUTES *)NULL, REMOTE_THREAD_STACK_SIZE, (LPTHREAD_START_ROUTINE)functionAddress, (void *)NULL, 0, (DWORD *)NULL))) {
		Error(FAIL_STARTING_REMOTE_PROCEDURE_STRING, INCORRECT_INPUT_STRING);
		return FALSE;
	}
	WaitForSingleObject(hRemoteThread, INFINITE);
	CloseHandle(hRemoteThread);
	Info(SUCCESS_STRING, SUCCESS_STRING);

	return TRUE;
}

void Error(const wchar_t *text, const wchar_t *caption)
{
	MessageBoxW(hMainWindow, text, caption, MB_ICONERROR);
}

void Info(const wchar_t *text, const wchar_t *caption)
{
	MessageBoxW(hMainWindow, text, caption, MB_ICONINFORMATION);
}

void GetStringFromInt(int value, wchar_t *result)
{
	int index;

	for (index = 0; index < 10; index++) {
		result[index] = L'0';
	}
	index = 9;
	do {
		result[index--] = L'0' + value % 10;
		value = value / 10;
	} while (value);
	while (index >= 0) {
		result[index--] = L' ';
	}
}