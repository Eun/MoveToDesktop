using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Reflection;

namespace MoveToDesktop
{
	internal class Runner
	{
		public string Architecture { get; set; }

		private string _runnerPath;

		public string RunnerPath
		{
			get
			{
				if (_runnerPath == null)
				{
					_runnerPath = ExtractFile($"MoveToDesktopRunner.{Architecture}.exe", $"MoveToDesktop.{Architecture}.exe");
				}
				if (File.Exists(_runnerPath))
					return _runnerPath;
				return null;
			}
		}

		
		public string Mutex { get; set; }

		public string ProcessName => $"MoveToDesktop.{Architecture}";


		private static readonly Assembly _assembly = Assembly.GetExecutingAssembly();
		private static readonly string _assemblyName = _assembly.GetName().Name;

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

	}
}
