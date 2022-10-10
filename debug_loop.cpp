
#include <cstdio>
#include <windows.h>
#include "debug_loop.h"

DebugLoop::DebugLoop(HWND window, UINT msg)
{
	hwnd = window;
	message = msg;
	semaphore = CreateSemaphore(0, 0, 1, 0);
	backSemaphore = CreateSemaphore(0, 0, 1, 0);
}

void DebugLoop::dispatchCommand() volatile
{
	ReleaseSemaphore(semaphore, 1, NULL);
}

void DebugLoop::loop() volatile
{
	while(true) 
	{  
		WaitForSingleObject(semaphore, INFINITE);
		switch (nextCommand.type) {
			case CommandType::CT_Finish: return;
			case CommandType::CT_WaitForEvent:
				WaitForDebugEvent(
					nextCommand.args.wait.event,
				 	nextCommand.args.wait.millisecs); 
				PostMessage(hwnd, message, (WPARAM)(void*)this, (LPARAM)NULL);
				break;
			case CommandType::CT_Continue:
				ContinueDebugEvent(
					nextCommand.args.cont.procId,
					nextCommand.args.cont.threadId,
					nextCommand.args.cont.continueStatus);
				ReleaseSemaphore(backSemaphore, 1, NULL);
				break;
			case CommandType::CT_AttachProcess:
				process = nextCommand.args.attach.procId;
				DebugActiveProcess(process);
				ReleaseSemaphore(backSemaphore, 1, NULL);
				break;
			case CommandType::CT_CreateProcess:
				::CreateProcess(
					(LPCWSTR)nextCommand.args.create.lpApplicationName,
					(LPWSTR)nextCommand.args.create.lpCommandLine,
					nextCommand.args.create.lpProcessAttributes,
					nextCommand.args.create.lpThreadAttributes,
					nextCommand.args.create.bInheritHandles,
					nextCommand.args.create.dwCreationFlags,
					nextCommand.args.create.lpEnvironment,
					nextCommand.args.create.lpCurrentDirectory,
					nextCommand.args.create.lpStartupInfo,
					nextCommand.args.create.lpProcessInformation);
				process = nextCommand.args.create.lpProcessInformation->dwProcessId;
				ReleaseSemaphore(backSemaphore, 1, NULL);
				break;
		}

	}

}	



