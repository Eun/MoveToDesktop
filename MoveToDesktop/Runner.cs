using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace MoveToDesktop
{

	internal class Runner
	{
		private static string _runner_path_x86;
		private static string _runner_path_x64;
		private static string _hook_path_x86;
		private static string _hook_path_x64;

		private static readonly Assembly _assembly = Assembly.GetExecutingAssembly();
		private static readonly string _assemblyName = _assembly.GetName().Name;

		private static Process _runner_x86;
		private static Process _runner_x64;

		private static readonly bool _is64BitOperatingSystem = Is64BitOperatingSystem();

		public static void Exit()
		{
			if (IsProcessRunning(_runner_x86))
			{
				_runner_x86.Kill();
				_runner_x86 = null;
			}


			if (_is64BitOperatingSystem && IsProcessRunning(_runner_x64))
			{
				_runner_x64.Kill();
				_runner_x64 = null;
			}

		}

		public static void Start()
		{
			if (IsProcessRunning(_runner_x86))
			{
				_runner_x86.Kill();
			}


			if (_is64BitOperatingSystem && IsProcessRunning(_runner_x64))
			{
				_runner_x64.Kill();
			}


			if (_runner_path_x86 == null || _hook_path_x86 == null)
			{
				_runner_path_x86 = ExtractFile("x86.MoveToDesktopRunner.exe", "MoveToDesktop.x86.exe");
				_hook_path_x86 = ExtractFile("x86.hook.dll");
			}

			_runner_x86 = Process.Start(new ProcessStartInfo()
			{
				FileName = _runner_path_x86,
				Arguments = _hook_path_x86,
				UseShellExecute = true,
			});
			_runner_x86.Exited += runnerExited;

			if (_is64BitOperatingSystem)
			{

				if (_runner_path_x64 == null || _hook_path_x64 == null)
				{
					_runner_path_x64 = ExtractFile("x64.MoveToDesktopRunner.exe", "MoveToDesktop.x64.exe");
					_hook_path_x64 = ExtractFile("x64.hook.dll");
				}

				_runner_x64 = Process.Start(new ProcessStartInfo()
				{
					FileName = _runner_path_x64,
					Arguments = _hook_path_x64,
					UseShellExecute = true,
				});
				_runner_x64.Exited += runnerExited;
			}

		}

		private static void runnerExited(object sender, EventArgs eventArgs)
		{
			if (sender == _runner_x86)
			{
				_runner_x86 = null;
			}
			else if (sender == _runner_x64)
			{
				_runner_x64 = null;
			}
		}

		private static bool IsProcessRunning(Process process)
		{
			if (process == null) { return false; }
			try { Process.GetProcessById(process.Id); }
			catch (InvalidOperationException) { return false; }
			catch (ArgumentException) { return false; }
			return true;
		}


		private static string ExtractFile(string path, string fileName = null)
		{
			var stream = _assembly.GetManifestResourceStream($"{_assemblyName}.Resources.{path}");
			if (stream == null)
			{
				return null;
				
			}
			string dest;

			if (string.IsNullOrEmpty(fileName))
				dest = Path.GetTempFileName() + System.IO.Path.GetExtension(path);
			else
				dest = System.IO.Path.Combine(Path.GetTempPath(), fileName);

			using (var writer = new BinaryWriter(File.Open(dest, FileMode.OpenOrCreate)))
			{
				writer.Seek(0, SeekOrigin.Begin);
				int n;
				byte[] buffer = new byte[4096];
				while ((n = stream.Read(buffer, 0, 4096)) > 0)
				{
					writer.Write(buffer, 0, n);
				}
			}
			return dest;
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
