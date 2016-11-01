using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
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
			foreach (var runner in Runners)
			{
				KillInstances(runner);
			}
		}


		private static bool StartInstance(Runner runner)
		{
			if (runner.RunnerPath != null)
			{
				return Process.Start(new ProcessStartInfo()
				{
					FileName = runner.RunnerPath,
					UseShellExecute = true,
				}) != null;
			}
			return false;
		}



		[DllImport("kernel32.dll")]
		static extern IntPtr OpenMutex(uint dwDesiredAccess, bool bInheritHandle, string lpName);

		const UInt32 MUTEX_ALL_ACCESS = 0x1F0001;

		[DllImport("kernel32.dll", SetLastError = true)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
		[SuppressUnmanagedCodeSecurity]
		[return: MarshalAs(UnmanagedType.Bool)]
		static extern bool CloseHandle(IntPtr hObject);


		private static bool KillInstances(Runner runner)
		{
			var mutex = OpenMutex(MUTEX_ALL_ACCESS, false, runner.Mutex);

			if (mutex == IntPtr.Zero)
			{
				return true;
			}
			else
			{
				CloseHandle(mutex);

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
