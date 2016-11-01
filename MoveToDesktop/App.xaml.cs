using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Interop;
using System.Windows.Media.Imaging;
using System.Windows.Shell;
using Mono.Options;
using Application = System.Windows.Application;
using MessageBox = System.Windows.MessageBox;

namespace MoveToDesktop
{
	/// <summary>
	/// Interaction logic for App.xaml
	/// </summary>
	public partial class App : Application
	{
		private Window mainWindow;
		private Mutex mutex;
		private NotifyIcon notifyIcon;

		private const int HWND_BROADCAST = 0xffff;
		private static readonly int WM_SHOWME = RegisterWindowMessage("WM_SHOWME");
		[DllImport("user32")]
		private static extern bool PostMessage(IntPtr hwnd, int msg, IntPtr wparam, IntPtr lparam);
		[DllImport("user32")]
		private static extern int RegisterWindowMessage(string message);



		private class Command
		{
			public enum Type
			{
				NormalStartup,
				ShowStartup,
				InstallTask,
				RemoveTask,
				ShowHelp,
			}

			public string Argument { get; private set; }

			public Type Id { get; private set; }

			public Command(Type id, string argument = null)
			{
				Id = id;
				Argument = argument;
			}
		}

		public App()
		{
#if DEBUG
			Debugger.Launch();
#endif
			Command command = new Command(Command.Type.NormalStartup);
			var p = new OptionSet()
			{
				$"Usage: {Assembly.GetExecutingAssembly().GetName().Name} [command]",
				"",
				"Commands:",
				{
					"install-task", "Install the scheduled task", v =>
					{
						if (v != null)
						{
							command = new Command(Command.Type.InstallTask);
						}
					}
				},
				{
					"remove-task", "Remove the scheduled task", v =>
					{
						if (v != null)
						{
							command = new Command(Command.Type.RemoveTask);
						}
					}
				},
				{
					"show-ui", "Show UI", v =>
					{
						if (v != null)
						{
							command = new Command(Command.Type.ShowStartup);
						}
					}
				},
				{
					"h|help", "show this message and exit", v =>
					{
						if (v != null)
						{
							command = new Command(Command.Type.ShowHelp);
						}
					}
				}
			};

			List<string> extra;
			try
			{
				extra = p.Parse(Environment.GetCommandLineArgs());
			}
			catch
			{
			}

			if (command.Id == Command.Type.ShowHelp)
			{
				p.WriteOptionDescriptions(Console.Out);
				Application.Current.Shutdown();
				return;
			}


			if (command.Id == Command.Type.InstallTask || command.Id == Command.Type.RemoveTask)
			{
				// are we administrator
				if (!MainViewModel.IsAdministrator)
				{
					Console.Out.WriteLine("This command must be run as administrator");
					Application.Current.Shutdown();
					return;

				}
			}
			if (command.Id == Command.Type.InstallTask || command.Id == Command.Type.RemoveTask || command.Id == Command.Type.ShowStartup)
			{
				// wait until the mutex is free
				do
				{
					Thread.Sleep(100);
					mutex = new Mutex(false, Settings.GuiMutex);
					if (mutex.WaitOne(0, false))
					{
						break;
					}
				} while (_contentLoaded);
			}
			else
			{
				mutex = new Mutex(false, Settings.GuiMutex);
				if (!mutex.WaitOne(0, false))
				{
					PostMessage((IntPtr) HWND_BROADCAST, WM_SHOWME, IntPtr.Zero, IntPtr.Zero);
					Application.Current.Shutdown();
					return;
				}
			}


			Application.Current.Exit += (sender, args) =>
			{
				RunHelper.Exit();
				mutex.ReleaseMutex();
				
			};

			try
			{
				RunHelper.Start();
			}
			catch (Exception e)
			{
				MessageBox.Show($"Could not extract runner!\n\n{e.Message}", "MoveToDesktop", MessageBoxButton.OK, MessageBoxImage.Error);
				Application.Current.Shutdown();
				return;
			}


			mainWindow = new MainWindow();

			mainWindow.SourceInitialized += (sender, args) =>
			{
				HwndSource source = PresentationSource.FromVisual(mainWindow) as HwndSource;
				source?.AddHook(WndProc);
			};

			mainWindow.StateChanged += (sender, args) =>
			{
				if (mainWindow.WindowState == WindowState.Minimized)
				{
					notifyIcon.Visible = !Settings.HideTray;
					mainWindow.Hide();
				}
			};

			mainWindow.Closed += (sender, args) =>
			{
				notifyIcon.Visible = false;
				Application.Current.Shutdown();
			};



			notifyIcon = new System.Windows.Forms.NotifyIcon();
			notifyIcon.Icon = MoveToDesktop.Properties.Resources.icon;
			notifyIcon.Text = "MoveToDesktop is running";
			notifyIcon.Click += (sender, args) =>
			{
				ShowWindow();
			};

			notifyIcon.Visible = !Settings.HideTray;


			RunCommand(command);
			
		}

		private void RunCommand(Command command)
		{
			switch (command.Id)
			{
				case Command.Type.InstallTask:
					MainViewModel.InstallTask();
					RunCommand(new Command(Command.Type.NormalStartup));
					break;
				case Command.Type.RemoveTask:
					MainViewModel.RemoveTask();
					RunCommand(new Command(Command.Type.NormalStartup));
					break;
				case Command.Type.ShowStartup:
					mainWindow.Show();
					break;
				case Command.Type.NormalStartup:
				default:
					if (!Settings.FirstTime)
					{
						mainWindow.ShowInTaskbar = false;
						mainWindow.WindowState = WindowState.Minimized;
						mainWindow.Show();
						mainWindow.Hide();
						mainWindow.ShowInTaskbar = true;
						Settings.FirstTime = false;
					}
					else
					{
						mainWindow.Show();
					}
					break;
			}
		}


		private void ShowWindow()
		{
			if (mainWindow.IsVisible)
			{
				mainWindow.Activate();
				// flash
			}
			else
			{
				mainWindow.Show();
				
			}
			mainWindow.WindowState = WindowState.Normal;

		}

		private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
		{
			if (msg == WM_SHOWME)
			{
				ShowWindow();
			}
			return IntPtr.Zero;
		}
	}


}
