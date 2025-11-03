[Setup]
AppName=Providers Agent GPU Probe
AppVersion=1.0.0
AppPublisher=Providers Agent Team
DefaultDirName={autopf}\ProvidersAgent
DefaultGroupName=Providers Agent
OutputBaseFilename=ProvidersAgent-GPUProbe-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
MinVersion=10.0
PrivilegesRequired=admin

[Files]
; Main executable
Source: "..\build\bin\Release\providers_agent.exe"; DestDir: "{app}"; Flags: ignoreversion
; Runtime dependencies (if needed)
Source: "..\build\bin\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; Documentation
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
; Create logs directory
Name: "{app}\logs"

[Icons]
; Start menu shortcut
Name: "{group}\Providers Agent GPU Probe"; Filename: "{app}\providers_agent.exe"; WorkingDir: "{app}"
; Desktop shortcut (optional)
Name: "{autodesktop}\Providers Agent GPU Probe"; Filename: "{app}\providers_agent.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Tasks]
; Optional desktop icon
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Run]
; Run application after install (optional)
Filename: "{app}\providers_agent.exe"; Description: "Launch Providers Agent GPU Probe"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Clean up log files on uninstall
Type: filesandordirs; Name: "{app}\logs"
Type: files; Name: "{app}\gpu_info.json"
