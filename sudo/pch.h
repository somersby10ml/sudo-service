#pragma once

#ifndef PCH_H
#define PCH_H

#define _CRT_STDIO_LEGACY_WIDE_SPECIFIERS 1

#define APP_NAME _T("sudo")
#define APP_VERSION _T("0.1")
#define SERVICE_NAME _T("sudo")
#define SERVICE_DISPLAYNAME _T("sudo service")
#define PIPE_NAME _T("\\\\.\\pipe\\sudo_service_pipe")

// standard
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

// STL
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

// WinAPI
#include <Windows.h>
#include <Shlwapi.h>
#include <userenv.h>
#include <wtsapi32.h>
#include <shellapi.h>

// link
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")

typedef std::basic_string<TCHAR> tstring;

#if defined(UNICODE) || defined(_UNICODE)
#define __UNICODE__
#endif

#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
#else
#define tcout std::cout
#endif

#endif //PCH_H