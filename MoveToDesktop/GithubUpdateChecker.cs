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
using System.Linq;
using System.Net;
using System.Text.RegularExpressions;
using Newtonsoft.Json.Linq;

public class GithubUpdateChecker
{
	public string Owner { get; set; }
	public string Repository { get; set; }

	public VersionParser VersionParser { get; set; }

	public AssetFinder AssetFinder { get; set; }

	private string Url => $"https://api.github.com/repos/{Owner}/{Repository}/releases/latest";



	public GithubUpdateChecker(string owner, string repository, VersionParser versionParser, AssetFinder assetFinder)
	{
		Owner = owner;
		Repository = repository;
		VersionParser = versionParser;
		AssetFinder = assetFinder;
	}
	public GithubUpdateChecker(string owner, string repository) : this(owner, repository, new TagVersionParser(), new FirstAssetFinder())
	{

	}


	public VersionInformation GetLatestVersion()
	{
		JObject json;
		try
		{
			using (var webClient = new WebClient())
			{
				webClient.Headers.Add(HttpRequestHeader.UserAgent, "GithubUpdateChecker.cs");
				var jsonString = webClient.DownloadString(Url);
				json = JObject.Parse(jsonString);
			}
		}
		catch (Exception e)
		{
			return null;
		}

		if (json == null)
			return null;
		var version =  VersionParser.ParseVersion(json);
		if (version == null)
			return null;

		var name = json.GetValue("name", StringComparison.InvariantCultureIgnoreCase);
		if (name == null)
			return null;

		var body = json.GetValue("body", StringComparison.InvariantCultureIgnoreCase);
		if (body == null)
			return null;

		

		return new VersionInformation()
		{
			Body = body.ToString(),
			Name = name.ToString(),
			Version = version,
			DownloadUrl = AssetFinder.DownloadUrl(json),
		};

	}
}

public class VersionInformation
{
	public Version Version { get; set; }
	public string Name { get; set; }
	public string Body { get; set; }
	public string DownloadUrl { get; set; }
}

public class TagVersionParser : VersionParser
{
	private static readonly Regex regex = new Regex(@"(?<major>\d)\.(?<minor>\d)(\.(?<build>\d))?(\.(?<revision>\d))?");
	public override Version ParseVersion(JObject json)
	{
		JToken tagName;
		if (!json.TryGetValue("tag_name", StringComparison.InvariantCultureIgnoreCase, out tagName))
			return null;
		var matches = regex.Matches(tagName.ToString());
		if (matches.Count <= 0)
			return null;


		if (matches[0].Groups["build"].Success && matches[0].Groups["revision"].Success)
			return new Version(Int32.Parse(matches[0].Groups["major"].Value), Int32.Parse(matches[0].Groups["minor"].Value), Int32.Parse(matches[0].Groups["build"].Value), Int32.Parse(matches[0].Groups["revision"].Value));
		else if (matches[0].Groups["build"].Success)
			return new Version(Int32.Parse(matches[0].Groups["major"].Value), Int32.Parse(matches[0].Groups["minor"].Value), Int32.Parse(matches[0].Groups["build"].Value));
		else
			return new Version(Int32.Parse(matches[0].Groups["major"].Value), Int32.Parse(matches[0].Groups["minor"].Value));

	}
}


public class FirstAssetFinder : AssetFinder
{
	public override string DownloadUrl(JObject json)
	{
		JToken assets;
		if (!json.TryGetValue("assets", StringComparison.InvariantCultureIgnoreCase, out assets))
			return null;

		if (!(assets is JArray))
			return null;

		var firstAsset = (((JArray) assets).FirstOrDefault() as JObject);

		if (firstAsset == null)
			return null;

		JToken downloadUrl;
		if (!firstAsset.TryGetValue("browser_download_url", StringComparison.InvariantCultureIgnoreCase, out downloadUrl))
			return null;


		return downloadUrl.ToString();
	}
}


public abstract class VersionParser
{
	public abstract Version ParseVersion(JObject json);
}

public abstract class AssetFinder
{
	public abstract string DownloadUrl(JObject json);
}
