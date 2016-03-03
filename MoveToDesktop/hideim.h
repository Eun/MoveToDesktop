/**
* HideImports 1.0
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
#pragma once
#include <Windows.h>

#define c(x) char((x) - 32)
#define un(x) char((x) + 32)

static inline HMODULE GetModuleHandleEncrypted(const char* szName)
{
	char szFunc[128] = { '\0' };
	for (int i = 0; *szName; i++)
		szFunc[i] = un(*szName++);

	return GetModuleHandleA(szFunc);
}



static inline FARPROC GetProcAddressEncrypted(HMODULE hModule, const char* szName)
{
	char szFunc[128] = { '\0' };
	for (int i = 0; *szName; i++)
		szFunc[i] = un(*szName++);

	return GetProcAddress(hModule, szFunc);
}

const char szUser32[] = { c('u'),c('s'),c('e'),c('r'),c('3'),c('2') , 0 };
const char szSetWindowsHookExA[] = { c('S'),c('e'),c('t'),c('W'),c('i'),c('n'),c('d'),c('o'),c('w'),c('s'),c('H'),c('o'),c('o'),c('k'),c('E'),c('x'),c('A'), 0 };
const char szUnhookWindowsHookEx[] = { c('U'),c('n'),c('h'),c('o'),c('o'),c('k'),c('W'),c('i'),c('n'),c('d'),c('o'),c('w'),c('s'),c('H'),c('o'),c('o'),c('k'),c('E'),c('x'), 0 };

typedef HHOOK(WINAPI* PSetWindowsHookExA)(int, HOOKPROC, HINSTANCE, DWORD);
PSetWindowsHookExA pfSetWindowsHookExA = (PSetWindowsHookExA)GetProcAddressEncrypted(GetModuleHandleEncrypted(szUser32), szSetWindowsHookExA);
#define SetWindowsHookExA pfSetWindowsHookExA

typedef BOOL(WINAPI* PUnhookWindowsHookEx)(HHOOK);
PUnhookWindowsHookEx pfUnhookWindowsHookEx = (PUnhookWindowsHookEx)GetProcAddressEncrypted(GetModuleHandleEncrypted(szUser32), szUnhookWindowsHookEx);
#define UnhookWindowsHookEx pfUnhookWindowsHookEx