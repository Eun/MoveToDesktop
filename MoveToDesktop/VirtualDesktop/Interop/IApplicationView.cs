using System;
using System.Runtime.InteropServices;

namespace WindowsDesktop.Interop
{
	[ComImport]
	[Guid("9ac0b5c8-1484-4c5b-9533-4134a0f97cea")]
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	public interface IApplicationView
	{
		int SetFocus();

		int SwitchTo();

		int TryInvokeBack(IntPtr /* IAsyncCallback* */ callback);

		int GetThumbnailWindow(out IntPtr hwnd);

		int GetMonitor(out IntPtr /* IImmersiveMonitor */ immersiveMonitor);

		int GetVisibility(out int visibility);

		int SetCloak(APPLICATION_VIEW_CLOAK_TYPE cloakType, int unknown);

		int GetPosition(ref Guid guid /* GUID for IApplicationViewPosition */, out IntPtr /* IApplicationViewPosition** */ position);

		int SetPosition(ref IntPtr /* IApplicationViewPosition* */ position);

		int InsertAfterWindow(IntPtr hwnd);

		int GetExtendedFramePosition(out Rect rect);

		int GetAppUserModelId([MarshalAs(UnmanagedType.LPWStr)] out string id);

		int SetAppUserModelId(string id);

		int IsEqualByAppUserModelId(string id, out int result);

		int GetViewState(out uint state);

		int SetViewState(uint state);

		int GetNeediness(out int neediness);

		int GetLastActivationTimestamp(out ulong timestamp);

		int SetLastActivationTimestamp(ulong timestamp);

		int GetVirtualDesktopId(out Guid guid);

		int SetVirtualDesktopId(ref Guid guid);

		int GetShowInSwitchers(out int flag);

		int SetShowInSwitchers(int flag);

		int GetScaleFactor(out int factor);

		int CanReceiveInput(out bool canReceiveInput);

		int GetCompatibilityPolicyType(out APPLICATION_VIEW_COMPATIBILITY_POLICY flags);

		int SetCompatibilityPolicyType(APPLICATION_VIEW_COMPATIBILITY_POLICY flags);

		int GetPositionPriority(out IntPtr /* IShellPositionerPriority** */ priority);

		int SetPositionPriority(IntPtr /* IShellPositionerPriority* */ priority);

		int GetSizeConstraints(IntPtr /* IImmersiveMonitor* */ monitor, out Size size1, out Size size2);

		int GetSizeConstraintsForDpi(uint uint1, out Size size1, out Size size2);

		int SetSizeConstraintsForDpi(ref uint uint1, ref Size size1, ref Size size2);

		int QuerySizeConstraintsFromApp();

		int OnMinSizePreferencesUpdated(IntPtr hwnd);

		int ApplyOperation(IntPtr /* IApplicationViewOperation* */ operation);

		int IsTray(out bool isTray);

		int IsInHighZOrderBand(out bool isInHighZOrderBand);

		int IsSplashScreenPresented(out bool isSplashScreenPresented);

		int Flash();

		int GetRootSwitchableOwner(out IApplicationView rootSwitchableOwner);

		int EnumerateOwnershipTree(out IObjectArray ownershipTree);

		/*** (Windows 10 Build 10584 or later?) ***/

		int GetEnterpriseId([MarshalAs(UnmanagedType.LPWStr)] out string enterpriseId);

		int IsMirrored(out bool isMirrored);
	}


	[StructLayout(LayoutKind.Sequential)]
	public struct Size
	{
		public int X;
		public int Y;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct Rect
	{
		public int Left;
		public int Top;
		public int Right;
		public int Bottom;
	}

	public enum APPLICATION_VIEW_CLOAK_TYPE : int
	{
		AVCT_NONE = 0,
		AVCT_DEFAULT = 1,
		AVCT_VIRTUAL_DESKTOP = 2
	}

	public enum APPLICATION_VIEW_COMPATIBILITY_POLICY : int
	{
		AVCP_NONE = 0,
		AVCP_SMALL_SCREEN = 1,
		AVCP_TABLET_SMALL_SCREEN = 2,
		AVCP_VERY_SMALL_SCREEN = 3,
		AVCP_HIGH_SCALE_FACTOR = 4
	}
}
