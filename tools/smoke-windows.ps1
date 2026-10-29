param([string]$Executable,[string]$OutputDirectory,[switch]$ForceSaveFailure,[switch]$Activities)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if (!$Executable) { $Executable = "$root\build\windows\Tabboz.exe" }
if (!$OutputDirectory) { $OutputDirectory = "$root\build\validation" }
New-Item -ItemType Directory -Force $OutputDirectory | Out-Null
$env:TABBOZ_DATA_DIR = "$OutputDirectory\state-$([DateTime]::UtcNow.Ticks)"
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;
public static class TabbozWindows {
 public delegate bool EnumProc(IntPtr hwnd, IntPtr arg);
 [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc callback, IntPtr arg);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hwnd,out uint id);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr hwnd,StringBuilder text,int max);
 [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr hwnd);
 [DllImport("user32.dll")] public static extern IntPtr GetWindow(IntPtr hwnd,uint command);
 [DllImport("user32.dll")] public static extern bool IsWindowEnabled(IntPtr hwnd);
 [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hwnd);
 [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr hwnd,uint msg,IntPtr wp,IntPtr lp);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hwnd,uint msg,IntPtr wp,IntPtr lp);
 [DllImport("user32.dll")] public static extern IntPtr GetDlgItem(IntPtr hwnd,int id);
 [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hwnd);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hwnd,out Rect rect);
 [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr hwnd,int x,int y,int w,int h,bool repaint);
 [StructLayout(LayoutKind.Sequential)] public struct Rect { public int Left,Top,Right,Bottom; }
 public static IntPtr Find(int process,string title) {
  IntPtr found=IntPtr.Zero;
  EnumWindows((h,a)=>{uint id;GetWindowThreadProcessId(h,out id);var s=new StringBuilder(256);GetWindowText(h,s,256);if(id==process&&IsWindowVisible(h)&&s.ToString()==title){found=h;return false;}return true;},IntPtr.Zero);
  return found;
 }
 public static IntPtr Other(int process,IntPtr main) {
  IntPtr found=IntPtr.Zero;EnumWindows((h,a)=>{uint id;GetWindowThreadProcessId(h,out id);var text=new StringBuilder(256);GetWindowText(h,text,256);Rect r;GetWindowRect(h,out r);if(id==process&&h!=main&&IsWindowVisible(h)&&GetWindow(h,4)==main&&text.Length>0&&r.Right-r.Left>20&&r.Bottom-r.Top>20){found=h;return false;}return true;},IntPtr.Zero);return found;
 }
 public static string Text(IntPtr hwnd) {var s=new StringBuilder(512);GetWindowText(hwnd,s,512);return s.ToString();}
}
'@
function Wait-Window([int]$ProcessId,[string]$Title) {
 for ($i=0;$i -lt 100;$i++) { $h=[TabbozWindows]::Find($ProcessId,$Title); if ($h -ne [IntPtr]::Zero) { return $h }; Start-Sleep -Milliseconds 100 }
 throw "Window did not appear: $Title"
}
function Save-Window([IntPtr]$Handle,[string]$Name) {
 [TabbozWindows]::SetForegroundWindow($Handle) | Out-Null
 Start-Sleep -Milliseconds 300
 $rect = New-Object TabbozWindows+Rect
 for($i=0;$i -lt 50;$i++) {
  $ok=[TabbozWindows]::GetWindowRect($Handle,[ref]$rect)
  if($ok -and $rect.Right-$rect.Left -gt 20 -and $rect.Bottom-$rect.Top -gt 20){break}
  Start-Sleep -Milliseconds 100
 }
 if(!$ok -or $rect.Right-$rect.Left -le 20 -or $rect.Bottom-$rect.Top -le 20){throw "Window capture has invalid bounds: $Name"}
 $image=New-Object Drawing.Bitmap ($rect.Right-$rect.Left),($rect.Bottom-$rect.Top)
 $graphics=[Drawing.Graphics]::FromImage($image)
 $graphics.CopyFromScreen($rect.Left,$rect.Top,0,0,$image.Size)
 $image.Save((Join-Path $OutputDirectory $Name),[Drawing.Imaging.ImageFormat]::Png)
 $graphics.Dispose(); $image.Dispose()
}
$app=Start-Process -FilePath $Executable -PassThru
$app.Id | Set-Content "$OutputDirectory\processid.txt"
try {
 $splash=Wait-Window $app.Id 'Tabboz Simulator - Windows Edition'
 Save-Window $splash 'splash.png'
 [TabbozWindows]::PostMessage($splash,0x111,[IntPtr]1,[IntPtr]::Zero) | Out-Null
 $new=Wait-Window $app.Id 'Nuova partita'
 Save-Window $new 'new-game.png'
 [TabbozWindows]::PostMessage($new,0x111,[IntPtr]1,[IntPtr]::Zero) | Out-Null
 $main=Wait-Window $app.Id 'Tabboz Simulator'
 Save-Window $main 'main.png'
 if ($Activities) {
  foreach($command in @(130,131,132,136,137,133,134,135,141,142,155,140)) {
   [TabbozWindows]::PostMessage($main,0x111,[IntPtr]$command,[IntPtr]::Zero) | Out-Null
   $modal=[IntPtr]::Zero
   for($i=0;$i -lt 100;$i++){ $modal=[TabbozWindows]::Other($app.Id,$main);if($modal -ne [IntPtr]::Zero){break};Start-Sleep -Milliseconds 100 }
   if($modal -eq [IntPtr]::Zero){throw "Activity $command did not open"}
   Write-Host "Activity $command : $([TabbozWindows]::Text($modal))"
   Save-Window $modal "activity-$command.png"
   [TabbozWindows]::PostMessage($modal,0x10,[IntPtr]::Zero,[IntPtr]::Zero) | Out-Null
   for($i=0;$i -lt 100 -and ([TabbozWindows]::IsWindow($modal) -or ![TabbozWindows]::IsWindowEnabled($main));$i++){Start-Sleep -Milliseconds 100}
   if(![TabbozWindows]::IsWindowEnabled($main)){throw "Activity $command did not close"}
  }
 }
 $money=[TabbozWindows]::Text([TabbozWindows]::GetDlgItem($main,105))
 if ($money -notmatch 'EUR') { throw "Euro amount missing: $money" }
 if ($ForceSaveFailure) {
  New-Item -ItemType Directory -Force "$env:TABBOZ_DATA_DIR\current.tbz" | Out-Null
 } else {
  [TabbozWindows]::PostMessage($main,0x111,[IntPtr]40002,[IntPtr]::Zero) | Out-Null
  Start-Sleep -Milliseconds 200
 }
 [TabbozWindows]::PostMessage($main,0x10,[IntPtr]::Zero,[IntPtr]::Zero) | Out-Null
 if ($ForceSaveFailure) {
  $errorWindow=Wait-Window $app.Id 'Salvataggio non riuscito'
  Save-Window $errorWindow 'save-error.png'
  [TabbozWindows]::PostMessage($errorWindow,0x466,[IntPtr]1003,[IntPtr]::Zero) | Out-Null
 }
 if (!$app.WaitForExit(10000)) { throw 'Application did not close.' }
 if ($app.ExitCode -ne 0) { throw "Application exit code: $($app.ExitCode)" }
 if (!$ForceSaveFailure -and !(Test-Path "$env:TABBOZ_DATA_DIR\current.tbz" -PathType Leaf)) { throw 'Autosave missing.' }
 @{status='passed';forced_save_failure=[bool]$ForceSaveFailure;os=[Environment]::OSVersion.VersionString;currency=$money;executable=$Executable;screenshots=@('splash.png','new-game.png','main.png')} | ConvertTo-Json | Set-Content "$OutputDirectory\smoke.json" -Encoding UTF8
} finally {
 if (!$app.HasExited) {
  for($i=0;$i -lt 30 -and !$app.HasExited;$i++) {
   $main=[TabbozWindows]::Find($app.Id,'Tabboz Simulator')
   $other=[TabbozWindows]::Other($app.Id,$main)
   $window=if($other -ne [IntPtr]::Zero){$other}else{$main}
   if($window -ne [IntPtr]::Zero){[TabbozWindows]::PostMessage($window,0x10,[IntPtr]::Zero,[IntPtr]::Zero)|Out-Null}
   Start-Sleep -Milliseconds 100
  }
  if(!$app.HasExited){Stop-Process -Id $app.Id}
 }
}
