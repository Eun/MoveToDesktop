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
	TCHAR szTempFileName[MAX_PATH] = _T("../hook/hook.dll");
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
