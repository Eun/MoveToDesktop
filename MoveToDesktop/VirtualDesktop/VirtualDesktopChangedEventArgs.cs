using System;

namespace WindowsDesktop
{
	/// <summary>
	/// Provides data for the <see cref="VirtualDesktop.CurrentChanged"/> event.
	/// </summary>
	public class VirtualDesktopChangedEventArgs : EventArgs
	{
		public VirtualDesktop OldDesktop { get; }
		public VirtualDesktop NewDesktop { get; }

		public VirtualDesktopChangedEventArgs(VirtualDesktop oldDesktop, VirtualDesktop newDesktop)
		{
			this.OldDesktop = oldDesktop;
			this.NewDesktop = newDesktop;
		}
	}
}
