
#include "debug_loop.h"
#include <windows.h>

DWORD WINAPI debug_loop_thread_entrypoint(void* loop)
{
	static_cast<DebugLoop*>(loop)->loop();

	return 0;
}

extern "C" __declspec(dllexport) DebugLoop* LaunchDebugLoop(HWND window, UINT msg)
{
	DebugLoop *loop = new DebugLoop(window, msg);
	CreateThread(NULL, 0, debug_loop_thread_entrypoint, loop, 0, NULL);
	return loop;
}

extern "C" __declspec(dllexport)
void SendCreateProcess (DebugLoop *loop, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	loop->nextCommand.type = CommandType::CT_CreateProcess;
	loop->nextCommand.args.create.lpApplicationName = lpApplicationName;
	loop->nextCommand.args.create.lpCommandLine = lpCommandLine;
	loop->nextCommand.args.create.lpProcessAttributes = lpProcessAttributes;
	loop->nextCommand.args.create.lpThreadAttributes = lpThreadAttributes;
	loop->nextCommand.args.create.bInheritHandles = bInheritHandles;
	loop->nextCommand.args.create.dwCreationFlags = dwCreationFlags;
	loop->nextCommand.args.create.lpEnvironment = lpEnvironment;
	loop->nextCommand.args.create.lpCurrentDirectory = lpCurrentDirectory;
	loop->nextCommand.args.create.lpStartupInfo = lpStartupInfo;
	loop->nextCommand.args.create.lpProcessInformation = lpProcessInformation;
	loop->dispatchCommand();
	WaitForSingleObject(loop->backSemaphore, INFINITE);
}

extern "C" __declspec(dllexport)
void SendAttachProcess(DebugLoop *loop, DWORD procId)
{
	loop->nextCommand.type = CommandType::CT_AttachProcess;
	loop->nextCommand.args.attach.procId = procId;
	loop->dispatchCommand();
	WaitForSingleObject(loop->backSemaphore, INFINITE);
}

extern "C" __declspec(dllexport)
void FinishDebugLoop(DebugLoop* loop)
{
	loop->nextCommand.type = CommandType::CT_Finish;
	loop->dispatchCommand();
}

extern "C" __declspec(dllexport)
void SendWaitForEvent(DebugLoop *loop, LPDEBUG_EVENT event, DWORD millisecs)
{
	loop->nextCommand.type = CommandType::CT_WaitForEvent;
	loop->nextCommand.args.wait.event = event;
	loop->nextCommand.args.wait.millisecs = millisecs;
	loop->dispatchCommand();
}

extern "C" __declspec(dllexport)
void SendContinue(DebugLoop *loop, DWORD procId, DWORD threadId, DWORD continueStatus)
{
	loop->nextCommand.type = CommandType::CT_Continue;
	loop->nextCommand.args.cont.procId = procId;
	loop->nextCommand.args.cont.threadId = threadId;
	loop->nextCommand.args.cont.continueStatus = continueStatus;
	loop->dispatchCommand();
	WaitForSingleObject(loop->backSemaphore, INFINITE);
}

