#pragma once
//#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>
//#include <strsafe.h>
//#include <conio.h>
#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define INSTANCES 4
#define PIPE_TIMEOUT 5000
#define BUFSIZE 1000
typedef struct tagPIPEINST {
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, * LPPIPEINST;
using func = void(*)(HANDLE hPipe, LPVOID lpBinary, DWORD dwSize);

void DisconnectAndReconnect(DWORD);
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);
DWORD WINAPI pipe_start(LPVOID lpParam);
void pipeStart();
void pipeStop();
void pipeSetRead(func fn);
