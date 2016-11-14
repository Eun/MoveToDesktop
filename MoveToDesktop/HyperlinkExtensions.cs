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


using System.Diagnostics;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Navigation;

namespace MoveToDesktop
{
	public static class HyperlinkExtensions
	{
		public static bool GetIsExternal(DependencyObject obj)
		{
			return (bool)obj.GetValue(IsExternalProperty);
		}

		public static void SetIsExternal(DependencyObject obj, bool value)
		{
			obj.SetValue(IsExternalProperty, value);
		}
		public static readonly DependencyProperty IsExternalProperty =
			DependencyProperty.RegisterAttached("IsExternal", typeof(bool), typeof(HyperlinkExtensions), new UIPropertyMetadata(false, OnIsExternalChanged));

		private static void OnIsExternalChanged(object sender, DependencyPropertyChangedEventArgs args)
		{
			var hyperlink = sender as Hyperlink;

			if ((bool)args.NewValue)
				hyperlink.RequestNavigate += Hyperlink_RequestNavigate;
			else
				hyperlink.RequestNavigate -= Hyperlink_RequestNavigate;
		}

		private static void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
		{
			Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
			e.Handled = true;
		}
	}
}
