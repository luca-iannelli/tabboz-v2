param([string]$Configuration = 'Release')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $env:TEMP 'Tabboz-v2-native-build'
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual Studio C++ Build Tools x64 are required.' }
$dev = Join-Path $vs 'Common7\Tools\VsDevCmd.bat'
$cmd = '"' + $dev + '" -arch=x64 -host_arch=x64 >nul && set'
& $env:ComSpec /d /s /c $cmd | ForEach-Object {
 if ($_ -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process') }
}
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ninja = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
& $cmake -S $root -B $build -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" "-DCMAKE_BUILD_TYPE=$Configuration"
if ($LASTEXITCODE) { throw 'CMake configuration failed' }
& $cmake --build $build --parallel 4
if ($LASTEXITCODE) { throw 'Build failed' }
$env:TABBOZ_UNC_TEST_DIR = "$root\build\unc-save-test"
& (Join-Path (Split-Path $cmake) 'ctest.exe') --test-dir $build --output-on-failure
if ($LASTEXITCODE) { throw 'Native tests failed' }
New-Item -ItemType Directory -Force "$root\build\windows" | Out-Null
Copy-Item "$build\*.exe" "$root\build\windows\" -Force
Write-Host "Build output: $root\build\windows; native CMake tree: $build"
