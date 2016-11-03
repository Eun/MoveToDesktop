using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

using System.Runtime.InteropServices;
using System.Threading;


namespace MoveToDesktop
{

	internal class RunHelper
	{
		

		private static readonly bool _is64BitOperatingSystem = Is64BitOperatingSystem();


		private static IEnumerable<Runner> Runners = new List<Runner>()
		{
			new Runner()
			{
				Architecture = "x86",
				Mutex = Settings.MutexX86,
			},
			_is64BitOperatingSystem ? new Runner()
			{
				Architecture = "x64",
				Mutex = Settings.MutexX64,
			} : null,
		};


		public static void Exit()
		{
			KillInstances();
		}

		public static void Start()
		{
			// Kill existing Instances
			Exit();

			// Start all instances
			StartInstances();
		}

		private static void StartInstances()
		{
			foreach (var runner in Runners)
			{
				StartInstance(runner);
			}
		}

		private static void KillInstances()
		{
			using (var wh = new EventWaitHandle(false, EventResetMode.AutoReset, "MoveToDesktopShutdown"))
			{
				wh.Set();
			}
			// give a bit time to react
			Thread.Sleep(100);

			foreach (var runner in Runners)
			{
				ForceKillInstances(runner);
			}
		}


		private static bool StartInstance(Runner runner)
		{
			if (runner.RunnerPath != null)
			{
				return Process.Start(new ProcessStartInfo()
				{
					WorkingDirectory = Path.GetDirectoryName(runner.RunnerPath),
					FileName = runner.RunnerPath,
					UseShellExecute = true,
				}) != null;
			}
			return false;
		}

		

		private static bool ForceKillInstances(Runner runner)
		{
			var mutex = Natives.OpenMutex(Natives.MUTEX_ALL_ACCESS, false, runner.Mutex);

			if (mutex == IntPtr.Zero)
			{
				return true;
			}
			else
			{
				Natives.CloseHandle(mutex);
				try
				{
					
					foreach (var process in Process.GetProcessesByName(runner.ProcessName))
					{
						process.Kill();
					}
				}
				catch
				{
					
				}
				return true;
			}
		}


		





		[DllImport("kernel32.dll")]
		static extern IntPtr GetCurrentProcess();

		[DllImport("kernel32.dll")]
		static extern IntPtr GetModuleHandle(string moduleName);

		[DllImport("kernel32")]
		static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

		[DllImport("kernel32.dll")]
		static extern bool IsWow64Process(IntPtr hProcess, out bool wow64Process);

		public static bool Is64BitOperatingSystem()
		{
			// Check if this process is natively an x64 process. If it is, it will only run on x64 environments, thus, the environment must be x64.
			if (IntPtr.Size == 8)
				return true;
			// Check if this process is an x86 process running on an x64 environment.
			IntPtr moduleHandle = GetModuleHandle("kernel32");
			if (moduleHandle != IntPtr.Zero)
			{
				IntPtr processAddress = GetProcAddress(moduleHandle, "IsWow64Process");
				if (processAddress != IntPtr.Zero)
				{
					bool result;
					if (IsWow64Process(GetCurrentProcess(), out result) && result)
						return true;
				}
			}
			// The environment must be an x86 environment.
			return false;
		}
	}
}
