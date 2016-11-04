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
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include "../shared.h"
#include <time.h>

extern VOID FreeCom();
extern VOID ReadIni();
extern VOID HandleSysCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
extern VOID AddMenu(HWND hwnd, HMENU menu);
extern VOID RemoveMenu(HWND hwnd, HMENU menu);

LRESULT CALLBACK CallWndProc(INT code, WPARAM wParam, LPARAM lParam)
{
#define msg ((PCWPSTRUCT)lParam)
	if (code == HC_ACTION)
	{
		switch (msg->message)
		{
			// I am not sure if this is required, lets leve it in
		case WM_ACTIVATE:
		{
			Log("WM_ACTIVATE");
			GetSystemMenu(msg->hwnd, FALSE);
			break;
		}

		// Populate menu
		case WM_INITMENUPOPUP:
		{
			Log("WM_INITMENUPOPUP");
			AddMenu(msg->hwnd, (HMENU)msg->wParam);
			break;
		}

		// Some applications trigger WM_INITMENUPOPUP never or to late, thats why we use WM_ENTERIDLE
		case WM_ENTERIDLE:
		{
			Log("WM_ENTERIDLE");
			if (msg->wParam == MSGF_MENU)
			{
				AddMenu(msg->hwnd, (HMENU)INVALID_HANDLE_VALUE);
				break;
			}
			break;
		}

		// Remove Entry again
		case WM_UNINITMENUPOPUP:
		{
			Log("WM_UNINITMENUPOPUP");
			RemoveMenu(msg->hwnd, (HMENU)msg->wParam);
			break;
		}

		// For those who doesn't fire WM_UNINITMENUPOPUP 
		case WM_MENUSELECT:
		{
			Log("WM_ENTERIDLE");
			if (msg->lParam == NULL && HIWORD(msg->wParam) == 0xFFFF)
			{
				RemoveMenu(msg->hwnd, (HMENU)INVALID_HANDLE_VALUE);
			}
			break;
		}

		// Do the command
		case WM_SYSCOMMAND:
		{
			Log("WM_SYSCOMMAND %X %X %X", msg->hwnd, msg->wParam, msg->lParam);
			HandleSysCommand(msg->hwnd, msg->wParam, msg->lParam);
			break;
		}
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
#undef msg
}

LRESULT CALLBACK GetMsgProc(INT code, WPARAM wParam, LPARAM lParam)
{
#define msg ((PMSG)lParam)
	if (code == HC_ACTION)
	{
		switch (msg->message)
		{
		case WM_SYSCOMMAND:
		{
			Log("WM_SYSCOMMAND %X %X %X", msg->hwnd, msg->wParam, msg->lParam);
			HandleSysCommand(msg->hwnd, msg->wParam, msg->lParam);
			break;
		}
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
#undef msg
}

BOOL WINAPI DllMain(HINSTANCE handle, DWORD dwReason, LPVOID reserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		ReadIni();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		FreeCom();
	}
	return TRUE;
}
