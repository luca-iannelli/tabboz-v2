param([string]$InnoCompiler)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if (!$InnoCompiler) {
 $found = Get-Command ISCC.exe -ErrorAction SilentlyContinue
 if ($found) { $InnoCompiler = $found.Source }
 elseif (Test-Path "$root\.tools\inno\tools\ISCC.exe") { $InnoCompiler = "$root\.tools\inno\tools\ISCC.exe" }
 elseif (Test-Path "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe") { $InnoCompiler = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" }
 else { throw 'Inno Setup 6.7.3+ compiler not found. Pass -InnoCompiler.' }
}
New-Item -ItemType Directory -Force "$root\dist" | Out-Null
& $InnoCompiler "/DSourceRoot=$root" "$root\installer\tabboz.iss"
if ($LASTEXITCODE) { throw 'Installer compilation failed.' }
$stage = Join-Path $env:TEMP 'Tabboz-v2-portable-stage'
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item "$root\build\windows\Tabboz.exe","$root\LICENSE","$root\docs\LEGGIMI.txt" $stage -Force
Compress-Archive -LiteralPath "$stage\Tabboz.exe","$stage\LICENSE","$stage\LEGGIMI.txt" -DestinationPath "$root\dist\TabbozSimulator-2.0.0-Windows-x64-Portable.zip" -Force
function Get-Sha256([string]$Path) {
 $stream=[IO.File]::OpenRead($Path); $sha=[Security.Cryptography.SHA256]::Create()
 try { return ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-','').ToLower() }
 finally { $stream.Dispose(); $sha.Dispose() }
}
Get-ChildItem "$root\dist" -File | Where-Object { $_.Name -ne 'SHA256SUMS.txt' } | ForEach-Object { '{0}  {1}' -f (Get-Sha256 $_.FullName),$_.Name } | Set-Content "$root\dist\SHA256SUMS.txt" -Encoding ASCII
