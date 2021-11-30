#include "pch.h"
#include "main.h"
#include "console.h"
#include "util.h"
#include "service_main.h"

void ShowHelpMessage() {
	tcout << BOLDGREEN << _T("Sudo Service v0.1 for windows") << RESET << std::endl;
	tcout << _T("  This will run the program with administrator privileges.") << RESET << std::endl;
	tcout << BOLDRED << _T("    The UAC window does not appear.") << RESET << std::endl;
	tcout << BOLDRED << _T("    Therefore, only run programs that are 100% trusted.") << RESET << std::endl;
	tcout << _T("    Please refer to github for install/uninstall/update.") << RESET << std::endl << std::endl;
	tcout << BOLDYELLOW << _T("Using") << RESET << _T(": sudo [-HV] executable path") << RESET << std::endl;
	tcout << BOLDGREEN << _T("-H, --help, /? : help message") << RESET << std::endl;
	tcout << BOLDGREEN << _T("-V, --version : show version") << RESET << std::endl;
	tcout << BOLDGREEN << _T("-S, --status : show status") << RESET << std::endl;
	tcout << _T("github) icon: https://github.com/somersby10ml/sudo_service") << RESET << std::endl;
	tcout << _T("License Notice) icon: https://findicons.com/icon/590712/uac") << RESET << std::endl;
}

int _tmain(int argc, TCHAR* argv[]) {
	std::vector<Argument> helper_messages;

	// if Session==0 call ServiceMain()
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwSessionID = 0;
	ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionID);
	if (dwSessionID == 0) {
		if (!MyStartService(SERVICE_NAME)) {
			//errorName = _T("StartServiceCtrlDispatcher");
			return GetLastError();
		}
		return ERROR_SUCCESS;
	}


	// using linux sequence color
	DWORD dwMode = 0;
	GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode);

	_tsetlocale(LC_ALL, _T(""));
	if (!(argc == 2)) {
		ShowHelpMessage();
		return ERROR_SUCCESS;
	}

	helper_messages.push_back({ { _T("-H"), _T("--help"), _T("/?") }, []()->DWORD {
		ShowHelpMessage();
		return ERROR_SUCCESS;
	} });

	helper_messages.push_back({ { _T("-V"), _T("--version") }, []()->DWORD {
		tcout << "v0.1" << std::endl;
		return ERROR_SUCCESS;
	} });

	helper_messages.push_back({ { _T("-S"), _T("--status") }, []()->DWORD {
		DWORD dwErrorCode = ERROR_SUCCESS;
		HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			dwErrorCode = GetLastError();
			tcout << _T("The service cannot be accessed. Reinstall the program.") << std::endl;
			return dwErrorCode;
		}
		if (!WaitNamedPipe(PIPE_NAME, 20000)) {
			dwErrorCode = GetLastError();
			tcout << _T("The service is not responding.") << std::endl;
			CloseHandle(hPipe);
			return dwErrorCode;
		}
		tcout << _T("The service status is OK.") << std::endl;
		CloseHandle(hPipe);
		return ERROR_SUCCESS;
	} });


	for (std::vector<Argument>::iterator it = helper_messages.begin(); it != helper_messages.end(); it++) {
		for (std::vector<const TCHAR*>::iterator iter = it->commands.begin(); iter != it->commands.end(); iter++) {
			if (_tcscmp(argv[1], *iter) == 0) {
				return it->fun();
			}
		}
	}

	// Send a command to execute a file as a pipe.
	dwErrorCode = SendPipe(argv[1]);
	return dwErrorCode;
}

// run command
DWORD SendPipe(tstring path) {
	DWORD dwErrorCode = ERROR_SUCCESS;
	HANDLE hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		dwErrorCode = GetLastError();
		tcout << _T("The service cannot be accessed. Reinstall the program.") << std::endl;
		return dwErrorCode;
	}

	if (!WaitNamedPipe(PIPE_NAME, 20000)) {
		dwErrorCode = GetLastError();
		tcout << _T("The service is not responding.") << std::endl;
		CloseHandle(hPipe);
		return dwErrorCode;
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	DWORD fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	if (!fSuccess) {
		dwErrorCode = GetLastError();
		tcout << _T("SetNamedPipeHandleState failed") << std::endl;
		tcout << _T("Error Message: ") << GetError(dwErrorCode) << std::endl;
		CloseHandle(hPipe);
		return dwErrorCode;
	}

	DWORD cbWritten;
	if (!WriteFile(hPipe, path.c_str(), (DWORD)path.size(), &cbWritten, NULL)) {
		dwErrorCode = GetLastError();
		tcout << _T("Failed to send to service.") << std::endl;
		tcout << _T("Error Message: ") << GetError(dwErrorCode) << std::endl;
		CloseHandle(hPipe);
		return dwErrorCode;
	}

	CloseHandle(hPipe);
	return ERROR_SUCCESS;
}