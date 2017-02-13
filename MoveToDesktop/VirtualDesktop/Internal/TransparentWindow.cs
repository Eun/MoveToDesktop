using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Interop;

namespace WindowsDesktop.Internal
{
	internal class TransparentWindow : RawWindow
	{
		public override void Show()
		{
			var parameters = new HwndSourceParameters(this.Name)
			{
				Width = 1,
				Height = 1,
				WindowStyle = 0x800000,
			};

			this.Show(parameters);
		}
	}
}
