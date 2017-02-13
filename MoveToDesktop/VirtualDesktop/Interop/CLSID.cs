using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
// ReSharper disable InconsistentNaming

namespace WindowsDesktop.Interop
{
	public static class CLSID
	{
		public static Guid ImmersiveShell { get; } = new Guid("c2f03a33-21f5-47fa-b4bb-156362a2f239");

		public static Guid VirtualDesktopManager { get; } = new Guid("aa509086-5ca9-4c25-8f95-589d3c07b48a");

		public static Guid VirtualDesktopAPIUnknown { get; } = new Guid("c5e0cdca-7b6e-41b2-9fc4-d93975cc467b");

		public static Guid VirtualDesktopNotificationService { get; } = new Guid("a501fdec-4a09-464c-ae4e-1b9c21b84918");

		public static Guid VirtualDesktopPinnedApps { get; } = new Guid("b5a399e7-1c87-46b8-88e9-fc5747b171bd");
	}
}
