using System;
using System.Runtime.InteropServices;

namespace WindowsDesktop.Interop
{
	[ComImport]
	[Guid("4ce81583-1e4c-4632-a621-07a53543148f")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	public interface IVirtualDesktopPinnedApps
	{
		bool IsAppIdPinned(string appId);

		void PinAppID(string appId);

		void UnpinAppID(string appId);

		bool IsViewPinned(IApplicationView applicationView);

		void PinView(IApplicationView applicationView);

		void UnpinView(IApplicationView applicationView);
	}
}
