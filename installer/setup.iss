; TODO: Configure Inno Setup script
[Setup]
AppName=Providers Agent
AppVersion=1.0.0
DefaultDirName={pf}\ProvidersAgent
DefaultGroupName=Providers Agent
OutputBaseFilename=ProvidersAgentSetup
Compression=lzma2
SolidCompression=yes

[Files]
; TODO: Add files to include in installer
; Source: "..\build\bin\Release\providers_agent.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
; TODO: Add shortcuts
; Name: "{group}\Providers Agent"; Filename: "{app}\providers_agent.exe"

[Run]
; TODO: Add post-install actions
