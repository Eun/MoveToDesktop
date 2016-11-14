/**
* MoveToDesktop
*
* Copyright (C) 2015-2016 by Tobias Salzmann
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.Globalization;
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

		public static string ConfigFile { get; private set; }



		static Settings()
		{
			ConfigFile = Environment.ExpandEnvironmentVariables(_inifile);
		}



		#region General Settings

		public static bool SwitchDesktopAfterMove
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "SwitchDesktopAfterMove", 0, ConfigFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "SwitchDesktopAfterMove", value ? "1" : "0", ConfigFile); }
		}

		public static bool CreateNewDesktopOnMove
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "CreateNewDesktopOnMove", 1, ConfigFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "CreateNewDesktopOnMove", value ? "1" : "0", ConfigFile); }
		}

		public static bool DeleteEmptyDesktops
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("MoveToDesktop", "DeleteEmptyDesktops", 0, ConfigFile) != 0);
			}
			set { WritePrivateProfileString("MoveToDesktop", "DeleteEmptyDesktops", value ? "1" : "0", ConfigFile); }
		}

		#endregion


		#region HotKeys



		private static string GetHotKey(string name, string defaultValue)
		{
			if (!File.Exists(ConfigFile))
			{
				return defaultValue;
			}
			StringBuilder result = new StringBuilder(128);
			if (GetPrivateProfileString("HotKeys", name, defaultValue, result, 128, ConfigFile) != 0)
			{
				return result.ToString();
			}
			return defaultValue;
		}

		private static void SetHotHey(string name, string value)
		{
			WritePrivateProfileString("HotKeys", name, value, ConfigFile);
		}


		public static string HotKeyMoveLeft
		{
			get
			{
				return GetHotKey("MoveLeft", "WIN+ALT+LEFT");
			}
			set { SetHotHey("MoveLeft", value); }
		}

		public static string HotKeyMoveRight
		{
			get
			{
				return GetHotKey("MoveRight", "WIN+ALT+RIGHT");
			}
			set { SetHotHey("MoveRight", value); }
		}

		public static string HotKeyMoveAndSwitchLeft
		{
			get
			{
				return GetHotKey("MoveAndSwitchLeft", "");
			}
			set { SetHotHey("MoveAndSwitchLeft", value); }
		}

		public static string HotKeyMoveAndSwitchRight
		{
			get
			{
				return GetHotKey("MoveAndSwitchRight", "");
			}
			set { SetHotHey("MoveAndSwitchRight", value); }
		}

		public static string HotKeySwitchDesktop1
		{
			get
			{
				return GetHotKey("HotKeySwitchDesktop1", "WIN+ALT+F1");
			}
			set { SetHotHey("HotKeySwitchDesktop1", value); }
		}

		public static string HotKeySwitchDesktop2
		{
			get
			{
				return GetHotKey("HotKeySwitchDesktop2", "WIN+ALT+F2");
			}
			set { SetHotHey("HotKeySwitchDesktop2", value); }
		}

		public static string HotKeySwitchDesktop3
		{
			get
			{
				return GetHotKey("HotKeySwitchDesktop3", "WIN+ALT+F3");
			}
			set { SetHotHey("HotKeySwitchDesktop3", value); }
		}

		#endregion

		#region Advanced Settings


		private static readonly string _mutex_x86 = "{4EF85FA7-55CB-4BD9-AD73-15EDA3BB149C}";
		private static readonly string _mutex_x64 = "{92B297B9-7430-4BB0-B77B-EB6D36DCF8F2}";

		public static string MutexX86
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return _mutex_x86;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Advanced", "Mutex_x86", _mutex_x86, result, 40, ConfigFile) != 0)
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
				if (!File.Exists(ConfigFile))
				{
					return _mutex_x64;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Advanced", "Mutex_x64", _mutex_x64, result, 40, ConfigFile) != 0)
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
				if (!File.Exists(ConfigFile))
				{
					return false;
				}
				return (GetPrivateProfileInt("Gui", "HideTray", 0, ConfigFile) != 0);
			}
			set { WritePrivateProfileString("Gui", "HideTray", value ? "1" : "0", ConfigFile); }
		}


		private static readonly string _gui_mutex = "{84D50373-C6EE-4AAD-AED9-6462FAA64B02}";

		public static string GuiMutex
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return _gui_mutex;
				}
				StringBuilder result = new StringBuilder(40);
				if (GetPrivateProfileString("Gui", "Mutex", _gui_mutex, result, 40, ConfigFile) != 0)
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
				if (!File.Exists(ConfigFile))
				{
					return true;
				}
				return (GetPrivateProfileInt("Gui", "FirstTime", 1, ConfigFile) != 0);
			}
			set { WritePrivateProfileString("Gui", "FirstTime", value ? "1" : "0", ConfigFile); }
		}

		public static DateTime LastUpdateCheck
		{
			get
			{
				if (!File.Exists(ConfigFile))
				{
					return DateTime.MinValue;
				}
				StringBuilder result = new StringBuilder(128);
				if (GetPrivateProfileString("Gui", "LastUpdateCheck", DateTime.MinValue.ToString("O"), result, 128, ConfigFile) != 0)
				{
					return DateTime.ParseExact(result.ToString(), "O", CultureInfo.InvariantCulture);
				}
				return DateTime.MinValue;

			}
			set { WritePrivateProfileString("Gui", "LastUpdateCheck", value.ToString("O"), ConfigFile); }

			#endregion

		}
	}
}
