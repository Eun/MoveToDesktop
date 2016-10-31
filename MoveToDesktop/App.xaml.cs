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

		public App()
		{
			mutex = new Mutex(false, Settings.Mutex);
			if (!mutex.WaitOne(0, false))
			{
				PostMessage((IntPtr)HWND_BROADCAST, WM_SHOWME, IntPtr.Zero, IntPtr.Zero);
				Application.Current.Shutdown();
				return;
			}

			
			Application.Current.Exit += (sender, args) =>
			{
				Runner.Exit();
				mutex.ReleaseMutex();
				
			};

			try
			{
				Runner.Start();
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, "MoveToDesktop", MessageBoxButton.OK, MessageBoxImage.Error);
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


			if (!Settings.FirstTime)
			{
				mainWindow.ShowInTaskbar = false;
				mainWindow.WindowState = WindowState.Minimized;
				mainWindow.Show();
				mainWindow.Hide();
				mainWindow.ShowInTaskbar = true;
			}
			else
			{
				mainWindow.Show();
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
