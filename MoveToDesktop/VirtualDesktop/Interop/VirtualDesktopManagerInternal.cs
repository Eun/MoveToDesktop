using System;

namespace WindowsDesktop.Interop
{
	internal class VirtualDesktopManagerInternal
		: IVirtualDesktopManagerInternal10130
		, IVirtualDesktopManagerInternal10240
		, IVirtualDesktopManagerInternal14328
	{
		private IVirtualDesktopManagerInternal10130 _manager10130;
		private IVirtualDesktopManagerInternal10240 _manager10240;
		private IVirtualDesktopManagerInternal14328 _manager14328;

		public static VirtualDesktopManagerInternal GetInstance()
		{
			var v14328 = GetInstanceCore<IVirtualDesktopManagerInternal14328>();
			if (v14328 != null) return new VirtualDesktopManagerInternal() { _manager14328 = v14328, };

			var v10240 = GetInstanceCore<IVirtualDesktopManagerInternal10240>();
			if (v10240 != null) return new VirtualDesktopManagerInternal() { _manager10240 = v10240, };

			var v10130 = GetInstanceCore<IVirtualDesktopManagerInternal10130>();
			if (v10130 != null) return new VirtualDesktopManagerInternal() { _manager10130 = v10130, };

			throw new NotSupportedException();
		}

		private static T GetInstanceCore<T>()
		{
			var shellType = Type.GetTypeFromCLSID(CLSID.ImmersiveShell);
			var shell = (IServiceProvider)Activator.CreateInstance(shellType);

			object ppvObject;
			shell.QueryService(CLSID.VirtualDesktopAPIUnknown, typeof(T).GUID, out ppvObject);

			return (T)ppvObject;
		}

		public int GetCount()
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.GetCount();
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.GetCount();
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.GetCount();
			}

			throw new NotSupportedException();
		}


		public void MoveViewToDesktop(IApplicationView pView, IVirtualDesktop desktop)
		{
			if (this._manager14328 != null)
			{
				this._manager14328?.MoveViewToDesktop(pView, desktop);
				return;
			}

			if (this._manager10240 != null)
			{
				this._manager10240?.MoveViewToDesktop(pView, desktop);
				return;
			}

			if (this._manager10130 != null)
			{
				this._manager10130.MoveViewToDesktop(pView, desktop);
				return;
			}

			throw new NotSupportedException();
		}

		public bool CanViewMoveDesktops(IApplicationView pView)
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.CanViewMoveDesktops(pView);
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.CanViewMoveDesktops(pView);
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.CanViewMoveDesktops(pView);
			}

			throw new NotSupportedException();
		}

		public IVirtualDesktop GetCurrentDesktop()
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.GetCurrentDesktop();
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.GetCurrentDesktop();
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.GetCurrentDesktop();
			}

			throw new NotSupportedException();
		}

		public IObjectArray GetDesktops()
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.GetDesktops();
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.GetDesktops();
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.GetDesktops();
			}

			throw new NotSupportedException();
		}

		public IVirtualDesktop GetAdjacentDesktop(IVirtualDesktop pDesktopReference, AdjacentDesktop uDirection)
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.GetAdjacentDesktop(pDesktopReference, uDirection);
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.GetAdjacentDesktop(pDesktopReference, uDirection);
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.GetAdjacentDesktop(pDesktopReference, uDirection);
			}

			throw new NotSupportedException();
		}

		public void SwitchDesktop(IVirtualDesktop desktop)
		{
			if (this._manager14328 != null)
			{
				this._manager14328?.SwitchDesktop(desktop);
				return;
			}

			if (this._manager10240 != null)
			{
				this._manager10240?.SwitchDesktop(desktop);
				return;
			}

			if (this._manager10130 != null)
			{
				this._manager10130.SwitchDesktop(desktop);
				return;
			}

			throw new NotSupportedException();
		}

		public IVirtualDesktop CreateDesktopW()
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.CreateDesktopW();
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.CreateDesktopW();
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.CreateDesktopW();
			}

			throw new NotSupportedException();
		}

		public void RemoveDesktop(IVirtualDesktop pRemove, IVirtualDesktop pFallbackDesktop)
		{
			if (this._manager14328 != null)
			{
				this._manager14328.RemoveDesktop(pRemove, pFallbackDesktop);
				return;
			}

			if (this._manager10240 != null)
			{
				this._manager10240.RemoveDesktop(pRemove, pFallbackDesktop);
				return;
			}

			if (this._manager10130 != null)
			{
				this._manager10130.RemoveDesktop(pRemove, pFallbackDesktop);
				return;
			}

			throw new NotSupportedException();
		}

		public IVirtualDesktop FindDesktop(ref Guid desktopId)
		{
			if (this._manager14328 != null)
			{
				return this._manager14328.FindDesktop(ref desktopId);
			}

			if (this._manager10240 != null)
			{
				return this._manager10240.FindDesktop(ref desktopId);
			}

			if (this._manager10130 != null)
			{
				return this._manager10130.FindDesktop(ref desktopId);
			}

			throw new NotSupportedException();
		}
	}
}
