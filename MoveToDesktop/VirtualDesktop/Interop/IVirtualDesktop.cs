using System;
using System.Runtime.InteropServices;

namespace WindowsDesktop.Interop
{
	[ComImport]
	[Guid("ff72ffdd-be7e-43fc-9c03-ad81681e88e4")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	public interface IVirtualDesktop
	{
		bool IsViewVisible(object pView);

		Guid GetID();
	}
}
