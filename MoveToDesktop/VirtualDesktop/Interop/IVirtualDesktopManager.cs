using System;
using System.Runtime.InteropServices;

namespace WindowsDesktop.Interop
{
	[ComImport]
	[Guid("a5cd92ff-29be-454c-8d04-d82879fb3f1b")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	public interface IVirtualDesktopManager
	{
		bool IsWindowOnCurrentVirtualDesktop(IntPtr topLevelWindow);

		Guid GetWindowDesktopId(IntPtr topLevelWindow);

		void MoveWindowToDesktop(IntPtr topLevelWindow, ref Guid desktopId);
	}

	// see also: 
	//  "C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\um\ShObjIdl.h"
	//  https://msdn.microsoft.com/en-us/library/windows/desktop/mt186440%28v=vs.85%29.aspx
	//  http://www.cyberforum.ru/blogs/105416/blog3671.html
}