#pragma once
//#include <Windows.h>
//#include <userenv.h>
//#include <wtsapi32.h>
//#include <tchar.h>
//#include <sddl.h>
//#include <processthreadsapi.h>
//
//#include <shellapi.h>
//#pragma comment(lib, "Shlwapi.lib")
//
//#pragma comment(lib, "Wtsapi32.lib")
//#pragma comment(lib, "Userenv.lib")

DWORD MyStartService(LPCTSTR _ServiceName);
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD CtrlCode);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
HRESULT RunAsInteractiveUser(const TCHAR* lpszProcessName, TCHAR* lpszCommendLine);

#pragma pack(push, 1)
typedef struct tagSlotData {
	char szFileName[MAX_PATH];
	char szRetunSlot[50];
} SlotData;

typedef struct tagSlotRecv {
	DWORD dwError;
} SlotRecv;

#pragma pack(pop)