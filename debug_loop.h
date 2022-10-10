
#ifndef _DEBUG_LOOP_H_
#define _DEBUG_LOOP_H_

#include <windows.h>

enum CommandType { CT_CreateProcess, CT_AttachProcess, CT_WaitForEvent, CT_Continue, CT_Finish};

struct CreateProcessParams {
	LPCWSTR               lpApplicationName;
	LPWSTR                lpCommandLine;
	LPSECURITY_ATTRIBUTES lpProcessAttributes;
	LPSECURITY_ATTRIBUTES lpThreadAttributes;
	BOOL                  bInheritHandles;
	DWORD                 dwCreationFlags;
	LPVOID                lpEnvironment;
	LPCWSTR               lpCurrentDirectory;
	LPSTARTUPINFOW        lpStartupInfo;
	LPPROCESS_INFORMATION lpProcessInformation;
};

struct AttachProcessParams {
	DWORD procId;
};

struct WaitEventParams {
	LPDEBUG_EVENT event;
	DWORD millisecs;
};

struct ContinueParams {
	DWORD procId;
	DWORD threadId;
	DWORD continueStatus;
};

struct DebugCommand
{
	CommandType type;
	union args{
		CreateProcessParams create;
		AttachProcessParams attach;
		WaitEventParams wait;
		ContinueParams cont;
	} args;
};

class DebugLoop
{
public:
	HWND hwnd;
	UINT message;
	HANDLE semaphore;
	HANDLE backSemaphore;
	DWORD process;
	volatile DebugCommand nextCommand;

	DebugLoop(HWND hwnd, UINT message);
	void loop() volatile;
	void dispatchCommand() volatile;
};

extern "C" __declspec(dllexport) DebugLoop* LaunchDebugLoop(HWND window, UINT msg);
extern "C" __declspec(dllexport) void SendCreateProcess (DebugLoop *loop, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation);
extern "C" __declspec(dllexport) void SendAttachProcess(DebugLoop *loop, DWORD procId);
extern "C" __declspec(dllexport) void FinishDebugLoop(DebugLoop* loop);
extern "C" __declspec(dllexport) void SendWaitForEvent(DebugLoop *loop, LPDEBUG_EVENT event, DWORD millisecs);
extern "C" __declspec(dllexport) void SendContinue(DebugLoop *loop, DWORD procId, DWORD threadId, DWORD continueStatus);

#endif  // ~ _DEBUG_LOOP_H_ ~


