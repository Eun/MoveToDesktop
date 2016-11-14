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


#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Windows.h>
#include "../hooklib/hook.h"


namespace HookTests
{	
	
	TEST_CLASS(ApiTests)
	{
	
	public:
		
		
		// We cannot test this since windows can only move their own window
		/*
		TEST_METHOD(TestCreateMoveDelete)
		{
			UINT createdDesktopIndex;
			UINT initialDesktopCount;
			UINT index, count;

			// Count the current open desktops
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &initialDesktopCount));

			// Create a Desktop
			Assert::AreEqual(TRUE, CreateDesktop(&createdDesktopIndex));

			// We should have now one more
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &count));
			Assert::AreEqual(initialDesktopCount+1, count);

			// The new desktop should have no windows
			Assert::AreEqual(FALSE, HasDesktopWindows(createdDesktopIndex));

			// Create a window and move it
			ShellExecuteA(NULL, NULL, "winver", NULL, NULL, SW_SHOWDEFAULT);

			Sleep(100);

			HWND testWindowHwnd = FindWindowA(NULL, "About Windows");


			// Move it to the created Desktop
			Assert::AreEqual(TRUE, MoveWindowToDesktop(testWindowHwnd, createdDesktopIndex));

			// There should be one window visible now
			Assert::AreEqual(TRUE, HasDesktopWindows(createdDesktopIndex));
			
			// close the window
			::DestroyWindow(testWindowHwnd);

			// The new desktop should hav eno windows now
			Assert::AreEqual(FALSE, HasDesktopWindows(createdDesktopIndex));

			// Delete the Desktop
			Assert::AreEqual(TRUE, RemoveDesktop(createdDesktopIndex, index));

			// We should be ready now
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &count));
			Assert::AreEqual(initialDesktopCount, count);
		}*/
		TEST_METHOD(TestCreateSwitchDelete)
		{
			UINT createdDesktopIndex;
			UINT initialDesktopCount;
			UINT initialDesktopIndex;
			UINT index, count;

			// Count the current open desktops
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&initialDesktopIndex, &initialDesktopCount));

			// Create a Desktop
			Assert::AreEqual(TRUE, CreateDesktop(&createdDesktopIndex));

			// We should have now one more
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &count));
			Assert::AreEqual(initialDesktopCount + 1, count);

			// Switch to the desktop
			Assert::AreEqual(TRUE, SwitchDesktop(createdDesktopIndex));

			// Are we currently on the new switched desktop?
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &count));
			Assert::AreEqual(createdDesktopIndex, index);

			// Delete the Desktop
			Assert::AreEqual(TRUE, RemoveDesktop(createdDesktopIndex, initialDesktopIndex));

			// We should be ready now
			Assert::AreEqual(TRUE, GetCurrentDesktopIndex(&index, &count));
			Assert::AreEqual(initialDesktopCount, count);
		}
		
		/*TEST_METHOD(CreateDesktopTest)
		{
			UINT index;
			Assert::AreEqual(TRUE, CreateDesktop(&index));
			Assert::AreEqual((UINT)1, index);
		}*/

	};
}