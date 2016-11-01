using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace MoveToDesktop
{

	internal class Settings
	{
		[DllImport("kernel32.dll")]
		private static extern uint GetPrivateProfileInt(string lpAppName, string lpKeyName, int nDefault, string lpFileName);

		[DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
		private static extern uint GetPrivateProfileString(string lpAppName, string lpKeyName, string lpDefault, StringBuilder lpReturnedString, uint nSize, string lpFileName);

		[DllImport("kernel32.dll")]
		private static extern uint WritePrivateProfileString(string lpAppName, string lpKeyName, string lpString, string lpFileName);

		private const string _inifile = "%APPDATA%\\MoveToDesktop.ini";

		private static string _configFile;



		static Settings()
		{
			_configFile = Environment.ExpandEnvironmentVariables(_inifile);
		}



		#region General Settings
		public static bool SwitchDesktopAfterMove
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "SwitchDesktopAfterMove", 0, _configFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "SwitchDesktopAfterMove", value ? "1" : "0", _configFile); }
		}

		public static bool CreateNewDesktopOnMove
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "CreateNewDesktopOnMove", 1, _configFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "CreateNewDesktopOnMove", value ? "1" : "0", _configFile); }
		}

		public static bool DeleteEmptyDesktops
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "DeleteEmptyDesktops", 0, _configFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "DeleteEmptyDesktops", value ? "1" : "0", _configFile); }
		}
		#endregion


		#region HotKeys

		private static readonly string _moveLeft = "WIN+ALT+LEFT";
		private static readonly string _moveRight = "WIN+ALT+RIGHT";

		public static string MoveLeft
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return _moveLeft;
				}
				StringBuilder result = new StringBuilder(128);
				if (GetPrivateProfileString("HotKeys", "MoveLeft", _gui_mutex, result, 128, _configFile) != 0)
				{
					return result.ToString();
				}
				return _moveLeft;
			}
			set { WritePrivateProfileString("HotKeys", "MoveLeft", value, _configFile); }
		}

		public static string MoveRight
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return _moveRight;
				}
				StringBuilder result = new StringBuilder(128);
				if (GetPrivateProfileString("HotKeys", "MoveRight", _gui_mutex, result, 128, _configFile) != 0)
				{
					return result.ToString();
				}
				return _moveRight;
			}
			set { WritePrivateProfileString("HotKeys", "MoveRight", value, _configFile); }
		}
		#endregion

		#region Advanced Settings


		private static readonly string _mutex_x86 = "{4EF85FA7-55CB-4BD9-AD73-15EDA3BB149C}";
		private static readonly string _mutex_x64 = "{92B297B9-7430-4BB0-B77B-EB6D36DCF8F2}";

		public static string MutexX86
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return _mutex_x86;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Advanced", "Mutex_x86", _mutex_x86, result, 40, _configFile) != 0)
				{
					return result.ToString();
				}
				return _mutex_x86;
			}
		}

		public static string MutexX64
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return _mutex_x64;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Advanced", "Mutex_x64", _mutex_x64, result, 40, _configFile) != 0)
				{
					return result.ToString();
				}
				return _mutex_x64;
			}
		}
		#endregion

		#region GUI Settings
		public static bool HideTray
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("Gui", "HideTray", 0, _configFile) != 0);
			}
			set { WritePrivateProfileString("Gui", "HideTray", value ? "1" : "0", _configFile); }
		}


		private static readonly string _gui_mutex = "{84D50373-C6EE-4AAD-AED9-6462FAA64B02}";

		public static string GuiMutex
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return _gui_mutex;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Gui", "Mutex", _gui_mutex, result, 40, _configFile) != 0)
				{
					return result.ToString();
				}
				return _gui_mutex;
			}
		}

		public static bool FirstTime
		{
			get
			{
				if (!File.Exists(_configFile))
				{
					return true;
				}
				return (GetPrivateProfileInt("Gui", "FirstTime", 1, _configFile) != 0);
			}
			set { WritePrivateProfileString("Gui", "FirstTime", value ? "1" : "0", _configFile); }
		}
		#endregion

	}
}
