/**
* MoveToDesktop
*
* Copyright (C) 2015-2016 by Tobias Salzmann
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
// compile with cl.exe close_all.cpp
// description: Closes all virtual desktops
#include <Windows.h>
#include <stdio.h>
#include "../Hook/VirtualDesktops.h"

#pragma comment (lib, "ole32.lib")

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


BOOL InitCom()
{
	if (ComStatus == COMSTATUS_INITIALIZED)
		return true;
	else if (ComStatus == COMSTATUS_ERROR)
		return false;
	
	ComStatus = COMSTATUS_ERROR;
	::CoInitialize(NULL);

	HRESULT hr = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&pServiceProvider);

	if (FAILED(hr))
	{
		return FALSE;
	}



	hr = pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager), &pDesktopManager);
	if (FAILED(hr))
	{

		pServiceProvider->Release();
		pServiceProvider = nullptr;
		return FALSE;
	}



	hr = pServiceProvider->QueryService(CLSID_VirtualDesktopAPI_Unknown, &pDesktopManagerInternal);
	if (FAILED(hr))
	{

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

void main()
{
	InitCom();


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



	for (UINT i = 0; i < count; ++i)
	{
		IVirtualDesktop *pDesktop = nullptr;

		if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
			continue;

		if (pDesktop != pCurrentDesktop)
		{
			printf("Closing %d\n",i);
			pDesktopManagerInternal->RemoveDesktop(pDesktop, pCurrentDesktop);
		}
		pDesktop->Release();
	}

	pObjectArray->Release();

	FreeCom();
}