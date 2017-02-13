using System;

namespace WindowsDesktop
{
	/// <summary>
	/// Provides data for the <see cref="VirtualDesktop.DestroyBegin"/> and <see cref="VirtualDesktop.DestroyFailed"/>, <see cref="VirtualDesktop.Destroyed"/> event.
	/// </summary>
	public class VirtualDesktopDestroyEventArgs : EventArgs
	{
		/// <summary>
		/// Gets which virtual desktop was destroyed.
		/// </summary>
		public VirtualDesktop Destroyed { get; }

		/// <summary>
		/// Gets the virtual desktop displayed after <see cref="Destroyed"/> is destroyed.
		/// </summary>
		public VirtualDesktop Fallback { get; }

		public VirtualDesktopDestroyEventArgs(VirtualDesktop destroyed, VirtualDesktop fallback)
		{
			this.Destroyed = destroyed;
			this.Fallback = fallback;
		}
	}
}
