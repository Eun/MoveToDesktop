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

	}
}
