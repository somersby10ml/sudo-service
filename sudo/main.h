#pragma once
DWORD SendPipe(tstring path);

using ArgumentFunc = DWORD(*)();
typedef struct tagArgument {
	std::vector<const TCHAR*> commands;
	ArgumentFunc fun;
} Argument;


