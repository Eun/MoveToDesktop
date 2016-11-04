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
