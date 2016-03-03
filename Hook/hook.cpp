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
#include <stdio.h>
#include <process.h>
#include "VirtualDesktops.h"
#include "hook.h"

IServiceProvider* pServiceProvider = nullptr;
IVirtualDesktopManager *pDesktopManager = nullptr;
IVirtualDesktopManagerInternal* pDesktopManagerInternal = nullptr;

enum EComStatus
{
	COMSTATUS_UNINITIALIZED,
	COMSTATUS_INITIALIZED,
	COMSTATUS_ERROR,
};

int ComStatus = COMSTATUS_UNINITIALIZED;

bool bAddedMenu = false;
bool bReadIni = false;
bool bSwitchDesktopAfterMove = false;

#define INIFILE "%APPDATA%\\MoveToDesktop.ini"

BOOL InitCom()
{
	Log("Initalizing Com");
	if (ComStatus == COMSTATUS_INITIALIZED)
	{
		Log("> Allready Initialized!");
		return true;
	}
	else if (ComStatus == COMSTATUS_ERROR)
	{
		Log("> Allready tried to initialize but it failed.");
		return false;
	}
	
	ComStatus = COMSTATUS_ERROR;
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		Log("> CoInitialize failed: %X", hr);
		return FALSE;
	}

	hr = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&pServiceProvider);
	if (FAILED(hr))
	{
		Log("> CoCreateInstance failed: %X", hr);
		return FALSE;
	}



	hr = pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager), &pDesktopManager);
	if (FAILED(hr))
	{
		Log("> QueryService(DesktopManager) failed");
		pServiceProvider->Release();
		pServiceProvider = nullptr;
		return FALSE;
	}



	hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, &pDesktopManagerInternal);
	if (FAILED(hr))
	{
		Log("> QueryService(DesktopManagerInternal) failed");
		pDesktopManager->Release();
		pDesktopManager = nullptr;
		pServiceProvider->Release();
		pServiceProvider = nullptr;
		return FALSE;
	}

	ComStatus = COMSTATUS_INITIALIZED;
	return TRUE;
}

VOID FreeCom()
{
	if (ComStatus == COMSTATUS_INITIALIZED)
	{
		pDesktopManager->Release();
		pDesktopManagerInternal->Release();
		pServiceProvider->Release();
		ComStatus = COMSTATUS_UNINITIALIZED;
	}
}

VOID ReadIni()
{
	if (bReadIni == true)
		return;
	bReadIni = true;
	Log("Reading Ini");
	TCHAR iniFile[MAX_PATH] = { 0 };
	ExpandEnvironmentStrings(INIFILE, iniFile, _countof(iniFile));
	bSwitchDesktopAfterMove = (GetPrivateProfileInt("MoveToDesktop", "SwitchDesktopAfterMove", 0, iniFile) != 0);
	Log("Ini: SwitchDesktopAfterMove = %d", (bSwitchDesktopAfterMove ? 1 : 0));
}

INT GetIndexOfItem(HMENU menu, UINT id)
{
	for (int i = GetMenuItemCount(menu) - 1; i >= 0; i--)
	{
		int x = GetMenuItemID(menu, i);
		if (id == x)
		{
			return i;
		}
	}
	return -1;
}

VOID AddMenu(HWND hwnd, HMENU menu)
{
	if (bAddedMenu == true)
		return;
	
	HMENU systemMenu;
	if ((systemMenu = GetSystemMenu(hwnd, FALSE)) == NULL)
	{
		return;
	}


	if (menu != INVALID_HANDLE_VALUE && menu != systemMenu)
	{
		return;
	}

	bAddedMenu = true;

	if (!InitCom())
	{
		return;
	}

	IObjectArray *pObjectArray = nullptr;
	IVirtualDesktop *pCurrentDesktop = nullptr;

	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	if (FAILED(hr))
	{
		return;
	}

	UINT count;
	hr = pObjectArray->GetCount(&count);
	if (FAILED(hr))
	{
		pObjectArray->Release();
		return;
	}


	hr = pDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop);
	if (FAILED(hr))
	{
		pCurrentDesktop = nullptr;
	}



	MENUITEMINFO MoveToItem = { 0 };
	MoveToItem.cbSize = sizeof(MoveToItem);
	MoveToItem.fMask = MIIM_SUBMENU | MIIM_STATE | MIIM_ID | MIIM_STRING;
	Log("Add MoveToMenu");
	MoveToItem.wID = MOVETOMENU_ID;
	MoveToItem.dwTypeData = TEXT("Move &To");
	MoveToItem.hSubMenu = CreateMenu();
	InsertMenuItem(systemMenu, SC_CLOSE, FALSE, &MoveToItem);
	

	for (UINT i = 0; i < count && i < MAXDESKTOPS; ++i)
	{
		IVirtualDesktop *pDesktop = nullptr;

		if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
			continue;

		char desktopName[64] = { 0 };

		sprintf_s(desktopName, sizeof(desktopName), "Desktop &%d", i + 1);

		MENUITEMINFO item = { 0 };
		item.cbSize = sizeof(item);
		item.fMask = MIIM_CHECKMARKS | MIIM_STATE | MIIM_ID | MIIM_STRING;
		item.fState = (pDesktop == pCurrentDesktop) ? MFS_CHECKED : MFS_UNCHECKED;
		item.wID = MOVETOMENU_START + i;
		item.dwTypeData = desktopName;
		InsertMenuItem(MoveToItem.hSubMenu, -1, FALSE, &item);
		pDesktop->Release();
	}

	// Create 'New Desktop' Item
	{
		MENUITEMINFO item = { 0 };
		item.cbSize = sizeof(item);
		item.fMask = MIIM_ID | MIIM_STRING;
		item.fState = MFS_UNCHECKED;
		item.wID = MOVETOMENU_NEW;
		item.dwTypeData = TEXT("&New Desktop");
		InsertMenuItem(MoveToItem.hSubMenu, -1, FALSE, &item);
	}


	pObjectArray->Release();

	if (pCurrentDesktop != nullptr)
	{
		pCurrentDesktop->Release();
	}
}

VOID RemoveMenu(HWND hwnd, HMENU menu)
{
	if (bAddedMenu == false)
		return;
	HMENU systemMenu;
	if ((systemMenu = GetSystemMenu(hwnd, FALSE)) == NULL)
	{
		return;
	}

	if (menu != INVALID_HANDLE_VALUE && menu != systemMenu)
	{
		return;
	}

	bAddedMenu = false;

	MENUITEMINFO MoveToItem = { 0 };
	MoveToItem.cbSize = sizeof(MoveToItem);
	MoveToItem.fMask = MIIM_SUBMENU | MIIM_STATE | MIIM_ID | MIIM_STRING;
	if (GetMenuItemInfo(systemMenu, MOVETOMENU_ID, MF_BYCOMMAND, &MoveToItem) == NULL)
	{
		return;
	}
	Log("Remove MoveToMenu");
	DestroyMenu(MoveToItem.hSubMenu);
	DeleteMenu(systemMenu, MoveToItem.wID, MF_BYCOMMAND);	
}

INT GetCurrentDesktopIndex(UINT *count)
{
	*count = 0;
	IObjectArray *pObjectArray = nullptr;
	IVirtualDesktop *pCurrentDesktop = nullptr;

	if (!InitCom())
	{
		Log("InitCom failed");
		return -1;
	}

	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	if (FAILED(hr))
	{
		Log("pDesktopManagerInternal->GetDesktops failed %x", hr);
		return -1;
	}

	hr = pObjectArray->GetCount(count);
	if (FAILED(hr))
	{
		Log("pObjectArray->GetCount failed %x", hr);
		pObjectArray->Release();
		return -1;
	}


	hr = pDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop);
	if (FAILED(hr))
	{
		Log("pDesktopManagerInternal->GetCurrentDesktop failed %x", hr);
		pObjectArray->Release();
		return -1;
	}

	int index = -1;
	for (UINT i = 0; i < *count && i < MAXDESKTOPS && index == -1; ++i)
	{
		IVirtualDesktop *pDesktop = nullptr;

		if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
			continue;
		if (pDesktop == pCurrentDesktop)
		{
			index = MOVETOMENU_START + i;
		}
		pDesktop->Release();
	}

	pObjectArray->Release();

	if (pCurrentDesktop != nullptr)
	{
		pCurrentDesktop->Release();
	}
	return index;
}

void HandleSysCommand(WPARAM wParam, HWND hwnd)
{
	if (wParam == MOVETOMENU_NEW)
	{
		Log("Getting RootWindow of %X", hwnd);
		HWND rootHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
		if (rootHwnd != NULL)
		{
			hwnd = rootHwnd;
		}

		Log("Moving %X to new", hwnd);
		IVirtualDesktop *pNewDesktop = nullptr;
		HRESULT hr = pDesktopManagerInternal->CreateDesktopW(&pNewDesktop);
		if (FAILED(hr))
		{
			return;
		}
		GUID id;
		hr = pNewDesktop->GetID(&id);
		if (SUCCEEDED(hr))
		{
			Log("pDesktopManager->MoveWindowToDesktop(%X, %X)", hwnd, id);
			hr = pDesktopManager->MoveWindowToDesktop(hwnd, id);
			if (SUCCEEDED(hr))
			{
				if (bSwitchDesktopAfterMove)
				{
					pDesktopManagerInternal->SwitchDesktop(pNewDesktop);
				}
			}
			else
			{
				Log("Error %d on moving %X to %X", hr, hwnd, id);
			}
		}
		pNewDesktop->Release();
	}
	else if (wParam >= MOVETOMENU_START && wParam <= MOVETOMENU_LAST)
	{
		Log("Getting RootWindow of %X", hwnd);
		HWND rootHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
		if (rootHwnd != NULL)
		{
			hwnd = rootHwnd;
		}

		Log("Moving %X to %X", hwnd, wParam);
		IObjectArray *pObjectArray = nullptr;
		HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
		if (FAILED(hr))
		{
			Log("Failed to get desktops for %X", hwnd);
			return;
		}

		IVirtualDesktop *pDesktop = nullptr;
		if (SUCCEEDED(pObjectArray->GetAt((UINT)wParam - MOVETOMENU_START, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
		{
			GUID id;
			hr = pDesktop->GetID(&id);

			if (SUCCEEDED(hr))
			{
				Log("pDesktopManager->MoveWindowToDesktop(%X, %X)", hwnd, id);
				hr = pDesktopManager->MoveWindowToDesktop(hwnd, id);
				if (SUCCEEDED(hr))
				{
					if (bSwitchDesktopAfterMove)
					{
						pDesktopManagerInternal->SwitchDesktop(pDesktop);
					}
				}
				else
				{
					Log("Error %X on moving %X to %X", hr, hwnd, id);
				}
			}
			pDesktop->Release();
		}
		pObjectArray->Release();
	}
	else if (wParam == MOVETOMENU_LEFT)
	{
		UINT count;
		int index = GetCurrentDesktopIndex(&count);
		Log("Current Index is %d", index);
		Log("Current Count is %d", count);
		if (index == -1)
			return;
		if (index == MOVETOMENU_START)
			return;
		Log("Switch to %d", index - 1);
		HandleSysCommand(--index, hwnd);
	}
	else if (wParam == MOVETOMENU_RIGHT)
	{
		UINT count;
		int index = GetCurrentDesktopIndex(&count);
		Log("Current Index is %d", index);
		Log("Current Count is %d", count);
		if (index == -1)
			return;
		if (index == MOVETOMENU_LAST)
			return;
		if ((++index) <= (int)(count + MOVETOMENU_NEW))
		{
			Log("Switch to %d", index);
			HandleSysCommand(index, hwnd);

		}
		else
		{
			Log("Create new desktop");
			HandleSysCommand(MOVETOMENU_NEW, hwnd);
		}
	}
}

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
				Log("WM_SYSCOMMAND %X %X", msg->wParam, msg->hwnd);
				HandleSysCommand(msg->wParam, msg->hwnd);
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
				Log("WM_SYSCOMMAND");
				HandleSysCommand(msg->wParam, msg->hwnd);
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
