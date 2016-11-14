/**
* MoveToDesktop
*
* Copyright (C) 2015-2016 by Tobias Salzmann
* Copyright (C) 2008-2011 by Manuel Meitinger
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <VersionHelpers.h>
#include "resource.h"
#include "hideim.h"
#include "KeyMapping.h"
#include "../shared.h"
#include <ShellAPI.h>
#include <signal.h>

#include "../hooklib/hook.h"

#ifdef _WIN64
#define MUTEX_NAME TEXT("{92B297B9-7430-4BB0-B77B-EB6D36DCF8F2}")
#define TITLE "MoveToDesktop x64"
#else
#define MUTEX_NAME TEXT("{4EF85FA7-55CB-4BD9-AD73-15EDA3BB149C}")
#define TITLE "MoveToDesktop"
#endif

HANDLE mutex = nullptr;
HMODULE hookLibrary = nullptr;
HHOOK callWndHook = nullptr;
HHOOK getMsgHook = nullptr;
HANDLE shutdownEvent = nullptr;

#ifndef _WIN64
BOOL bGotMoveLeftHotKey = FALSE;
BOOL bGotMoveRightHotKey = FALSE;
BOOL bGotMoveAndSwitchLeftHotKey = FALSE;
BOOL bGotMoveAndSwitchRightHotKey = FALSE;
BOOL bHotSwitchToDesktop1 = FALSE;
BOOL bHotSwitchToDesktop2 = FALSE;
BOOL bHotSwitchToDesktop3 = FALSE;
BOOL bHotSwitchToDesktop4 = FALSE;
BOOL bHotSwitchToDesktop5 = FALSE;
BOOL bHotSwitchToDesktop6 = FALSE;
BOOL bHotSwitchToDesktop7 = FALSE;
BOOL bHotSwitchToDesktop8 = FALSE;
BOOL bHotSwitchToDesktop9 = FALSE;
BOOL bHotSwitchToDesktop10 = FALSE;
BOOL bHotSwitchToDesktop11 = FALSE;
BOOL bHotSwitchToDesktop12 = FALSE;
#endif

bool ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCTSTR szFilename)
{
	try
	{
		HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
		if (hResource == NULL)
			return false;
		HGLOBAL hFileResource = LoadResource(hInstance, hResource);
		if (hFileResource == NULL)
			return false;

		LPVOID lpFile = LockResource(hFileResource);
		DWORD dwSize = SizeofResource(hInstance, hResource);

		HANDLE hFile = CreateFile(szFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwByteWritten;
		WriteFile(hFile, lpFile, dwSize, &dwByteWritten, NULL);
		CloseHandle(hFile);
		FreeResource(hFileResource);
		return true;

	}
	catch (...)
	{
		return false;
	}
}

BOOL GetHotKey(PTCHAR setting, SIZE_T size, PINT modifiers, PINT keycode)
{
	if (setting == NULL || size == 0)
		return FALSE;
	INT start = 0;
	SIZE_T nLen;
	*modifiers = 0;
	for (UINT i = 0; i < size; i++)
	{
		if (setting[i] == '+')
		{
			nLen = i - start;
			for (INT j = 0; j < nKeyModifiersSize; j++)
			{
				if (_tcslen(keyModifiers[j].Name) == nLen && _strnicmp(&setting[start], keyModifiers[j].Name, nLen) == 0)
				{
					*modifiers |= keyModifiers[j].Value;
					break;
				}
			}
			start = i+1;
		}
	}
	*keycode = 0;
	nLen = size - start;
	for (INT j = 0; j < nKeyKeysSize; j++)
	{
		if (_tcslen(keyKeys[j].Name) == nLen && _strnicmp(&setting[start], keyKeys[j].Name, nLen) == 0)
		{
			*keycode = keyKeys[j].Value;
			break;
		}
	}
	return *keycode != 0;
}

BOOL RegisterHotKey(PTCHAR iniFile, PTCHAR setting, PTCHAR default, INT intval)
{
	BOOL bGotHotKey = FALSE;

	TCHAR Key[MAX_HOTKEY_SIZE] = { 0 };

	INT KeySize = GetPrivateProfileString("HotKeys", setting, default, Key, MAX_HOTKEY_SIZE, iniFile);
	INT KeyModifiers = 0;
	INT KeyCode = 0;
	bGotHotKey = GetHotKey(Key, KeySize, &KeyModifiers, &KeyCode);

	if (bGotHotKey)
	{
		Log("Ini: %s Modifiers=%d, Key=%d", setting, KeyModifiers, KeyCode);
		if (!::RegisterHotKey(NULL, intval, KeyModifiers | MOD_NOREPEAT, KeyCode))
		{
			INT error = GetLastError();
			char buffer[128] = "";
			sprintf_s(buffer, sizeof(buffer), "Error on Registering %s (%s)!\nErrorCode: %d", setting, Key, error);
			throw buffer;
		}
	}
	return bGotHotKey;
}

void sighandler(int signum)
{
	Log("Caught Signal %d", signum);
	if (getMsgHook != nullptr)
		UnhookWindowsHookEx(getMsgHook);
	if (callWndHook != nullptr)
		UnhookWindowsHookEx(callWndHook);

#ifndef _WIN64
	if (bGotMoveLeftHotKey)
		UnregisterHotKey(NULL, MOVETOMENU_LEFT);
	if (bGotMoveRightHotKey)
		UnregisterHotKey(NULL, MOVETOMENU_RIGHT);
	if (bGotMoveAndSwitchLeftHotKey)
		UnregisterHotKey(NULL, MOVETOMENU_LEFT_SWITCH);
	if (bGotMoveAndSwitchRightHotKey)
		UnregisterHotKey(NULL, MOVETOMENU_RIGHT_SWITCH);
	if (bHotSwitchToDesktop1)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP1);
	if (bHotSwitchToDesktop2)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP2);
	if (bHotSwitchToDesktop3)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP3);
	if (bHotSwitchToDesktop4)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP4);
	if (bHotSwitchToDesktop5)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP5);
	if (bHotSwitchToDesktop6)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP6);
	if (bHotSwitchToDesktop7)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP7);
	if (bHotSwitchToDesktop8)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP8);
	if (bHotSwitchToDesktop9)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP9);
	if (bHotSwitchToDesktop10)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP10);
	if (bHotSwitchToDesktop11)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP11);
	if (bHotSwitchToDesktop12)
		UnregisterHotKey(NULL, SWITCH_TO_DESKTOP12);
#endif
	if (hookLibrary != nullptr)
		FreeLibrary(hookLibrary);

	if (mutex != nullptr)
		ReleaseMutex(mutex);

	if (shutdownEvent != nullptr)
		CloseHandle(shutdownEvent);

	exit(signum);
}

DWORD WINAPI WaitForShutDownEvent(LPVOID)
{
	if (!WaitForSingleObject(shutdownEvent, INFINITE))
	{
		sighandler(-1);
	}
	return 0;
}

HHOOK SetHook(int id, LPCSTR method)
{
	HOOKPROC proc;
	HHOOK hook;

	if ((proc = (HOOKPROC)GetProcAddress(hookLibrary, method)) == NULL)
	{
		int error = GetLastError();
		char buffer[128] = "";
		sprintf_s(buffer, sizeof(buffer), "Could not find %s in hook.dll!\nErrorCode: %d", method, error);
		throw buffer;;
	}
	if ((hook = SetWindowsHookExA(id, proc, hookLibrary, 0)) == NULL)
	{
		int error = GetLastError();
		char buffer[128] = "";
		sprintf_s(buffer, sizeof(buffer), "Error on calling SetWindowsHookEx(%d) for %s!\nErrorCode: %d", id, method, error);
		throw buffer;
	}
	return hook;
}

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, INT cmdShow)
{
	if (!IsWindows10OrGreater())
	{
		MessageBox(0, "This application is for Windows 10 only!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}

	LPWSTR *szArglist;
	int nArgs;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist != NULL)
	{
		if (nArgs > 1)
		{
			if (!_wcsicmp(szArglist[1], L"--create-desktop"))
			{
				UINT index;
				return !CreateDesktop(&index);
			}
			else if (!_wcsicmp(szArglist[1], L"--desktop-count"))
			{
				UINT index;
				UINT count;
				if (GetCurrentDesktopIndex(&index, &count))
					return count;
				return -1;
			}
			else if (!_wcsicmp(szArglist[1], L"--desktop-index"))
			{
				UINT index;
				UINT count;
				if (GetCurrentDesktopIndex(&index, &count))
					return index;
				return -1;
			}
			else if (!_wcsicmp(szArglist[1], L"--remove-empty-desktops"))
			{
				return RemoveEmptyDesktops();
			}

			else if (!_wcsicmp(szArglist[1], L"--is-running"))
			{
				Log("Reading Ini");
				TCHAR iniFile[MAX_PATH] = { 0 };
				ExpandEnvironmentStrings(INIFILE, iniFile, _countof(iniFile));
				TCHAR MutexName[MAX_HOTKEY_SIZE] = { 0 };
				INT MutexNameSize;

#ifdef _WIN64
				MutexNameSize = GetPrivateProfileString("Advanced", "Mutex_x64", MUTEX_NAME, MutexName, 40, iniFile);
#else
				MutexNameSize = GetPrivateProfileString("Advanced", "Mutex_x86", MUTEX_NAME, MutexName, 40, iniFile);
#endif

				HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MutexName);

				if (mutex != NULL)
				{
					CloseHandle(mutex);
					return 1;
				}
				return 0;
			}

			else if (!_wcsicmp(szArglist[1], L"--exit"))
			{
				HANDLE shutdownEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "MoveToDesktopShutdown");
				if (shutdownEvent != NULL)
				{
					SetEvent(shutdownEvent);
					CloseHandle(shutdownEvent);
				}
				return 0;
			}

			else if (!_wcsicmp(szArglist[1], L"/help") || !_wcsicmp(szArglist[1], L"/?") || !_wcsicmp(szArglist[1], L"--help"))
			{
				printf("MoveToDesktop\n");
				printf("Desktop Functions\n");
				printf("    --create-desktop                     Create a new desktop\n");
				printf("    --desktop-count                      Return the current desktop count\n");
				printf("    --desktop-index                      Return the current desktop index\n");
				printf("    --switch-desktop <INDEX>             Switch the desktop to the index\n");
				printf("    --remove-desktop <INDEX> <FALLBACK>  Remove the desktop at INDEX, and switch to FALLBACK if it is the current desktop\n");
				printf("    --remove-empty-desktops              Remove all desktops that are empty\n");
				printf("\n");
				printf("Window Functions\n");
				printf("    --move-to-new-desktop <HWND>         Create a new desktop and move the window with the HWND to the desktop\n");
				printf("    --move-to-left-desktop <HWND>        Move the window with the HWND to the left desktop\n");
				printf("    --move-to-right-desktop <HWND>       Move the window with the HWND to the right desktop\n");
				printf("    --move-to-desktop <HWND> <INDEX>     Move the window with the HWND to the desktop at INDEX\n");
				printf("\n");
				printf("General Functions\n");
				printf("    --is-running              Return 0 if this runner is running\n");
				printf("    --exit                    Exit the runner\n");
				printf("    --help                    Show the help\n");
				printf("\n");
				printf("Notice that all return values are exit codes!\n");
				return 1;
			}

		}
		if (nArgs > 2)
		{
			if (!_wcsicmp(szArglist[1], L"--switch-desktop"))
			{
				UINT index = _wtoi(szArglist[2]);
				return !SwitchDesktop(index);
			}
			else if (!_wcsicmp(szArglist[1], L"--move-to-new-desktop"))
			{
				HWND hwnd = (HWND)wcstoll(szArglist[2], NULL, 10);
				return (int)PostMessage(hwnd, WM_SYSCOMMAND, MOVETOMENU_NEW, 1);
			}
			else if (!_wcsicmp(szArglist[1], L"--move-to-left-desktop"))
			{
				HWND hwnd = (HWND)wcstoll(szArglist[2], NULL, 10);
				return (int)PostMessage(hwnd, WM_SYSCOMMAND, MOVETOMENU_LEFT, 1);
			}
			else if (!_wcsicmp(szArglist[1], L"--move-to-right-desktop"))
			{
				HWND hwnd = (HWND)wcstoll(szArglist[2], NULL, 10);
				return (int)PostMessage(hwnd, WM_SYSCOMMAND, MOVETOMENU_RIGHT, 1);
			}
		}
		if (nArgs > 3)
		{
			if (!_wcsicmp(szArglist[1], L"--remove-desktop"))
			{
				UINT index = _wtoi(szArglist[2]);
				UINT fallback = _wtoi(szArglist[3]);
				return !RemoveDesktop(index, fallback);
			}
			else if (!_wcsicmp(szArglist[1], L"--move-to-desktop"))
			{
				HWND hwnd = (HWND)wcstoll(szArglist[2], NULL, 10);
				UINT index = _wtoi(szArglist[3]);
				return (int)PostMessage(hwnd, WM_SYSCOMMAND, MOVETOMENU_START + index, 1);
			}
		}
	}
	


#ifndef _DEBUG
	TCHAR lpTempPathBuffer[MAX_PATH];
	TCHAR szTempFileName[MAX_PATH];
	DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
	if (dwRetVal > MAX_PATH || (dwRetVal == 0))
	{
		MessageBox(0, "GetTempPath failed!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}


	if (GetTempFileName(lpTempPathBuffer, _T(""), 0, szTempFileName) == 0)
	{
		MessageBox(0, "GetTempFileName failed!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}


	if (ExtractResource(instance, IDR_DLL1, szTempFileName) == false)
	{
		MessageBox(0, "Extracting hook failed!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}
#else
	TCHAR szTempFileName[MAX_PATH] = _T("../hookdll/hook.dll");
#endif

	Log("Reading Ini");
	TCHAR iniFile[MAX_PATH] = { 0 };
	ExpandEnvironmentStrings(INIFILE, iniFile, _countof(iniFile));
	TCHAR MutexName[MAX_HOTKEY_SIZE] = { 0 };
	INT MutexNameSize;
	
#ifdef _WIN64
	MutexNameSize = GetPrivateProfileString("Advanced", "Mutex_x64", MUTEX_NAME, MutexName, 40, iniFile);
#else
	MutexNameSize = GetPrivateProfileString("Advanced", "Mutex_x86", MUTEX_NAME, MutexName, 40, iniFile);
#endif
	SECURITY_ATTRIBUTES sa = { sizeof(sa) };
	SECURITY_DESCRIPTOR SD;
	InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&SD, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &SD;
	mutex = CreateMutex(&sa, FALSE, MutexName);

	if (mutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(0, "MoveToDesktop is already running!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}

#ifdef _WIN64
	Log("hook.x64.dll is at %s", szTempFileName);
#else
	Log("hook.x86.dll is at %s", szTempFileName);
#endif

	INT status = ERROR_SUCCESS;
	DWORD error;
	
	// load the hook library
	if ((hookLibrary = LoadLibrary(szTempFileName)) == NULL)
	{
		error = GetLastError();
		char buffer[128] = "";
		sprintf_s(buffer, sizeof(buffer), "Could not load hook!\nErrorCode: %d", error);
		MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
		status = 1;
		goto end;
	}
	try
	{
		callWndHook = SetHook(WH_CALLWNDPROC, "CallWndProc");
		getMsgHook = SetHook(WH_GETMESSAGE, "GetMsgProc");
	}
	catch (char *buffer)
	{
		MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
		status = 1;
		goto end;
	}
	try
	{
		#ifndef _WIN64
				bGotMoveLeftHotKey = RegisterHotKey(iniFile, "MoveLeft", "WIN+ALT+LEFT", MOVETOMENU_LEFT);
				bGotMoveRightHotKey = RegisterHotKey(iniFile, "MoveRight", "WIN+ALT+RIGHT", MOVETOMENU_RIGHT);
				bGotMoveAndSwitchLeftHotKey = RegisterHotKey(iniFile, "MoveAndSwitchLeft", "", MOVETOMENU_LEFT_SWITCH);
				bGotMoveAndSwitchRightHotKey = RegisterHotKey(iniFile, "MoveAndSwitchRight", "", MOVETOMENU_RIGHT_SWITCH);

				bHotSwitchToDesktop1 = RegisterHotKey(iniFile, "HotKeySwitchDesktop1", "WIN+ALT+F1", SWITCH_TO_DESKTOP1);
				bHotSwitchToDesktop2 = RegisterHotKey(iniFile, "HotKeySwitchDesktop2", "WIN+ALT+F2", SWITCH_TO_DESKTOP2);
				bHotSwitchToDesktop3 = RegisterHotKey(iniFile, "HotKeySwitchDesktop3", "WIN+ALT+F3", SWITCH_TO_DESKTOP3);
				bHotSwitchToDesktop4 = RegisterHotKey(iniFile, "HotKeySwitchDesktop4", "WIN+ALT+F4", SWITCH_TO_DESKTOP4);
				bHotSwitchToDesktop5 = RegisterHotKey(iniFile, "HotKeySwitchDesktop5", "WIN+ALT+F5", SWITCH_TO_DESKTOP5);
				bHotSwitchToDesktop6 = RegisterHotKey(iniFile, "HotKeySwitchDesktop6", "WIN+ALT+F6", SWITCH_TO_DESKTOP6);
				bHotSwitchToDesktop7 = RegisterHotKey(iniFile, "HotKeySwitchDesktop7", "WIN+ALT+F7", SWITCH_TO_DESKTOP7);
				bHotSwitchToDesktop8 = RegisterHotKey(iniFile, "HotKeySwitchDesktop8", "WIN+ALT+F8", SWITCH_TO_DESKTOP8);
				bHotSwitchToDesktop9 = RegisterHotKey(iniFile, "HotKeySwitchDesktop9", "WIN+ALT+F9", SWITCH_TO_DESKTOP9);
				bHotSwitchToDesktop10 = RegisterHotKey(iniFile, "HotKeySwitchDesktop10", "WIN+ALT+F10", SWITCH_TO_DESKTOP10);
				bHotSwitchToDesktop11 = RegisterHotKey(iniFile, "HotKeySwitchDesktop11", "WIN+ALT+F11", SWITCH_TO_DESKTOP11);
				bHotSwitchToDesktop12 = RegisterHotKey(iniFile, "HotKeySwitchDesktop12", "WIN+ALT+F12", SWITCH_TO_DESKTOP12);
		#endif
		MSG msg;
		BOOL ret;

		signal(SIGABRT, &sighandler);
		signal(SIGTERM, &sighandler);
		signal(SIGINT, &sighandler);
		signal(SIGBREAK, &sighandler);

		shutdownEvent = CreateEvent(0, TRUE, FALSE, "MoveToDesktopShutdown");

		Log("shutdownEvent is %x", shutdownEvent);

		CreateThread(NULL, NULL, WaitForShutDownEvent, NULL, NULL, NULL);

		// pump the messages until the end of the session
		while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (ret == -1)
			{
				return GetLastError();
			}
			if (msg.message == WM_HOTKEY)
			{
				if (msg.wParam == MOVETOMENU_LEFT)
				{
					Log("Sending to %X: MOVETOMENU_LEFT", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_LEFT, 0);
				}
				else if (msg.wParam == MOVETOMENU_RIGHT)
				{
					Log("Sending to %X: MOVETOMENU_RIGHT", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_RIGHT, 0);
				}
				else if (msg.wParam == MOVETOMENU_LEFT_SWITCH)
				{
					Log("Sending to %X: MOVETOMENU_LEFT_SWITCH", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_LEFT_SWITCH, 0);
				}
				else if (msg.wParam == MOVETOMENU_RIGHT_SWITCH)
				{
					Log("Sending to %X: MOVETOMENU_RIGHT_SWITCH", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_RIGHT_SWITCH, 0);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP1)
				{
					Log("Switching Desktop to 0");
					SwitchDesktop((UINT)0);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP2)
				{
					Log("Switching Desktop to 1");
					SwitchDesktop((UINT)1);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP3)
				{
					Log("Switching Desktop to 2");
					SwitchDesktop((UINT)2);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP4)
				{
					Log("Switching Desktop to 3");
					SwitchDesktop((UINT)3);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP5)
				{
					Log("Switching Desktop to 4");
					SwitchDesktop((UINT)4);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP6)
				{
					Log("Switching Desktop to 5");
					SwitchDesktop((UINT)5);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP7)
				{
					Log("Switching Desktop to 6");
					SwitchDesktop((UINT)6);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP8)
				{
					Log("Switching Desktop to 7");
					SwitchDesktop((UINT)7);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP9)
				{
					Log("Switching Desktop to 8");
					SwitchDesktop((UINT)8);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP10)
				{
					Log("Switching Desktop to 9");
					SwitchDesktop((UINT)9);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP11)
				{
					Log("Switching Desktop to 10");
					SwitchDesktop((UINT)10);
				}
				else if (msg.wParam == SWITCH_TO_DESKTOP12)
				{
					Log("Switching Desktop to 11");
					SwitchDesktop((UINT)11);
				}
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	catch(char *buffer)
	{
		MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
		status = 1;
	}
end:
	sighandler(status);

	// return success
	return status;
}
