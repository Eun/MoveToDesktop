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

using System;
using System.Runtime.InteropServices;
using System.Runtime.ConstrainedExecution;
using System.Security;

namespace MoveToDesktop
{
	internal static class Natives
	{
		[DllImport("kernel32.dll")]
		internal static extern IntPtr OpenMutex(uint dwDesiredAccess, bool bInheritHandle, string lpName);

		internal const UInt32 MUTEX_ALL_ACCESS = 0x1F0001;


		[DllImport("kernel32.dll", SetLastError = true)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
		[SuppressUnmanagedCodeSecurity]
		[return: MarshalAs(UnmanagedType.Bool)]
		internal static extern bool CloseHandle(IntPtr hObject);


		[DllImport("kernel32.dll")]
		internal static extern IntPtr GetCurrentProcess();

		[DllImport("kernel32.dll")]
		internal static extern IntPtr GetModuleHandle(string moduleName);

		[DllImport("kernel32")]
		internal static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

		[DllImport("kernel32.dll")]
		internal static extern bool IsWow64Process(IntPtr hProcess, out bool wow64Process);





		[DllImport("user32.dll")]
		internal static extern int GetWindowThreadProcessId(IntPtr hWnd, out IntPtr lpdwProcessId);



	}
}
