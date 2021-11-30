#include "pch.h"
#include "util.h"

//#include <winnls.h>
//#include <shobjidl.h>
//#include <objbase.h>
//#include <objidl.h>
//#include <shlguid.h>

BOOL IsElevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation = { 0, };
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}
void EnableSystemPriv(void) {
	HANDLE  hToken;
	LUID   luidDebug;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug)) {  // 디버그 권한..............
		CloseHandle(hToken);
		return;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luidDebug;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
		printf("Adjust error: %d", GetLastError());
		CloseHandle(hToken);
	}
	CloseHandle(hToken);
}

void MyOutputDebugString(LPCTSTR pszStr, ...) {
#ifdef _DEBUG
	TCHAR szMsg[2000];
	va_list args;
	va_start(args, pszStr);
	_vstprintf_s(szMsg, _countof(szMsg), pszStr, args);
	OutputDebugString(szMsg);
#endif
}

void MyOutputDebugStringA(LPCSTR pszStr, ...) {
	char szMsg[2000];
	va_list args;
	va_start(args, pszStr);
	vsprintf_s(szMsg, _countof(szMsg), pszStr, args);
	OutputDebugStringA(szMsg);
}

tstring GetErrorName(DWORD dwErrorCode) {
	tstring error_string;
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	error_string = (LPTSTR)lpMsgBuf;
	LocalFree(lpMsgBuf);
	error_string = error_string.substr(0, error_string.find_last_of(_T("\r")));
	return error_string;
}

tstring GetError(DWORD dwErrorCode) {
	tstring err = GetErrorName(dwErrorCode).c_str();
	switch (dwErrorCode) {
		case ERROR_ACCESS_DENIED:
			err += _T("\n\tPlease run it with administrator privileges.");
			break;
	}
	return err;
}
std::wstring string_to_unicode(std::string multiByte) {
	std::wstring strUnicode;
	int nLen = MultiByteToWideChar(CP_ACP, 0, multiByte.c_str(), multiByte.size(), NULL, NULL);
	strUnicode.resize(nLen);
	MultiByteToWideChar(CP_ACP, 0, multiByte.c_str(), multiByte.size(), (LPWSTR)strUnicode.data(), strUnicode.size());
	return strUnicode;
}

std::string unicode_to_string(std::wstring unicode) {
	std::string strMultiBytes;
	int nLen = WideCharToMultiByte(CP_ACP, 0, unicode.c_str(), unicode.size(), nullptr, 0, nullptr, nullptr);
	strMultiBytes.resize(nLen);
	WideCharToMultiByte(CP_ACP, 0, unicode.c_str(), unicode.size(), (LPSTR)strMultiBytes.data(), strMultiBytes.size(), nullptr, nullptr);
	return strMultiBytes;
}