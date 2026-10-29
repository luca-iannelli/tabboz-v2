param([string]$Installer)
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
if(!$Installer){$Installer="$root\dist\TabbozSimulator-2.0.0-Windows-x64-Setup.exe"}
$tag=[DateTime]::UtcNow.Ticks
$install=Join-Path $env:TEMP "Tabboz-v2-install-test-$tag"
$report="$root\build\validation-installer"
New-Item -ItemType Directory -Force $report | Out-Null
function Invoke-Setup {
 $p=Start-Process -FilePath $Installer -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',"/DIR=$install","/GROUP=TabbozV2-Validation-$tag") -PassThru -Wait
 if($p.ExitCode -ne 0){throw "Installer failed: $($p.ExitCode)"}
}
Invoke-Setup
if(!(Test-Path "$install\Tabboz.exe")){throw 'Installed executable missing.'}
& "$PSScriptRoot\smoke-windows.ps1" -Executable "$install\Tabboz.exe" -OutputDirectory $report
if(!(Test-Path "$report\smoke.json")){throw 'Installed application smoke report missing.'}
$state=Get-ChildItem $report -Directory -Filter 'state-*' | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$save=Join-Path $state.FullName 'current.tbz'
$before=[IO.File]::ReadAllBytes($save)
Invoke-Setup
if([Convert]::ToBase64String($before) -ne [Convert]::ToBase64String([IO.File]::ReadAllBytes($save))){throw 'Upgrade changed saved game.'}
$p=Start-Process -FilePath "$install\unins000.exe" -ArgumentList '/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART' -Wait -PassThru
if($p.ExitCode -ne 0){throw "Uninstall failed: $($p.ExitCode)"}
for($i=0;$i -lt 50 -and (Test-Path "$install\Tabboz.exe");$i++){Start-Sleep -Milliseconds 100}
if(Test-Path "$install\Tabboz.exe"){throw 'Executable still exists after uninstall.'}
if(!(Test-Path $save)){throw 'Saved game was removed by uninstall.'}
@{status='passed';install=$true;installed_app_launch=$true;upgrade=$true;uninstall=$true;saved_game_preserved=$true} | ConvertTo-Json | Set-Content "$report\installer.json" -Encoding UTF8
