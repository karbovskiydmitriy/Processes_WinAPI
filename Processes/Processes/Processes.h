#include <Windows.h>
#include <Psapi.h>
#include <wchar.h>

#define MAX_PROCESSES_COUNT 256
#define MAX_MODULES_COUNT 256
#define PROCESSES_BUFFER_SIZE 4096
#define PROCESS_INFO_BUFFER_LENGTH 16384
#define UPDATE_RATE 1

#define Append(destination, source) wcscat_s ((wchar_t *)(destination), (int)(BUFFER_SIZE), (wchar_t *)(source));

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WindowProc(HWND, UINT, WPARAM, LPARAM);
void Init(HINSTANCE);
void Release();
void GetProcessesList();
void ShowInfo(int);
void MoveWindows();
void GetProcessModulesInfo(int, wchar_t *);
HANDLE GetProcessByName(wchar_t *);
BOOL Inject(wchar_t *, wchar_t *, char *);
void Error(const wchar_t *, const wchar_t *);
void Info(const wchar_t *, const wchar_t *);
void GetStringFromInt(int, wchar_t *);
