# Debug Loop Library

Windows debuger API is quite tough to get working. You usually do not need to
use it because you already have some debugger that uses it behind the scenes.
If you are thinking about _making_ a debugger, then you'll need to make use of
the API.

This little library just provides a convenient abstraction to create event
loops in secondary threads, where each process has its own loop that waits
for orders from the UI one.  Debug Events are passed back as Windows messages
and can be handled within the message loop.

The library is thought to be compiled as a DLL callable from C code (so that
you can use it from any programming language).

To understand why this is convenient/necessary, continue reading.

The Windows Debugging API has a blocking function known as WaitForDebugEvent,
which _must be called from the thread that created/attached to the process
being debugged_. 
This has some problems: 
 - WaitForDebugEvent has no explicit argument to determine which process you
   are waiting for, [I think] it is implicitly assumed to be any process
   launched by that thread. You cannot wait for X process, you wait for all.
   While you may think you need to debug just one process, you are probably
   going to launch it many times, so the events could get mixed, confusing
   your debugger.
 - If you call WaitForDebugEvent with INFINITE wait time from your main thread,
   then you are going to block your UI until an event happens.
   
Then, the easiest way to get around this problem is to have one thread for
each debuggee, so that all events are mapped one to one to a process and you
can wait for a particular process to do something, instead of having to
demultiplex events from multiple processes in a single thread.

# How to use


It provides a single kind of object, the `DebugLoop` and async versions
of the Debugger API. You'll need to get an `HWND` handle because the communication
is done through the windows message pump to make things easier:

```
DebugLoop* LaunchDebugLoop(HWND window, UINT msg);
void SendCreateProcess (DebugLoop *loop, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags,
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation);
void SendAttachProcess(DebugLoop *loop, DWORD procId);
void FinishDebugLoop(DebugLoop* loop);
void SendWaitForEvent(DebugLoop *loop, LPDEBUG_EVENT event, DWORD millisecs);
void SendContinue(DebugLoop *loop, DWORD procId, DWORD threadId, DWORD continueStatus);
```

Usage example 
```

// ~~~ system setup ~~~
// we assume you already have the code to create a Window, now create
// a custom message code that your message pump will use to pass debug events
UINT WM_DEBUG_EVENT = RegisterWindowMessage(L"WM_DEBUG_EVENT");

...

// ~~~ debuggee process setup ~~~

// we first launch a new debug loop thread
DebugLoop *debuggeeLoop = LaunchDebugLoop(hwnd, msg);

// then, to launch a process in that thread, just do it as if using CreateProcess,
// but using the async version that passes the extra DebugLoop* parameter
SendCreateProcess (debuggeeLoop, your_exe, ... more params ...);

...

// window proc
switch (msg)
{
	case WM_DEBUG_EVENT:
	{
		DebugLoop *debugee = (DebugLoop*)wparam;
	    // ... handle the event ...
	    break;
	}
   ...
}

```

# How to build

I tried two toolchains in Windows: 
 - Visual Studio 2019 C++ Build Tools. 
   It takes quite some space but works OK.
 - MSYS2 based MinGW GCC/CMake. Fully open source but has some undesired properties
   like linking to libgcc dlls and being compiled with elf (linux) debug information
   instead of PDBs.
   

Build process can be triggered from console (using native tools command prompt for
VS 2019 or MSYS2-mingw64 environment) or from Visual Studio Code (not tested). 

## Compilation environment setup

### Visual Studio Build Tools

To build the launcher with Visual C++ compiler, you can install VS Build Tools with
chocolatey by running this in Powershell (run as administrator):

```
choco install visualstudio2019-workload-vctools
```

then open "x64 native tools command prompt for VS 2019" from start menu, you should
have the compiler ready. Maybe you need to install CMake manually (don't know because
I already had it before testing with this setup).

### MSYS2

Open an MSYS2 MinGW 64-bit environment from start menu. Then install the needed
tools using `pacman`:

```
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake
```


## Building from console

Compiling from the console is fairly easy. Instructions vary slightly from toolchain to
toolchain, but are equivalent in the end.  First execute CMake to create a Makefile file.

### Visual C++ Compiler

Open a Development Powershell for VS 2019. You can compile the 64 bits version with:

```
cmake -B build-vs2019-x64 -G "Visual Studio 16 2019" -A x64
cmake --build build-vs2019-x64 --config Release
```

### MSYS2 / mingw

You can compile for 64 bits using MSYS2-mingw64 like this:

```
cmake -B build-msys2-x64 -G "MinGW Makefiles"
cmake --build build-msys2-x64
```

If cross compiling from linux you will probably need to add a toolchain parameter to cmake
so that it compiles for Windows (`cmake -B build-msys2-x64 -DCMAKE_TOOLCHAIN_FILE:FILEPATH=toolchain-mingw-x64.cmake`
for 64 bits).


