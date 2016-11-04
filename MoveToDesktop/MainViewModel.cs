using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Security.Policy;
using System.Security.Principal;

using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using Microsoft.Win32.TaskScheduler;
using Newtonsoft.Json.Linq;


namespace MoveToDesktop
{
	class MainViewModel : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;

		protected virtual void OnPropertyChanged(string propertyName)
		{
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
		}


		public MainViewModel()
		{
			InstallTaskCommand = new RelayCommand(o =>
			{
				InstallTask();
				OnPropertyChanged(nameof(IsTaskInstalled));			
			});
			RemoveTaskCommand = new RelayCommand(o =>
			{
				RemoveTask();
				OnPropertyChanged(nameof(IsTaskInstalled));
			});

			RestartRunnerCommand = new RelayCommand(o =>
			{
				Settings.HotKeyMoveLeft = BuildKey(HotKeyLeft);
				Settings.HotKeyMoveRight = BuildKey(HotKeyRight);
				Settings.HotKeyMoveAndSwitchLeft = BuildKey(HotKeyLeftSwitch);
				Settings.HotKeyMoveAndSwitchRight = BuildKey(HotKeyRightSwitch);
				Settings.HotKeySwitchDesktop1 = BuildKey(HotKeySwitchDesktop1);
				Settings.HotKeySwitchDesktop2 = BuildKey(HotKeySwitchDesktop2);
				Settings.HotKeySwitchDesktop3 = BuildKey(HotKeySwitchDesktop3);

				HotKeyLeft = ExtractKey(Settings.HotKeyMoveLeft);
				HotKeyRight = ExtractKey(Settings.HotKeyMoveRight);
				HotKeyLeftSwitch = ExtractKey(Settings.HotKeyMoveAndSwitchLeft);
				HotKeyRightSwitch = ExtractKey(Settings.HotKeyMoveAndSwitchRight);
				HotKeySwitchDesktop1 = ExtractKey(Settings.HotKeySwitchDesktop1);
				HotKeySwitchDesktop2 = ExtractKey(Settings.HotKeySwitchDesktop2);
				HotKeySwitchDesktop3 = ExtractKey(Settings.HotKeySwitchDesktop3);
				OnPropertyChanged(nameof(HotKeyLeft));
				OnPropertyChanged(nameof(HotKeyRight));
				OnPropertyChanged(nameof(HotKeyLeftSwitch));
				OnPropertyChanged(nameof(HotKeyRightSwitch));
				OnPropertyChanged(nameof(HotKeySwitchDesktop1));
				OnPropertyChanged(nameof(HotKeySwitchDesktop2));
				OnPropertyChanged(nameof(HotKeySwitchDesktop3));

				RunHelper.Exit();
				RunHelper.Start();
			});


			HotKeyLeft = ExtractKey(Settings.HotKeyMoveLeft);
			HotKeyRight = ExtractKey(Settings.HotKeyMoveRight);
			HotKeyLeftSwitch = ExtractKey(Settings.HotKeyMoveAndSwitchLeft);
			HotKeyRightSwitch = ExtractKey(Settings.HotKeyMoveAndSwitchRight);
			HotKeySwitchDesktop1 = ExtractKey(Settings.HotKeySwitchDesktop1);
			HotKeySwitchDesktop2 = ExtractKey(Settings.HotKeySwitchDesktop2);
			HotKeySwitchDesktop3 = ExtractKey(Settings.HotKeySwitchDesktop3);
			OnPropertyChanged(nameof(HotKeyLeft));
			OnPropertyChanged(nameof(HotKeyRight));
			OnPropertyChanged(nameof(HotKeyLeftSwitch));
			OnPropertyChanged(nameof(HotKeyRightSwitch));
			OnPropertyChanged(nameof(HotKeySwitchDesktop1));
			OnPropertyChanged(nameof(HotKeySwitchDesktop2));
			OnPropertyChanged(nameof(HotKeySwitchDesktop3));
		}

		public static string Version
		{
			get
			{
				var version = Assembly.GetExecutingAssembly().GetName().Version;
				return $"{version.Major}.{version.Minor}";
			}
		}
		public static string Title => $"MoveToDesktop {Version}";

		public bool? SwitchDesktopAfterMove
		{
			get
			{
				return Settings.SwitchDesktopAfterMove;
			}
			set
			{
				if (value.HasValue)
					Settings.SwitchDesktopAfterMove = value.Value;
			}
		}
		public bool? CreateNewDesktopOnMove
		{
			get
			{
				return Settings.CreateNewDesktopOnMove;
			}
			set
			{
				if (value.HasValue)
					Settings.CreateNewDesktopOnMove = value.Value;
			}
		}
		public bool? DeleteEmptyDesktops
		{
			get
			{
				return Settings.DeleteEmptyDesktops;
			}
			set
			{
				if (value.HasValue)
					Settings.DeleteEmptyDesktops = value.Value;
			}
		}
		public bool? HideTray
		{
			get
			{
				return Settings.HideTray;
			}
			set {
				if (value.HasValue)
					Settings.HideTray = value.Value;
			}
		}

		public static ListCollectionView Keys
		{
			get
			{
				var l = new List<KeyItem>();
				l.Add(new KeyItem()
				{
					Category = "",
					Name = "None",
				});
				foreach (var key in MoveToDesktop.Keys.Modifiers)
				{
					l.Add(new KeyItem()
					{
						Category = "Modifiers",
						Name = key,
					});
				}
				foreach (var key in MoveToDesktop.Keys.BaseKeys)
				{
					l.Add(new KeyItem()
					{
						Category = "Keys",
						Name = key,
					});
				}

				var collection = new ListCollectionView(l);
				collection.GroupDescriptions.Add(new PropertyGroupDescription("Category"));
				return collection;

			}
		}

		private static KeyItem GetKey(string key)
		{
			int offset;
			return GetKey(key, out offset);
		}
		private static KeyItem GetKey(string key, out int offset)
		{
			for (int i = 0; i < Keys.SourceCollection.Cast<KeyItem>().Count(); i++)
			{
				var item = Keys.SourceCollection.Cast<KeyItem>().ElementAt(i);
				if (string.Compare(item.Name, key, StringComparison.InvariantCultureIgnoreCase) == 0)
				{
					offset = i;
					return item;
				}
			}
			offset = -1;
			return null;
		}
		private static KeyItem GetKey(int offset)
		{
			return Keys.SourceCollection.Cast<KeyItem>().ElementAtOrDefault(offset);
		}

		private static int[] ExtractKey(string hotkey)
		{
			var keyCodes = new List<int>();
			var keys = hotkey.Split(new char[] { '+' }, StringSplitOptions.RemoveEmptyEntries);
			for (int i = 0; i < keys.Length; i++)
			{
				int result;
				if (GetKey(keys[i], out result) != null)
				{
					keyCodes.Add(result);
				}
			}

			int noneKey;
			GetKey("None", out noneKey);
			while (keyCodes.Count < 4)
			{
				keyCodes.Add(noneKey);
			}

			return keyCodes.ToArray();
		}

		private static string BuildKey(params int[] keyparts)
		{
			var keys = new List<KeyItem>();

			for (int i = 0; i < keyparts.Count(); i++)
			{
				var key = GetKey(keyparts[i]);
				if (key != null)
					keys.Add(key);
			}

			// remove modifiers after keys
			bool hasKey = false;
			for (int i = 0; i < keys.Count; i++)
			{
				if (hasKey && keys[i].Category == "Modifiers")
				{
					keys[i] = GetKey("None");
				}
				hasKey |= (keys[i].Category == "Keys");
			}

			keys.RemoveAll(x => x.Name.Equals("None"));
			
			// if the last key is a modifier
			if (keys.Count > 0)
			{
				if (GetKey(keys[keys.Count - 1].Name).Category == "Modifiers")
				{
					keys.Clear();
				}
			}

			return string.Join("+", keys.Select(x=> x.Name));
		}

		public int[] HotKeyLeft { get; set; }


		public int[] HotKeyRight { get; set; }


		public int[] HotKeyLeftSwitch { get; set; }


		public int[] HotKeyRightSwitch { get; set; }

		public int[] HotKeySwitchDesktop1 { get; set; }
		public int[] HotKeySwitchDesktop2 { get; set; }
		public int[] HotKeySwitchDesktop3 { get; set; }



		public ICommand RestartRunnerCommand { get; private set; }

		public ICommand OpenConfig { get; private set; } = new RelayCommand(o =>
		{
			Process.Start(new ProcessStartInfo()
			{
				FileName = "explorer.exe",
				Arguments = $"/select,\"{Settings.ConfigFile}\"",
				UseShellExecute = true,
			});
		});

		public ICommand CheckForUpdatesCommand { get; private set; } = new RelayCommand(o =>
		{
			CheckForUpdates();
		});

		private static System.Threading.Tasks.Task updateTask = null;

		public static void CheckForUpdates()
		{
			if (updateTask == null)
			{
				updateTask = new System.Threading.Tasks.Task(() =>
				{
					try
					{
						var updateChecker = new GithubUpdateChecker("Eun", "MoveToDesktop");
						var info = updateChecker.GetLatestVersion();

						if (info.Version > Assembly.GetExecutingAssembly().GetName().Version)
						{
							if (MessageBox.Show($"There is a new Version available!\n\nName:    {info.Name}\nVersion: {info.Version}\n\nDo you want to download it?", Title, MessageBoxButton.YesNoCancel, MessageBoxImage.Information) == MessageBoxResult.Yes)
							{
								Process.Start(new ProcessStartInfo(info.DownloadUrl));
							}
						}
					}
					finally
					{
						Settings.LastUpdateCheck = DateTime.UtcNow;
						updateTask = null;
					}
				});

				updateTask.Start();
			}
		}


		private static readonly string taskName = "MoveToDesktop";
		private static readonly string taskDescription = "Start MoveToDesktop as Administrator for all users";

		public static Task Task
		{
			get
			{
				using (TaskService ts = new TaskService())
				{
					return ts.FindTask(taskName, false);
				}
			}
		}

		public bool IsTaskInstalled
		{
			get { return Task != null; }
		}

		public static bool IsAdministrator
		{
			get
			{
				WindowsIdentity identity = WindowsIdentity.GetCurrent();
				WindowsPrincipal principal = new WindowsPrincipal(identity);
				return principal.IsInRole(WindowsBuiltInRole.Administrator);
			}
		}

		public ICommand InstallTaskCommand { get; private set; }
		public ICommand RemoveTaskCommand { get; private set; }

		public static void InstallTask()
		{ 
			if (!IsAdministrator)
			{
				if (Process.Start(new ProcessStartInfo()
				{
					FileName = Assembly.GetExecutingAssembly().Location,
					UseShellExecute = true,
					Verb = "runas",
					Arguments = "--install-task --show-ui",
				}) != null)
				{
					Application.Current.Shutdown();
				}
				return;
			}
			using (TaskService ts = new TaskService())
			{
				TaskDefinition td = ts.NewTask();
				td.RegistrationInfo.Description = taskDescription;

				td.Triggers.Add(new LogonTrigger());

				
				td.Actions.Add(new ExecAction(Assembly.GetExecutingAssembly().Location));

				//td.Settings.RunOnlyIfLoggedOn = true;
				td.Principal.RunLevel = TaskRunLevel.Highest;

				ts.RootFolder.RegisterTaskDefinition(taskName, td);

				// Force the view
				Settings.FirstTime = true;
				Task.Run();
				Application.Current.Shutdown();
			}
		}

		public static void RemoveTask() { 
			if (!IsAdministrator)
			{
				if (Process.Start(new ProcessStartInfo()
				{
					FileName = Assembly.GetExecutingAssembly().Location,
					UseShellExecute = true,
					Verb = "runas",
					Arguments = "--remove-task --show-ui",
				}) != null)
				{
					Application.Current.Shutdown();
				}
				return;
			}
			using (TaskService ts = new TaskService())
			{
				try
				{
					ts.RootFolder.DeleteTask(taskName);
				}
				catch
				{
					
				}
			}
		}

	}
}
