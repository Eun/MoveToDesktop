using System;
using System.Runtime.InteropServices;

namespace WindowsDesktop.Interop
{
	[ComImport]
	[Guid("0cd45e71-d927-4f15-8b0a-8fef525337bf")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	public interface IVirtualDesktopNotificationService
	{
		uint Register(IVirtualDesktopNotification pNotification);

		void Unregister(uint dwCookie);
	}
}
