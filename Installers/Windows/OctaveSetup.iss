; Octave Engine - Inno Setup Script
; Builds a Windows installer from the staged dist/Editor/ directory.
;
; Prerequisites:
;   1. Build the engine (ReleaseEditor x64)
;   2. Run: python Installers/stage_distribution.py --platform windows
;   3. Run: ISCC Installers/Windows/OctaveSetup.iss
;
; Or just run: Installers\build_installer_windows.bat

#define MyAppName "Octave Engine"
#define MyAppExeName "Octave.exe"
#define MyAppPublisher "Octave Engine"
#define MyAppURL "https://github.com/nicholasgasior/octave"

; Read version from staged version.txt
#define VersionFile AddBackslash(SourcePath) + "..\..\dist\Editor\version.txt"
#ifexist VersionFile
  #define MyAppVersion ReadIni(VersionFile, "Octave", "Version", "5")
#else
  #define MyAppVersion "5"
#endif

[Setup]
AppId={{A7E3F2C1-8B4D-4F6A-9C5E-1D2B3A4F5E6C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
DefaultDirName={autopf}\Octave
DefaultGroupName={#MyAppName}
LicenseFile=..\..\dist\Editor\LICENSE
OutputDir=..\..\dist
OutputBaseFilename=OctaveSetup-{#MyAppVersion}
SetupIconFile=..\..\dist\Editor\Standalone\Octave.ico
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
UninstallDisplayIcon={app}\Standalone\Octave.ico
PrivilegesRequired=admin
DisableProgramGroupPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Compact installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "core"; Description: "Octave Engine (required)"; Types: full compact custom; Flags: fixed
Name: "sdk"; Description: "Native Addon SDK (headers)"; Types: full
Name: "tools"; Description: "Build Tools (Python scripts)"; Types: full
Name: "docs"; Description: "Documentation"; Types: full

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "addtopath"; Description: "Add Octave to system PATH"; GroupDescription: "System integration:"
Name: "associateoctp"; Description: "Associate .octp files with Octave"; GroupDescription: "System integration:"

[Files]
; --- Core: Binary ---
Source: "..\..\dist\Editor\Octave.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: core

; --- Core: Engine Assets ---
Source: "..\..\dist\Editor\Engine\Assets\*"; DestDir: "{app}\Engine\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: core

; --- Core: Shaders ---
Source: "..\..\dist\Editor\Engine\Shaders\*"; DestDir: "{app}\Engine\Shaders"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: core

; --- Core: Scripts ---
Source: "..\..\dist\Editor\Engine\Scripts\*"; DestDir: "{app}\Engine\Scripts"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: core

; --- Core: Generated (IntelliSense) ---
Source: "..\..\dist\Editor\Engine\Generated\*"; DestDir: "{app}\Engine\Generated"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: core

; --- Core: Template ---
Source: "..\..\dist\Editor\Template\*"; DestDir: "{app}\Template"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: core

; --- Core: Standalone sentinel ---
Source: "..\..\dist\Editor\Standalone\Standalone.rc"; DestDir: "{app}\Standalone"; Flags: ignoreversion; Components: core
Source: "..\..\dist\Editor\Standalone\resource.h"; DestDir: "{app}\Standalone"; Flags: ignoreversion; Components: core
Source: "..\..\dist\Editor\Standalone\Octave.ico"; DestDir: "{app}\Standalone"; Flags: ignoreversion skipifsourcedoesntexist; Components: core

; --- Core: Root files ---
Source: "..\..\dist\Editor\LICENSE"; DestDir: "{app}"; Flags: ignoreversion; Components: core
Source: "..\..\dist\Editor\OctaveLogo_128.png"; DestDir: "{app}"; Flags: ignoreversion; Components: core
Source: "..\..\dist\Editor\OctaveLogo_256.png"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist; Components: core
Source: "..\..\dist\Editor\version.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: core

; --- SDK: Engine Source Headers ---
Source: "..\..\dist\Editor\Engine\Source\*"; DestDir: "{app}\Engine\Source"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: sdk

; --- SDK: External Headers ---
Source: "..\..\dist\Editor\External\*"; DestDir: "{app}\External"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: sdk

; --- Tools ---
Source: "..\..\dist\Editor\Tools\*"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: tools

; --- Documentation ---
Source: "..\..\dist\Editor\Documentation\*"; DestDir: "{app}\Documentation"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: docs

[Dirs]
; Ensure Engine/Saves is writable by all users (preferences, editor state, shader cache)
Name: "{app}\Engine\Saves"; Permissions: users-modify

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[Registry]
; .octp file association
Root: HKLM; Subkey: "Software\Classes\.octp"; ValueType: string; ValueData: "OctaveProject"; Flags: uninsdeletevalue; Tasks: associateoctp
Root: HKLM; Subkey: "Software\Classes\OctaveProject"; ValueType: string; ValueData: "Octave Engine Project"; Flags: uninsdeletekey; Tasks: associateoctp
Root: HKLM; Subkey: "Software\Classes\OctaveProject\DefaultIcon"; ValueType: string; ValueData: "{app}\Standalone\Octave.ico,0"; Tasks: associateoctp
Root: HKLM; Subkey: "Software\Classes\OctaveProject\shell\open\command"; ValueType: string; ValueData: """{app}\{#MyAppExeName}"" -project ""%1"""; Tasks: associateoctp

[UninstallDelete]
; Clean up imgui.ini and any temp files at app root, but NOT Engine/Saves
Type: files; Name: "{app}\imgui.ini"
Type: filesandordirs; Name: "{app}\Engine\Saves\TempMaterial*"

[Code]
// --- PATH management ---

const
  EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

procedure AddToPath(Dir: String);
var
  OldPath: String;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', OldPath) then
    OldPath := '';
  // Only add if not already present
  if Pos(Uppercase(Dir), Uppercase(OldPath)) = 0 then
  begin
    if OldPath <> '' then
      OldPath := OldPath + ';';
    OldPath := OldPath + Dir;
    RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', OldPath);
  end;
end;

procedure RemoveFromPath(Dir: String);
var
  OldPath, NewPath, Token: String;
  P: Integer;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', OldPath) then
    Exit;
  NewPath := '';
  while OldPath <> '' do
  begin
    P := Pos(';', OldPath);
    if P = 0 then
    begin
      Token := OldPath;
      OldPath := '';
    end
    else
    begin
      Token := Copy(OldPath, 1, P - 1);
      OldPath := Copy(OldPath, P + 1, Length(OldPath));
    end;
    Token := Trim(Token);
    if (Token <> '') and (CompareText(Token, Dir) <> 0) then
    begin
      if NewPath <> '' then
        NewPath := NewPath + ';';
      NewPath := NewPath + Token;
    end;
  end;
  RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', NewPath);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    if IsTaskSelected('addtopath') then
      AddToPath(ExpandConstant('{app}'));
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    RemoveFromPath(ExpandConstant('{app}'));
  end;
end;
