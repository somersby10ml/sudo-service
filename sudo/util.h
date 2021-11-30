#pragma once
#include "pch.h"

BOOL IsElevated();
tstring GetErrorName(DWORD dwErrorCode);
void MyOutputDebugString(LPCTSTR pszStr, ...);
void MyOutputDebugStringA(LPCSTR pszStr, ...);
std::wstring string_to_unicode(std::string multiByte);
std::string unicode_to_string(std::wstring unicode);
tstring GetError(DWORD dwErrorCode);
HRESULT CreateLink(tstring lpszPathObj, std::wstring lpszPathLink, tstring lpszDesc);
void EnableSystemPriv(void);