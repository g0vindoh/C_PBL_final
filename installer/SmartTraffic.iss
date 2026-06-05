; SmartTrafficSystem - Inno Setup Installer Script
; Produces: SmartTrafficSystem_Setup.exe
; The installed package is fully self-contained (SDL2 statically linked).

#define AppName       "Smart Traffic System"
#define AppVersion    "1.0.0"
#define AppPublisher  "g0vindoh"
#define AppURL        "https://github.com/g0vindoh/C_PBL_final"
#define AppExe        "SmartTrafficSystem_portable.exe"
#define InstalledExe  "SmartTrafficSystem.exe"

[Setup]
AppId={{B7A4D3F2-1C8E-4A0B-9F3D-2E5C7B8A1D94}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=Output
OutputBaseFilename=SmartTrafficSystem_Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
SetupIconFile=
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#InstalledExe}
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "startmenuicon"; Description: "Create a Start Menu shortcut"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; The main executable (SDL2 statically linked - no other DLLs needed)
Source: "..\{#AppExe}"; DestDir: "{app}"; DestName: "{#InstalledExe}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}";      Filename: "{app}\{#InstalledExe}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#InstalledExe}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#InstalledExe}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove the logs folder on uninstall
Type: filesandordirs; Name: "{app}\logs"
