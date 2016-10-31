using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace MoveToDesktop
{
	public static class ProcessExtensions
	{
		private static class Win32
		{
			[DllImport("user32.dll", CharSet = CharSet.Auto)]
			public static extern IntPtr SendMessage(IntPtr hWnd, UInt32 Msg, IntPtr wParam, IntPtr lParam);

			[return: MarshalAs(UnmanagedType.Bool)]
			[DllImport("user32.dll", SetLastError = true)]
			public static extern bool PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

			[return: MarshalAs(UnmanagedType.Bool)]
			[DllImport("user32.dll", SetLastError = true)]
			public static extern bool PostThreadMessage(uint threadId, uint msg, IntPtr wParam, IntPtr lParam);

			public delegate bool EnumThreadDelegate(IntPtr hWnd, IntPtr lParam);
			[DllImport("user32.dll")]
			public static extern bool EnumThreadWindows(uint dwThreadId, EnumThreadDelegate lpfn, IntPtr lParam);

			[DllImport("user32.dll", SetLastError = true)]
			public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint lpdwProcessId);
		}

		//Sends a message to the first enumerated window in the first enumerated thread with at least one window, and returns the handle of that window through the hwnd output parameter if such a window was enumerated.  If a window was enumerated, the return value is the return value of the SendMessage call, otherwise the return value is zero.
		public static IntPtr SendMessage(this Process p, out IntPtr hwnd, UInt32 msg, IntPtr wParam, IntPtr lParam)
		{
			hwnd = p.WindowHandles().FirstOrDefault();
			if (hwnd != IntPtr.Zero)
				return Win32.SendMessage(hwnd, msg, wParam, lParam);
			else
				return IntPtr.Zero;
		}

		//Posts a message to the first enumerated window in the first enumerated thread with at least one window, and returns the handle of that window through the hwnd output parameter if such a window was enumerated.  If a window was enumerated, the return value is the return value of the PostMessage call, otherwise the return value is false.
		public static bool PostMessage(this Process p, out IntPtr hwnd, UInt32 msg, IntPtr wParam, IntPtr lParam)
		{
			hwnd = p.WindowHandles().FirstOrDefault();
			if (hwnd != IntPtr.Zero)
				return Win32.PostMessage(hwnd, msg, wParam, lParam);
			else
				return false;
		}

		//Posts a thread message to the first enumerated thread (when ensureTargetThreadHasWindow is false), or posts a thread message to the first enumerated thread with a window, unless no windows are found in which case the call fails.  If an appropriate thread was found, the return value is the return value of PostThreadMessage call, otherwise the return value is false.
		public static bool PostThreadMessage(this Process p, UInt32 msg, IntPtr wParam, IntPtr lParam, bool ensureTargetThreadHasWindow = true)
		{
			uint targetThreadId = 0;
			if (ensureTargetThreadHasWindow)
			{
				IntPtr hwnd = p.WindowHandles().FirstOrDefault();
				uint processId = 0;
				if (hwnd != IntPtr.Zero)
					targetThreadId = Win32.GetWindowThreadProcessId(hwnd, out processId);
			}
			else
			{
				targetThreadId = (uint)p.Threads[0].Id;
			}
			if (targetThreadId != 0)
				return Win32.PostThreadMessage(targetThreadId, msg, wParam, lParam);
			else
				return false;
		}

		public static IEnumerable<IntPtr> WindowHandles(this Process process)
		{
			var handles = new List<IntPtr>();
			foreach (ProcessThread thread in process.Threads)
				Win32.EnumThreadWindows((uint)thread.Id, (hWnd, lParam) => { handles.Add(hWnd); return true; }, IntPtr.Zero);
			return handles;
		}
	}
}
