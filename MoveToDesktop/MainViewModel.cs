using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Security.Principal;

using System.Text;
using System.Windows;
using System.Windows.Data;
using System.Windows.Input;
using Microsoft.Win32.TaskScheduler;


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
		}


		public string Title
		{
			get
			{
				var version = Assembly.GetEntryAssembly().GetName().Version;
				return $"MoveToDesktop {version.Major}.{version.Minor}";
			}
		}

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

		public ListCollectionView Keys
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

		private KeyItem GetKey(string key)
		{
			int offset;
			return GetKey(key, out offset);
		}
		private KeyItem GetKey(string key, out int offset)
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


		private int ExtractKey(string key, int offset)
		{
			var keys = key.Split(new char[] { '+' }, StringSplitOptions.RemoveEmptyEntries);
			if (keys.Length > offset)
			{
				int result;
				if (GetKey(keys[offset], out result) != null)
				{
					return result;
				}
			}
			return 0;
		}

		private string BuildKey(string hotkey, int key, int offset)
		{
			var keys = new List<string>(hotkey.Split(new char[] {'+'}, StringSplitOptions.RemoveEmptyEntries));
			if (keys.Count > offset)
			{
				keys[offset] = Keys.SourceCollection.Cast<KeyItem>().ElementAt(key).Name;
			}
			else
			{
				keys.Add(Keys.SourceCollection.Cast<KeyItem>().ElementAt(key).Name);
			}
			keys.RemoveAll(x => x.Equals("None"));

			bool hasKey = false;
			for (int i = 0; i < keys.Count; i++)
			{
				var cat = GetKey(keys[i]).Category;

				if (hasKey && cat == "Modifiers")
				{
					keys[i] = "None";
				}

				hasKey |= (cat == "Keys");
				
			}

			keys.RemoveAll(x => x.Equals("None"));

			if (GetKey(keys[keys.Count - 1]).Category == "Modifiers")
			{
				keys.Clear();
			}

			return string.Join("+", keys);
		}

		public int HotKeyLeft1
		{
			get
			{
				return ExtractKey(Settings.MoveLeft, 0);
			}
			set { Settings.MoveLeft = BuildKey(Settings.MoveLeft, value, 0); }
		}

		public int HotKeyLeft2
		{
			get
			{
				return ExtractKey(Settings.MoveLeft, 1);
			}
			set { Settings.MoveLeft = BuildKey(Settings.MoveLeft, value, 1); }
		}

		public int HotKeyLeft3
		{
			get
			{
				return ExtractKey(Settings.MoveLeft, 2);
			}
			set { Settings.MoveLeft = BuildKey(Settings.MoveLeft, value, 2); }
		}

		public int HotKeyLeft4
		{
			get
			{
				return ExtractKey(Settings.MoveLeft, 3);
			}
			set { Settings.MoveLeft = BuildKey(Settings.MoveLeft, value, 3); }
		}

		public int HotKeyRight1
		{
			get
			{
				return ExtractKey(Settings.MoveRight, 0);
			}
			set { Settings.MoveRight = BuildKey(Settings.MoveRight, value, 0); }
		}

		public int HotKeyRight2
		{
			get
			{
				return ExtractKey(Settings.MoveRight, 1);
			}
			set { Settings.MoveRight = BuildKey(Settings.MoveRight, value, 1); }
		}

		public int HotKeyRight3
		{
			get
			{
				return ExtractKey(Settings.MoveRight, 2);
			}
			set { Settings.MoveRight = BuildKey(Settings.MoveRight, value, 2); }
		}

		public int HotKeyRight4
		{
			get
			{
				return ExtractKey(Settings.MoveRight, 3);
			}
			set { Settings.MoveRight = BuildKey(Settings.MoveRight, value, 3); }
		}

		public ICommand RestartRunnerCommand { get; internal set; } = new RelayCommand(o =>
		{
			RunHelper.Exit();
			RunHelper.Start();
		});


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

				// Create an action that will launch Notepad whenever the trigger fires
				td.Actions.Add(new ExecAction(Assembly.GetExecutingAssembly().Location));

				//td.Settings.RunOnlyIfLoggedOn = true;
				td.Principal.RunLevel = TaskRunLevel.Highest;

				// Register the task in the root folder
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
