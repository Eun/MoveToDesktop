using System;
using System.Collections.Generic;
using System.Linq;
using WindowsDesktop.Interop;

namespace WindowsDesktop
{
	public static class ApplicationHelper
	{
		internal static IApplicationView GetApplicationView(this IntPtr hWnd)
		{
			IApplicationView view;
			ComObjects.ApplicationViewCollection.GetViewForHwnd(hWnd, out view);

			return view;
		}

		public static string GetAppId(IntPtr hWnd)
		{
			string appId;
			hWnd.GetApplicationView().GetAppUserModelId(out appId);

			return appId;
		}
	}
}
