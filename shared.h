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

#define MAXDESKTOPS 255
#define MOVETOMENU_ID (0xDE100)
#define MOVETOMENU_NEW (MOVETOMENU_ID + 1)
#define MOVETOMENU_START (MOVETOMENU_NEW + 1)
#define MOVETOMENU_LAST (MOVETOMENU_START + MAXDESKTOPS)
#define MOVETOMENU_LEFT (MOVETOMENU_LAST + 1)
#define MOVETOMENU_RIGHT (MOVETOMENU_LEFT + 1)
#define MOVETOMENU_LEFT_SWITCH (MOVETOMENU_RIGHT + 1)
#define MOVETOMENU_RIGHT_SWITCH (MOVETOMENU_LEFT_SWITCH + 1)
#define SWITCH_TO_DESKTOP1 (MOVETOMENU_LEFT_SWITCH + 1)
#define SWITCH_TO_DESKTOP2 (SWITCH_TO_DESKTOP1 + 1)
#define SWITCH_TO_DESKTOP3 (SWITCH_TO_DESKTOP2 + 1)
#define SWITCH_TO_DESKTOP4 (SWITCH_TO_DESKTOP3 + 1)
#define SWITCH_TO_DESKTOP5 (SWITCH_TO_DESKTOP4 + 1)
#define SWITCH_TO_DESKTOP6 (SWITCH_TO_DESKTOP5 + 1)
#define SWITCH_TO_DESKTOP7 (SWITCH_TO_DESKTOP6 + 1)
#define SWITCH_TO_DESKTOP8 (SWITCH_TO_DESKTOP7 + 1)
#define SWITCH_TO_DESKTOP9 (SWITCH_TO_DESKTOP8 + 1)
#define SWITCH_TO_DESKTOP10 (SWITCH_TO_DESKTOP9 + 1)
#define SWITCH_TO_DESKTOP11 (SWITCH_TO_DESKTOP10 + 1)
#define SWITCH_TO_DESKTOP12 (SWITCH_TO_DESKTOP11 + 1)

#define INIFILE "%APPDATA%\\MoveToDesktop.ini"
#define MAX_HOTKEY_SIZE 128

#ifdef _DEBUG
#define LOGFILE "%temp%\\MoveToDesktop.log"
#include <stdarg.h>
inline void Log(char *message, ...)
{
	FILE *file;

	TCHAR logFile[MAX_PATH] = { 0 };
	ExpandEnvironmentStrings(LOGFILE, logFile, _countof(logFile));
	fopen_s(&file, logFile, "a");

	if (file == NULL) {
		return;
	}
	else
	{
		char exepath[MAX_PATH];
		char inpmsg[1024] = "";
		char finalmsg[1024 + MAX_PATH] = "";
		va_list ptr;
		va_start(ptr, message);
		vsprintf_s(inpmsg, sizeof(inpmsg), message, ptr);
		va_end(ptr);
		GetModuleFileName(0, exepath, MAX_PATH);

		sprintf_s(finalmsg, sizeof(finalmsg), "%s: %s\n", exepath, inpmsg);
		fputs(finalmsg, file);
		fclose(file);
	}

	if (file)
		fclose(file);
}
#else
#define Log
#endif