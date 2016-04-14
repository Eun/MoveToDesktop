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
#include <Windows.h>

struct KeyMapping
{
	PTCHAR Name;
	INT	Value;
};

static struct KeyMapping keyModifiers[] = {
	{ "WIN", MOD_WIN },
	{ "ALT", MOD_ALT },
	{ "CTRL", MOD_CONTROL },
	{ "SHIFT", MOD_SHIFT },
};
INT nKeyModifiersSize = sizeof(keyModifiers) / sizeof(keyModifiers[0]);


static struct KeyMapping keyKeys[] = {
	{ "LMOUSE", VK_LBUTTON },
	{ "RMOUSE", VK_RBUTTON},
	{ "MMOUSE", VK_MBUTTON },
	{ "XMOUSE1", VK_XBUTTON1 },
	{ "XMOUSE2", VK_XBUTTON2 },
	{ "BACK", VK_BACK },
	{ "TAB", VK_TAB },
	{ "CLEAR", VK_CLEAR },
	{ "RETURN", VK_RETURN },
	{ "PAUSE", VK_PAUSE },
	{ "ESCAPE", VK_ESCAPE },
	{ "SPACE", VK_SPACE },
	{ "PRIOR", VK_PRIOR },
	{ "NEXT", VK_NEXT },
	{ "END", VK_END },
	{ "HOME", VK_HOME },
	{ "LEFT", VK_LEFT },
	{ "UP", VK_UP },
	{ "RIGHT", VK_RIGHT },
	{ "DOWN", VK_DOWN },
	{ "SELECT", VK_SELECT },
	{ "PRINT", VK_PRINT },
	{ "EXECUTE", VK_EXECUTE },
	{ "SNAPSHOT", VK_SNAPSHOT },
	{ "INSERT", VK_INSERT },
	{ "DELETE", VK_DELETE },
	{ "HELP", VK_HELP },
	{ "NUMPAD0", VK_NUMPAD0 },
	{ "NUMPAD1", VK_NUMPAD1 },
	{ "NUMPAD2", VK_NUMPAD2 },
	{ "NUMPAD3", VK_NUMPAD3 },
	{ "NUMPAD4", VK_NUMPAD4 },
	{ "NUMPAD5", VK_NUMPAD5 },
	{ "NUMPAD6", VK_NUMPAD6 },
	{ "NUMPAD7", VK_NUMPAD7 },
	{ "NUMPAD8", VK_NUMPAD8 },
	{ "NUMPAD9", VK_NUMPAD9 },
	{ "MULTIPLY", VK_MULTIPLY },
	{ "ADD", VK_ADD },
	{ "SEPARATOR", VK_SEPARATOR },
	{ "SUBTRACT", VK_SUBTRACT },
	{ "DECIMAL", VK_DECIMAL },
	{ "DIVIDE", VK_DIVIDE },
	{ "F1", VK_F1 },
	{ "F2", VK_F2 },
	{ "F3", VK_F3 },
	{ "F4", VK_F4 },
	{ "F5", VK_F5 },
	{ "F6", VK_F6 },
	{ "F7", VK_F7 },
	{ "F8", VK_F8 },
	{ "F9", VK_F9 },
	{ "F10", VK_F10 },
	{ "F11", VK_F11 },
	{ "F12", VK_F12 },
	{ "F13", VK_F13 },
	{ "F14", VK_F14 },
	{ "F15", VK_F15 },
	{ "F16", VK_F16 },
	{ "F17", VK_F17 },
	{ "F18", VK_F18 },
	{ "F19", VK_F19 },
	{ "F20", VK_F20 },
	{ "F21", VK_F21 },
	{ "F22", VK_F22 },
	{ "F23", VK_F23 },
	{ "F24", VK_F24 },
	{ "0", 0x30 },
	{ "1", 0x31 },
	{ "2", 0x32 },
	{ "3", 0x33 },
	{ "4", 0x34 },
	{ "5", 0x35 },
	{ "6", 0x36 },
	{ "7", 0x37 },
	{ "8", 0x38 },
	{ "9", 0x39 },
	{ "A", 0x41 },
	{ "B", 0x42 },
	{ "C", 0x43 },
	{ "D", 0x44 },
	{ "E", 0x45 },
	{ "F", 0x46 },
	{ "G", 0x47 },
	{ "H", 0x48 },
	{ "I", 0x49 },
	{ "J", 0x4A },
	{ "K", 0x4B },
	{ "L", 0x4C },
	{ "M", 0x4D },
	{ "N", 0x4E },
	{ "O", 0x4F },
	{ "P", 0x50 },
	{ "Q", 0x51 },
	{ "R", 0x52 },
	{ "S", 0x53 },
	{ "T", 0x54 },
	{ "U", 0x55 },
	{ "V", 0x56 },
	{ "W", 0x57 },
	{ "X", 0x58 },
	{ "Y", 0x59 },
	{ "Z", 0x5A },
};
INT nKeyKeysSize = sizeof(keyKeys) / sizeof(keyKeys[0]);