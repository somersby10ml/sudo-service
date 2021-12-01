#include "pch.h"
#include "service_main.h"
#include "util.h"
#include "pipe.h"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

LPCTSTR ServiceName;

DWORD MyStartService(LPCTSTR _ServiceName) {
	ServiceName = _ServiceName;
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPTSTR)ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};
	return StartServiceCtrlDispatcher(ServiceTable);
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
		case SERVICE_CONTROL_STOP:
			if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
				break;

			g_ServiceStatus.dwControlsAccepted = 0;
			g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			g_ServiceStatus.dwWin32ExitCode = 0;
			g_ServiceStatus.dwCheckPoint = 4;
			SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
			SetEvent(g_ServiceStopEvent);
			break;

		default:
			break;
	}
}

HRESULT RunAsInteractiveUser(const TCHAR* lpszProcessName, TCHAR* lpszCommendLine) {
	BOOL bRet;
	HRESULT hr;
	HANDLE processToken = NULL;
	TOKEN_PRIVILEGES oldTokenPrivileges = { 0 };
	HANDLE impersonationToken = NULL;
	HANDLE userToken = NULL;
	LPVOID pEnvironment = NULL;
	PROCESS_INFORMATION processInformation = { 0 };
	TOKEN_LINKED_TOKEN linkedToken = { 0 };
	__try {
		bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &processToken);
		if (!bRet) {
			MyOutputDebugString(_T("OpenProcessToken=%d, GetLastError=%d"), bRet, GetLastError());
			hr = GetLastError();
			__leave;
		}

		LUID luid;
		bRet = LookupPrivilegeValue(NULL, _T("SeTcbPrivilege"), &luid);
		if (!bRet) {
			MyOutputDebugString(_T("LookupPrivilegeValue=%d, GetLastError=%d"), bRet, GetLastError());

			hr = GetLastError();
			__leave;
		}

		TOKEN_PRIVILEGES adjTokenPrivileges = { 0 };
		adjTokenPrivileges.PrivilegeCount = 1;
		adjTokenPrivileges.Privileges[0].Luid = luid;
		adjTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		DWORD dwOldTPLen;
		bRet = AdjustTokenPrivileges(processToken, FALSE, &adjTokenPrivileges, sizeof(TOKEN_PRIVILEGES), &oldTokenPrivileges, &dwOldTPLen);
		if (bRet) {
			hr = GetLastError();
			if (hr == ERROR_SUCCESS);

			else if (hr == ERROR_NOT_ALL_ASSIGNED) {
				// Enabled by default
			}
		}
		else {
			hr = GetLastError();
			MyOutputDebugString(_T("AdjustTokenPrivileges=%d, GetLastError=%d"), bRet, GetLastError());
			__leave;
		}

		HANDLE hToken = GetCurrentProcessToken();
		DWORD conSessId = WTSGetActiveConsoleSessionId();
		if (conSessId == 0xFFFFFFFF) {
			MyOutputDebugString(_T("WTSGetActiveConsoleSessionId=%d, GetLastError=%d"), conSessId, GetLastError());
			__leave;
		}


		bRet = WTSQueryUserToken(conSessId, &impersonationToken);
		MyOutputDebugStringA("WTSQueryUserToken:%d GetLastError: %d", bRet, GetLastError());
		MyOutputDebugStringA("impersonationToken:%X", bRet, impersonationToken);
		if (!bRet) {
			__leave;
		}

		TOKEN_ELEVATION_TYPE eType;
		DWORD dwSize = sizeof eType;
		bRet = GetTokenInformation(impersonationToken, TokenElevationType, &eType, dwSize, &dwSize);
		MyOutputDebugStringA("GetTokenInformation(TokenElevationType):%d GetLastError: %d", bRet, GetLastError());
		MyOutputDebugStringA("eType: %d", eType);


		// ERROR_NO_SUCH_LOGON_SESSION 1312
		dwSize = sizeof linkedToken;
		bRet = GetTokenInformation(impersonationToken, TokenLinkedToken, &linkedToken, dwSize, &dwSize);
		if (bRet) {
			MyOutputDebugStringA("GetTokenInformation:%d GetLastError: %d", bRet, GetLastError());

			bRet = DuplicateTokenEx(linkedToken.LinkedToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenImpersonation, &userToken);
			if (!bRet) {
				MyOutputDebugStringA("DuplicateTokenEx:%d GetLastError: %d", bRet, GetLastError());
				//__leave;
			}
			//__leave;
		}

		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);
		si.lpDesktop = (LPTSTR)_T("winsta0\\default");
		bRet = CreateEnvironmentBlock(&pEnvironment, userToken, TRUE);
		if (!bRet) {
			MyOutputDebugString(_T("CreateEnvironmentBlock=%d, GetLastError=%d"), conSessId, GetLastError());
			hr = GetLastError();
			__leave;
		}



		if (userToken) {
			bRet = CreateProcessAsUser(userToken, lpszProcessName, lpszCommendLine, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, pEnvironment, NULL, &si, &processInformation);
		}
		else {
			bRet = CreateProcessAsUser(impersonationToken, lpszProcessName, lpszCommendLine, NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, pEnvironment, NULL, &si, &processInformation);
		}

		MyOutputDebugString(_T("CreateProcessAsUser=%d, GetLastError=%d"), bRet, GetLastError());

		if (!bRet) {
			hr = GetLastError();
			__leave;
		}

		else {
			/*bRet = WaitForSingleObject(processInformation.hProcess, INFINITE);
			DWORD retVal = NULL;
			GetExitCodeProcess(processInformation.hProcess, &retVal);
			if (1 == (int)retVal) {
				__leave;
			}*/
			__leave;
		}
	}

	__finally {
		if (processInformation.hThread) {
			CloseHandle(processInformation.hThread);
		}

		if (processInformation.hProcess) {
			CloseHandle(processInformation.hProcess);
		}

		if (pEnvironment) {
			bRet = DestroyEnvironmentBlock(pEnvironment);
		}

		if (userToken) {
			CloseHandle(userToken);
		}

		if (impersonationToken) {
			CloseHandle(impersonationToken);
		}

		if (linkedToken.LinkedToken) {
			CloseHandle(linkedToken.LinkedToken);
		}

		if (processToken) {
			bRet = AdjustTokenPrivileges(processToken, FALSE, &oldTokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

			CloseHandle(processToken);
		}
	}
	return ERROR_SUCCESS;
}

void readData(HANDLE hPipe, LPVOID lpBinary, DWORD dwSize) {
	LPCSTR pFilePath = (PCHAR)lpBinary;
	try {
		if (!pFilePath)
			throw ERROR_INVALID_PARAMETER;


		//tstring CurrentDirectory = GetBaseName((TCHAR*)lpBinary);
		//MyOutputDebugString(_T("CurrentDirectory: %ws"), CurrentDirectory.c_str());
		DWORD h = (DWORD)RunAsInteractiveUser(nullptr, (TCHAR*)lpBinary);
	}
	catch (DWORD e) {
		e;
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {

	pipeStart();
	pipeSetRead(readData);

	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) {
		Sleep(500);
	}

	pipeStop();
	return ERROR_SUCCESS;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
	DWORD Status = E_FAIL;
	HANDLE hThread;

	// Register our service control handler with the SCM
	g_StatusHandle = RegisterServiceCtrlHandler(ServiceName, ServiceCtrlHandler);
	if (g_StatusHandle == NULL)
		goto exit;

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Create a service stop event to wait on later
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL) {
		// Error creating event
		// Tell service controller we are stopped and exit
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		goto exit;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Start a thread that will perform the main task of the service
	hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
	if (hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
exit:
	return;
}