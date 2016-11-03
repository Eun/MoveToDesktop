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
#include "../shared.h"
#include <time.h>



IServiceProvider* pServiceProvider = nullptr;
IVirtualDesktopManager *pDesktopManager = nullptr;
IVirtualDesktopManagerInternal* pDesktopManagerInternal = nullptr;

#define EXPORT __declspec(dllexport)

enum EComStatus
{
	COMSTATUS_UNINITIALIZED,
	COMSTATUS_INITIALIZED,
	COMSTATUS_ERROR,
};

int ComStatus = COMSTATUS_UNINITIALIZED;
#define COMMAND_TIMEOUT 500 // Blocks Command below this timeout
bool bAddedMenu = false;
bool bReadIni = false;
bool bSwitchDesktopAfterMove = false;
bool bCreateNewDesktopOnMove = false;
bool bDeleteEmptyDesktops = true;
ULONGLONG nLastCommand = 0;

UINT16 GetWinBuildNumber()
{
	UINT16 buildNumbers[] = { 10130, 10240, 14393 };
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0,{ 0 }, 0, 0 };
	ULONGLONG mask = ::VerSetConditionMask(0, VER_BUILDNUMBER, VER_EQUAL);



	for (size_t i = 0; i < sizeof(buildNumbers) / sizeof(buildNumbers[0]); i++)
	{
		osvi.dwBuildNumber = buildNumbers[i];
		if (VerifyVersionInfoW(&osvi, VER_BUILDNUMBER, mask) != FALSE)
		{
			return buildNumbers[i];
		}
	}
	
	return 0;
}

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


	UINT16 buildNumber = GetWinBuildNumber();

	switch (buildNumber)
	{
		case 10130:
			hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, UUID_IVirtualDesktopManagerInternal_10130, (void**)&pDesktopManagerInternal);
			break;
		case 10240:
			hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, UUID_IVirtualDesktopManagerInternal_10240, (void**)&pDesktopManagerInternal);
			break;
		case 14393:
		default:
			hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, UUID_IVirtualDesktopManagerInternal_14393, (void**)&pDesktopManagerInternal);
			break;
	}
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
		Log("Releasing Com");
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
	bCreateNewDesktopOnMove = (GetPrivateProfileInt("MoveToDesktop", "CreateNewDesktopOnMove", 1, iniFile) != 0);
	Log("Ini: CreateNewDesktopOnMove = %d", (bCreateNewDesktopOnMove ? 1 : 0));
	bDeleteEmptyDesktops = (GetPrivateProfileInt("MoveToDesktop", "DeleteEmptyDesktops", 0, iniFile) != 0);
	Log("Ini: DeleteEmptyDesktops = %d", (bDeleteEmptyDesktops ? 1 : 0));
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


// Taken from http://www.dfcd.net/projects/switcher/switcher.c
BOOL IsAltTabWindow(HWND hwnd)
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	if (!IsWindowVisible(hwnd))
		return FALSE;

	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
	while (hwndTry != hwndWalk)
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup(hwndWalk);
		if (IsWindowVisible(hwndTry))
			break;
	}
	if (hwndWalk != hwnd)
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	// Tool windows should not be displayed either, these do not appear in the
	// task bar.
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	// Also remove all windows without a title
	if (GetWindowTextLength(hwnd) == 0)
		return FALSE;


	return TRUE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {

	Log("EnumWindowsProc: Checking %x", hwnd);
	if (IsAltTabWindow(hwnd))
	{
		GUID desktopId;
		if (SUCCEEDED(pDesktopManager->GetWindowDesktopId(hwnd, &desktopId)))
		{
			if (desktopId == *(GUID*)lParam)
			{
				CHAR buffer[1024] = "";
				GetWindowText(hwnd, buffer, 1024);
				Log("EnumWindowsProc: %x - %s is visible", hwnd, buffer);
				// stop enumeration
				return FALSE;
			}
		}
	}
	// window does not belong to us or is not a "valid window"
	// continue with the next window
	return TRUE;
}

BOOL GetDesktop(IVirtualDesktop** pDesktop, UINT index)
{
	BOOL status = FALSE;
	IObjectArray *pObjectArray;
	if (SUCCEEDED(pDesktopManagerInternal->GetDesktops(&pObjectArray)))
	{
		UINT count;
		if (SUCCEEDED(pObjectArray->GetCount(&count)) && count >= index + 1)
		{
			status = SUCCEEDED(pObjectArray->GetAt(index, __uuidof(IVirtualDesktop), (void**)pDesktop));
		}
		pObjectArray->Release();
	}
	return status;
}


EXPORT BOOL GetCurrentDesktopIndex(PUINT index, PUINT count)
{
	IObjectArray *pObjectArray;
	IVirtualDesktop *pCurrentDesktop;

	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}

	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	if (FAILED(hr))
	{
		Log("pDesktopManagerInternal->GetDesktops failed %x", hr);
		return FALSE;
	}

	hr = pObjectArray->GetCount(count);
	if (FAILED(hr))
	{
		Log("pObjectArray->GetCount failed %x", hr);
		pObjectArray->Release();
		return FALSE;
	}


	hr = pDesktopManagerInternal->GetCurrentDesktop(&pCurrentDesktop);
	if (FAILED(hr))
	{
		Log("pDesktopManagerInternal->GetCurrentDesktop failed %x", hr);
		pObjectArray->Release();
		return FALSE;
	}

	*index = -1;
	for (UINT i = 0; i < *count && i < MAXDESKTOPS && *index == -1; ++i)
	{
		IVirtualDesktop *pDesktop = nullptr;

		if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
			continue;
		if (pDesktop == pCurrentDesktop)
		{
			*index = i;
		}
		pDesktop->Release();
	}

	pObjectArray->Release();

	if (pCurrentDesktop != nullptr)
	{
		pCurrentDesktop->Release();
	}
	return TRUE;
}

EXPORT BOOL SwitchDesktop(UINT index)
{
	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}
	BOOL status = FALSE;
	IVirtualDesktop *pDesktop;
	if (GetDesktop(&pDesktop, index))
	{
		HRESULT hr = pDesktopManagerInternal->SwitchDesktop(pDesktop);
		Log("pDesktopManagerInternal->SwitchDesktop(%X) => %X", pDesktop, hr);
		status = SUCCEEDED(hr);
		pDesktop->Release();
	}
	return status;
}


HWND GetRoot(HWND hwnd)
{
	Log("Getting RootWindow of %X", hwnd);
	HWND rootHwnd = GetAncestor(hwnd, GA_ROOTOWNER);
	if (rootHwnd != NULL)
	{
		Log("Root of %X is %X", hwnd, rootHwnd);
		hwnd = rootHwnd;
	}
	return hwnd;
}

BOOL MoveWindowToDesktop(HWND hwnd, IVirtualDesktop* pDesktop)
{
	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}
	BOOL status = FALSE;
	GUID id;
	if (SUCCEEDED(pDesktop->GetID(&id)))
	{
		HRESULT hr = pDesktopManager->MoveWindowToDesktop(hwnd, id);
		Log("pDesktopManager->MoveWindowToDesktop(%X, %X) => %X", hwnd, id, hr);
		status = SUCCEEDED(hr);
	}
	return status;
}

EXPORT BOOL MoveWindowToDesktop(HWND hwnd, UINT index)
{
	hwnd = GetRoot(hwnd);
	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}
	BOOL status = FALSE;
	IVirtualDesktop *pDesktop;
	if (GetDesktop(&pDesktop, index))
	{
		status = MoveWindowToDesktop(hwnd, pDesktop);
		pDesktop->Release();
	}
	return status;
}

#undef CreateDesktop
BOOL CreateDesktop(IVirtualDesktop **pCreatedDesktop)
{
	return SUCCEEDED(pDesktopManagerInternal->CreateDesktopW(pCreatedDesktop));
}

EXPORT BOOL CreateDesktop(PUINT index)
{
	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}
	BOOL status = FALSE;
	IVirtualDesktop *pCreatedDesktop;
	if (CreateDesktop(&pCreatedDesktop))
	{
		GUID createdId;
		if (SUCCEEDED(pCreatedDesktop->GetID(&createdId)))
		{
			IObjectArray *pObjectArray;
			if (SUCCEEDED(pDesktopManagerInternal->GetDesktops(&pObjectArray)))
			{
				UINT count;
				if (SUCCEEDED(pObjectArray->GetCount(&count)) && count > 0)
				{
					for (UINT i = 0; i < count && status == FALSE; i++)
					{
						IVirtualDesktop *pDesktop;
						if (SUCCEEDED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
						{
							if (pCreatedDesktop == pDesktop)
							{
								*index = i;
								status = TRUE;
							}
							pDesktop->Release();
						}
					}

				}
				pObjectArray->Release();
			}
		}
		pCreatedDesktop->Release();
	}
	return status;
}
EXPORT BOOL RemoveDesktop(UINT index, UINT fallbackIndex)
{
	if (!InitCom())
	{
		Log("InitCom failed");
		return FALSE;
	}
	BOOL status = FALSE;
	IVirtualDesktop *pDesktop;
	if (!GetDesktop(&pDesktop, index))
	{
		return status;
	}

	IVirtualDesktop *pFallbackDesktop;
	if (!GetDesktop(&pFallbackDesktop, fallbackIndex))
	{
		pDesktop->Release();
		return status;
	}


	if (SUCCEEDED(pDesktopManagerInternal->RemoveDesktop(pDesktop, pFallbackDesktop)))
	{
		status = TRUE;
	}

	pDesktop->Release();
	pFallbackDesktop->Release();
	return status;
}

BOOL HasDesktopWindows(IVirtualDesktop* pDesktop)
{
	GUID id;
	if (SUCCEEDED(pDesktop->GetID(&id)))
	{
		return ((EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&id)) == FALSE);
	}
	else
	{
		// better be safe than sorry
		return TRUE;
	}
}

EXPORT BOOL HasDesktopWindows(UINT index)
{
	if (!InitCom())
	{
		Log("InitCom failed");
		return TRUE;
	}
	BOOL status = TRUE; // better be safe than sorry
	IVirtualDesktop *pDesktop;
	if (!GetDesktop(&pDesktop, index))
	{
		return status;
	}
	status = HasDesktopWindows(pDesktop);
	pDesktop->Release();
	return status;
}




BOOL GetCurrentDesktop(IVirtualDesktop** pDesktop)
{
	return SUCCEEDED(pDesktopManagerInternal->GetCurrentDesktop(pDesktop));
}

BOOL MoveWindowToDesktop(HWND hwnd, UINT index, bool switchTo)
{
	hwnd = GetRoot(hwnd);
	BOOL status = FALSE;
	HWND focusHwnd = NULL;
	if (!switchTo)
	{
		focusHwnd = hwnd;
		do
		{
			focusHwnd = GetNextWindow(focusHwnd, GW_HWNDNEXT);
		} while (focusHwnd && !IsAltTabWindow(focusHwnd));
	}

	IVirtualDesktop *pDesktop;

	if (index == -1)
	{
		Log("Moving %X to new", hwnd);
		// create new
		if (!CreateDesktop(&pDesktop))
		{
			return status;
		}
	}
	else
	{
		Log("Moving %X to %X", hwnd, index);
		// move to existant
		if (!GetDesktop(&pDesktop, index))
		{
			return status;
		}
	}

	if (MoveWindowToDesktop(hwnd, pDesktop))
	{
		// If there are no windows delete the desktop
		/*if (bDeleteEmptyDesktops)
		{
			if (!HasDesktopWindows(pDesktop))
			{
				IVirtualDesktop *pCurrentDekstop;
				if (GetCurrentDesktop(&pCurrentDekstop))
				{
					pDesktopManagerInternal->RemoveDesktop(pDesktop, pCurrentDekstop);
					pCurrentDekstop->Release;
				}
			}
		}*/

		if (switchTo)
		{
			pDesktopManagerInternal->SwitchDesktop(pDesktop);
		}
		else if (focusHwnd != NULL)
		{
			SetForegroundWindow(focusHwnd);
		}
		status = true;
	}
	pDesktop->Release();
	return status;
}

BOOL ShouldExecuteCommand()
{
	// abort command, too many commands in a short period of time
	if (nLastCommand > GetTickCount64())
	{
		Log("ShouldExecuteCommand => No");
		return FALSE;
	}
	nLastCommand = GetTickCount64() + COMMAND_TIMEOUT;
	Log("ShouldExecuteCommand => Yes");
	return TRUE;
}

VOID HandleSysCommand(WPARAM wParam, HWND hwnd)
{
	if (wParam == MOVETOMENU_NEW)
	{
		if (!ShouldExecuteCommand())
		{
			return;
		}
		MoveWindowToDesktop(hwnd, -1, bSwitchDesktopAfterMove);
	}
	else if (wParam >= MOVETOMENU_START && wParam <= MOVETOMENU_LAST)
	{
		if (!ShouldExecuteCommand())
		{
			return;
		}
		MoveWindowToDesktop(hwnd, (UINT)wParam - MOVETOMENU_START, bSwitchDesktopAfterMove);
	}
	// Hotkeys
	else if (wParam == MOVETOMENU_LEFT || wParam == MOVETOMENU_LEFT_SWITCH)
	{
		if (!ShouldExecuteCommand())
		{
			return;
		}
		UINT index;
		UINT count;
		if (GetCurrentDesktopIndex(&index, &count))
		{
			Log("Current Index is %d", index);
			Log("Current Count is %d", count);
			if (index == 0)
				return;
			Log("Switch to %d", index - 1);
			MoveWindowToDesktop(hwnd, --index, wParam == MOVETOMENU_LEFT_SWITCH);
		}
	}
	else if (wParam == MOVETOMENU_RIGHT || wParam == MOVETOMENU_RIGHT_SWITCH)
	{
		if (!ShouldExecuteCommand())
		{
			return;
		}
		UINT index;
		UINT count;
		if (GetCurrentDesktopIndex(&index, &count))
		{
			Log("Current Index is %d", index);
			Log("Current Count is %d", count);
			if (index == -1)
				return;
			if (index >= MAXDESKTOPS)
				return;
			if (++index <= count)
			{
				Log("Switch to %d", index);
				MoveWindowToDesktop(hwnd, index, false);

			}
			else if (bCreateNewDesktopOnMove)
			{
				Log("Create new desktop");
				MoveWindowToDesktop(hwnd, -1, wParam == MOVETOMENU_RIGHT_SWITCH);
			}
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
