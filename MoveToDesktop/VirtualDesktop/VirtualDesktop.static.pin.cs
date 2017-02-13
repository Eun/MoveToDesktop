using System;
using System.Collections.Generic;
using System.Linq;
using WindowsDesktop.Interop;

namespace WindowsDesktop
{
	partial class VirtualDesktop
	{
		public static bool IsPinnedWindow(IntPtr hWnd)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			return ComObjects.VirtualDesktopPinnedApps.IsViewPinned(hWnd.GetApplicationView());
		}

		public static void PinWindow(IntPtr hWnd)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			var view = hWnd.GetApplicationView();

			if (!ComObjects.VirtualDesktopPinnedApps.IsViewPinned(view))
			{
				ComObjects.VirtualDesktopPinnedApps.PinView(view);
			}
		}

		public static void UnpinWindow(IntPtr hWnd)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			var view = hWnd.GetApplicationView();

			if (ComObjects.VirtualDesktopPinnedApps.IsViewPinned(view))
			{
				ComObjects.VirtualDesktopPinnedApps.UnpinView(view);
			}
		}

		public static bool IsPinnedApplication(string appId)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			return ComObjects.VirtualDesktopPinnedApps.IsAppIdPinned(appId);
		}

		public static void PinApplication(string appId)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			if (!ComObjects.VirtualDesktopPinnedApps.IsAppIdPinned(appId))
			{
				ComObjects.VirtualDesktopPinnedApps.PinAppID(appId);
			}
		}

		public static void UnpinApplication(string appId)
		{
			VirtualDesktopHelper.ThrowIfNotSupported();

			if (ComObjects.VirtualDesktopPinnedApps.IsAppIdPinned(appId))
			{
				ComObjects.VirtualDesktopPinnedApps.UnpinAppID(appId);
			}
		}
	}
}
