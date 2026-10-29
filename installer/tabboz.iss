#ifndef SourceRoot
  #define SourceRoot ".."
#endif
#define Version "2.0.0"
[Setup]
AppId={{B598A483-CE33-47CB-8711-51342EA8C820}
AppName=Tabboz Simulator - Windows Edition
AppVersion={#Version}
AppPublisher=Tabboz Simulator contributors
AppPublisherURL=https://github.com/luca-iannelli/tabboz-v2
AppSupportURL=https://github.com/luca-iannelli/tabboz-v2/issues
DefaultDirName={localappdata}\Programs\Tabboz Simulator
DefaultGroupName=Tabboz Simulator
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.18362
WizardStyle=modern
DisableProgramGroupPage=yes
LicenseFile={#SourceRoot}\LICENSE
OutputDir={#SourceRoot}\dist
OutputBaseFilename=TabbozSimulator-{#Version}-Windows-x64-Setup
SetupIconFile={#SourceRoot}\assets\app.ico
UninstallDisplayIcon={app}\Tabboz.exe
Compression=lzma2
SolidCompression=yes
CloseApplications=yes
RestartApplications=no
AllowUNCPath=yes
[Languages]
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"
[Tasks]
Name: "desktopicon"; Description: "Crea un collegamento sul desktop"; Flags: unchecked
[Files]
Source: "{#SourceRoot}\build\windows\Tabboz.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\docs\LEGGIMI.txt"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\Tabboz Simulator"; Filename: "{app}\Tabboz.exe"
Name: "{userdesktop}\Tabboz Simulator"; Filename: "{app}\Tabboz.exe"; Tasks: desktopicon
[Run]
Filename: "{app}\Tabboz.exe"; Description: "Avvia Tabboz Simulator"; Flags: nowait postinstall skipifsilent
