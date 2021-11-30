#include "pch.h"
#include "pipe.h"
#include "util.h"
PIPEINST Pipe[INSTANCES] = { 0, };
HANDLE hEvents[INSTANCES] = { 0, };
HANDLE hThread = nullptr;
bool isStart = true;

func ReadData;
func WriteData;

void pipeStop() {
	isStart = false;
	SetEvent(hEvents[0]);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}
void pipeStart() {
	[[maybe_unused]] DWORD ThreadID;
	hThread = CreateThread(NULL, 0, pipe_start, NULL, 0, NULL);
}

void pipeSetRead(func fn) {
	ReadData = fn;
}

DWORD WINAPI pipe_start(LPVOID lpParam) {
	DWORD i, dwWait, cbRet, dwErr;
	BOOL fSuccess;
	LPCTSTR lpszPipename = PIPE_NAME;

	// free right 
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, true, NULL, false);

	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = true;

	for (i = 0; i < INSTANCES; i++) {

		// Create an event object for this instance.
		hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

		if (hEvents[i] == NULL) {
			// todo
			return 0;
		}

		Pipe[i].oOverlap.hEvent = hEvents[i];

		Pipe[i].hPipeInst = CreateNamedPipe(
			lpszPipename,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			INSTANCES, BUFSIZE, BUFSIZE, PIPE_TIMEOUT, &sa);

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
			// todo
			return 0;
		}

		Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
		Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
	}

	while (1) {
		if (!isStart) break;
		dwWait = WaitForMultipleObjects(INSTANCES, hEvents, FALSE, INFINITE);
		if (!isStart) break;

		i = dwWait - WAIT_OBJECT_0;
		if (i < 0 || i >(INSTANCES - 1)) {
			// todo range out
			return 0;
		}

		if (Pipe[i].fPendingIO) {
			fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap, &cbRet, FALSE);

			switch (Pipe[i].dwState) {
				case CONNECTING_STATE:
					if (!fSuccess) {
						// todo
						MyOutputDebugString(_T("[pipe] Error %d."), GetLastError());
						printf("Error %d.\n", GetLastError());
						return 0;
					}
					Pipe[i].dwState = READING_STATE;
					break;

					// Pending read operation
				case READING_STATE:
					if (!fSuccess || cbRet == 0) {
						DisconnectAndReconnect(i);
						continue;
					}
					Pipe[i].cbRead = cbRet;
					Pipe[i].dwState = WRITING_STATE;
					break;

					// Pending write operation
				case WRITING_STATE:
					if (!fSuccess || cbRet != Pipe[i].cbToWrite) {
						DisconnectAndReconnect(i);
						continue;
					}
					Pipe[i].dwState = READING_STATE;
					break;

				default:
					printf("Invalid pipe state.\n");
					return 0;

			}
		}

		// The pipe state determines which operation to do next.

		switch (Pipe[i].dwState) {
			// READING_STATE:
			// The pipe instance is connected to the client
			// and is ready to read a request from the client.

			case READING_STATE:
				fSuccess = ReadFile(Pipe[i].hPipeInst, Pipe[i].chRequest, BUFSIZE, &Pipe[i].cbRead, &Pipe[i].oOverlap);

				// The read operation completed successfully.
				if (fSuccess && Pipe[i].cbRead != 0) {
					Pipe[i].fPendingIO = FALSE;
					Pipe[i].dwState = WRITING_STATE;
					continue;
				}

				dwErr = GetLastError();
				if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
					Pipe[i].fPendingIO = TRUE;
					continue;
				}

				DisconnectAndReconnect(i);
				break;

			case WRITING_STATE:
				//GetAnswerToRequest(&Pipe[i]);
				//ReadData(Pipe[i].chReply, Pipe[i].cbToWrite);
				ReadData(Pipe[i].hPipeInst, Pipe[i].chRequest, Pipe[i].cbRead);
				Pipe[i].fPendingIO = FALSE;
				Pipe[i].dwState = READING_STATE;
				continue;

				fSuccess = WriteFile(Pipe[i].hPipeInst, Pipe[i].chReply, Pipe[i].cbToWrite, &cbRet, &Pipe[i].oOverlap);
				if (fSuccess && cbRet == Pipe[i].cbToWrite) {
					Pipe[i].fPendingIO = FALSE;
					Pipe[i].dwState = READING_STATE;
					continue;
				}

				dwErr = GetLastError();
				if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
					Pipe[i].fPendingIO = TRUE;
					continue;
				}

				DisconnectAndReconnect(i);
				break;

			default:
				// todo
				// printf("Invalid pipe state.\n");
				return 0;

		}
	}

	for (int i = 0; i < INSTANCES; i++) {
		if (hEvents[i]) CloseHandle(hEvents[i]);
		if (Pipe[i].hPipeInst) CloseHandle(Pipe[i].hPipeInst);
	}
	MyOutputDebugString(_T("[pipe] finish"));
	return 0;
}

void DisconnectAndReconnect(DWORD i) {

	if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
		// todo
		// printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	Pipe[i].fPendingIO = ConnectToNewClient(
		Pipe[i].hPipeInst,
		&Pipe[i].oOverlap);

	Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
}


BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) {
	BOOL fConnected, fPendingIO = FALSE;
	fConnected = ConnectNamedPipe(hPipe, lpo);
	if (fConnected) {
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError()) {
		case ERROR_IO_PENDING:
			fPendingIO = TRUE;
			break;

		case ERROR_PIPE_CONNECTED:
			if (SetEvent(lpo->hEvent))
				break;
		default:
			printf("ConnectNamedPipe failed with %d.\n", GetLastError());
			return 0;
	}
	return fPendingIO;
}
