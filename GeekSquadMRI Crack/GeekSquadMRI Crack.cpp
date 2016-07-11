/*
Compiled in Visual Studio 2016.
*/

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

// Comment out this line if debugging is not necessary.

#define DEBUG false

// Includes
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "detours.h"
#include "Winbase.h"
#include "Winhttp.h"

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"detours.lib")
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Winhttp.lib")

// Global variables
bool __hooked = false;
bool __calledBefore = false;

// Functions to be hooked
void (WINAPI *__hook_GetSystemTimeAsFileTime)(LPFILETIME lpSystemTimeAsFileTime) = GetSystemTimeAsFileTime;
HINTERNET (WINAPI *__hook_WinHttpConnect)(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved) = WinHttpConnect;

// Functions to be used prior to hooked functions
void (WINAPI __detour_GetSystemTimeAsFileTime)(LPFILETIME lpSystemTimeAsFileTime);
HINTERNET (WINAPI __detour_WinHttpConnect)(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved);

// Entry Point
// Just setting up everything
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH && __hooked == false) {
		__hooked = true;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(void*&)__hook_GetSystemTimeAsFileTime, __detour_GetSystemTimeAsFileTime);
		if (DetourTransactionCommit() != NO_ERROR) {
			MessageBox(NULL, "An error occured.  Please report error code 0x01 to developer", "MRI Loader", MB_OK | MB_ICONERROR);
			__hooked = false;
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(void*&)__hook_WinHttpConnect, __detour_WinHttpConnect);
		if (DetourTransactionCommit() != NO_ERROR) {
			MessageBox(NULL, "An error occured.  Please report error code 0x02 to developer", "MRI Loader", MB_OK | MB_ICONERROR);
			__hooked = false;
		}

		#ifdef DEBUG
			if (DEBUG) {
				if (__hooked) {
					MessageBox(NULL, "Successfully hooked all functions", "MRI Loader", MB_OK | MB_ICONERROR);
				}
			}
		#endif

		if (!__hooked) {
			MessageBox(NULL, "Patch failed", "MRI Loader", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	return true;
}

void (WINAPI __detour_GetSystemTimeAsFileTime)(LPFILETIME lpSystemTimeAsFileTime) {
	__hook_GetSystemTimeAsFileTime(lpSystemTimeAsFileTime);

	#ifdef DEBUG
		if(DEBUG) {
			char buffer[150];
			sprintf(buffer, "%d", lpSystemTimeAsFileTime->dwHighDateTime);
			MessageBox(NULL, buffer, "MRI Loader", MB_OK);
			sprintf(buffer, "%d", lpSystemTimeAsFileTime->dwLowDateTime);
			MessageBox(NULL, buffer, "MRI Loader", MB_OK);
		}
	#endif

	lpSystemTimeAsFileTime->dwHighDateTime = INT_MAX;
	lpSystemTimeAsFileTime->dwLowDateTime = INT_MAX;
}

HINTERNET(WINAPI __detour_WinHttpConnect)(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, DWORD dwReserved) {
	char buffer[500];
	wcstombs(buffer, pswzServerName, 500);
	#ifdef DEBUG
		if (DEBUG) {
			MessageBox(NULL, buffer, "MRI Loader", MB_OK);
		}
	#endif

	if (strstr(buffer, "geeksquadcentral.com")) {
		#ifdef DEBUG
			if (DEBUG) {
				char mbBuffer[150];
				sprintf(mbBuffer, "Blocked %s", buffer);
				MessageBox(NULL, mbBuffer, "MRI Loader", MB_OK);
			}
		#endif
		return 0;
	}

	return __hook_WinHttpConnect(hSession, pswzServerName, nServerPort, dwReserved);
}

/*
Fake export function
*/
extern "C" __declspec(dllexport) void NullExport(void ) {
	return;
}