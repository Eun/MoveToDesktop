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
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"
#include "hideim.h"
#include <VersionHelpers.h>
#include "../Hook/hook.h"


#ifdef _WIN64
#define MUTEX_NAME TEXT("{92B297B9-7430-4BB0-B77B-EB6D36DCF8F2}")
#define TITLE "MoveToDesktop x64"
#else
#define MUTEX_NAME TEXT("{4EF85FA7-55CB-4BD9-AD73-15EDA3BB149C}")
#define TITLE "MoveToDesktop"
#endif

bool bKeyDown = false;

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

BOOL IsWow64()
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
		}
	}
	return bIsWow64;
}


LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
#define msg ((PKBDLLHOOKSTRUCT)lParam)
	if (code == HC_ACTION)
	{
		if (wParam == WM_SYSKEYDOWN)
		{
			if (msg->vkCode == VK_RIGHT)
			{
				if (bKeyDown == false && ((GetKeyState(VK_LMENU) && GetKeyState(VK_LWIN)) || (GetKeyState(VK_RMENU) && GetKeyState(VK_RWIN))))
				{
					bKeyDown = true;
					Log("Sending to %X: MOVETOMENU_RIGHT", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_RIGHT, 0);
					return 1;
				}
			}
			else if (msg->vkCode == VK_LEFT)
			{
				if (bKeyDown == false && ((GetKeyState(VK_LMENU) && GetKeyState(VK_LWIN)) || (GetKeyState(VK_RMENU) && GetKeyState(VK_RWIN))))
				{
					bKeyDown = true;
					Log("Sending to %X: MOVETOMENU_LEFT", GetForegroundWindow());
					PostMessage(GetForegroundWindow(), WM_SYSCOMMAND, MOVETOMENU_LEFT, 0);
					return 1;
				}
			}
		}
		else if (wParam == WM_SYSKEYUP)
		{
			if (msg->vkCode == VK_RIGHT || msg->vkCode == VK_LEFT)
			{
				bKeyDown = false;
			}
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
#undef msg
}

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, INT cmdShow)
{

	if (!IsWindows10OrGreater())
	{
		MessageBox(0, "This application is for Windows 10 only!", TITLE, MB_OK | MB_ICONERROR);
		return 1;
	}
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

	// ensure that only one instance is running (but don't fail if the test fails)
	HANDLE mutex = CreateMutex(NULL, FALSE, MUTEX_NAME);

	if (mutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		#ifdef _WIN32
			MessageBox(0, "MoveToDesktop is already running!", TITLE, MB_OK | MB_ICONERROR);
		#endif
		return 1;
	}

	// Extract 64bit version
	#ifdef _WIN32
	#ifndef _DEBUG
	if (IsWow64())
	{
		TCHAR szTempExeFileName[MAX_PATH];
		_tcscpy_s(szTempExeFileName, _countof(szTempExeFileName), lpTempPathBuffer);
		_tcscat_s(szTempExeFileName, _countof(szTempExeFileName), _T("MoveToDesktop.x64.exe"));

		if (ExtractResource(instance, IDR_EXE64, szTempExeFileName) == false)
		{
			MessageBox(0, "Extracting x64 version failed!", TITLE, MB_OK | MB_ICONERROR);
			return 1;
		}

		ShellExecute(NULL, _T("open"), szTempExeFileName, NULL, NULL, SW_SHOWNORMAL);

	}
	#endif
	#endif

	DWORD error;
	HMODULE library;
	// load the hook library
	if ((library = LoadLibrary(szTempFileName)) == NULL)
	{
		error = GetLastError();
		char buffer[128] = "";
		sprintf_s(buffer, sizeof(buffer), "Could not load hook!\nErrorCode: %d", error);
		MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
		return error;
	}
	try
	{
		HOOKPROC callWndProc;
		HHOOK callWndHook;

		if ((callWndProc = (HOOKPROC)GetProcAddress(library, "CallWndProc")) == NULL)
		{
			error = GetLastError();
			throw "Could not find CallWndProc in hook!";
		}
		if ((callWndHook = SetWindowsHookExA(WH_CALLWNDPROC, callWndProc, library, 0)) == NULL)
		{
			error = GetLastError();
			char buffer[128] = "";
			sprintf_s(buffer, sizeof(buffer), "Error on calling SetWindowsHookEx(WH_CALLWNDPROC)!\nErrorCode: %d", error);
			throw buffer;
		}
		try
		{
			HOOKPROC getMsgProc;
			HHOOK getMsgHook;

			if ((getMsgProc = (HOOKPROC)GetProcAddress(library, "GetMsgProc")) == NULL)
			{
				error = GetLastError();
				throw "Could not find GetMsgProc in hook.dll!";
			}
			if ((getMsgHook = SetWindowsHookExA(WH_GETMESSAGE, getMsgProc, library, 0)) == NULL)
			{
				error = GetLastError();
				char buffer[128] = "";
				sprintf_s(buffer, sizeof(buffer), "Error on calling SetWindowsHookEx(WH_GETMESSAGE)!\nErrorCode: %d", error);
				throw buffer;
			}

			try
			{
				HHOOK lowLevelKeyboardHook;

				if ((lowLevelKeyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, LowLevelKeyboardProc, library, 0)) == NULL)
				{
					error = GetLastError();
					char buffer[128] = "";
					sprintf_s(buffer, sizeof(buffer), "Error on calling SetWindowsHookEx(WH_KEYBOARD_LL)!\nErrorCode: %d", error);
					throw buffer;
				}
				try
				{
					MSG msg;
					BOOL ret;

					// pump the messages until the end of the session
					while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0)
					{
						if (ret == -1)
						{
							return GetLastError();
						}
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				catch(char *buffer)
				{
					MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
				}
				UnhookWindowsHookEx(lowLevelKeyboardHook);
			}
			catch (char *buffer)
			{
				MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
			}
			UnhookWindowsHookEx(getMsgHook);
		}
		catch (char *buffer)
		{
			MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
		}
		UnhookWindowsHookEx(callWndHook);
	}
	catch (char *buffer)
	{
		MessageBox(0, buffer, TITLE, MB_OK | MB_ICONERROR);
	}

	FreeLibrary(library);

	ReleaseMutex(mutex);

	// return success
	return ERROR_SUCCESS;
}
