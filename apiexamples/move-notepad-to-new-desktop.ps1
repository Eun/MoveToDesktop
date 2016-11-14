$MoveToDesktopPath="MoveToDesktop.exe"

function Get-WindowByTitle($WindowTitle)
{
    return (Get-Process | Where-Object {$_.MainWindowTitle -like "$WindowTitle"} | Select-Object MainWindowHandle).MainWindowHandle
}

function Get-Helper($hwnd)
{
	$pinfo = New-Object System.Diagnostics.ProcessStartInfo
	$pinfo.FileName = $MoveToDesktopPath
	$pinfo.RedirectStandardError = $true
	$pinfo.RedirectStandardOutput = $true
	$pinfo.UseShellExecute = $false
	$pinfo.Arguments = "--get-api-helper=$hwnd"
	$p = New-Object System.Diagnostics.Process
	$p.StartInfo = $pinfo
	$p.Start() | Out-Null
	$p.WaitForExit()
	$stdout = $p.StandardOutput.ReadToEnd()
	return $stdout.Trim()
}


$hwnd = Get-WindowByTitle("* - Notepad")

if ($hwnd)
{
	Write-Verbose "HWND IS $hwnd"
	$helper = Get-Helper($hwnd)
	Write-Verbose "HelperPath is: '$helper'"
	Start-Process $helper -ArgumentList "--move-to-new-desktop $hwnd"
}