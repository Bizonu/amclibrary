;--------------------------------
;  File:        Build_Setup_x86.nsi
;  Description: The build script for the x86 setup.
;  Author:      Chiuta Adrian Marius
;  Created:     17-02-2010
;
;  Licensed under the Apache License, Version 2.0 (the "License");
;  you may not use this file except in compliance with the License.
;  You may obtain a copy of the License at
;  http://www.apache.org/licenses/LICENSE-2.0
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.
;
;--------------------------------

;--------------------------------
;Include

  !include "MUI2.nsh"

;--------------------------------
;General
  SetCompressor /FINAL /SOLID lzma

  ;Name and file
  Name "AMC Tools"
  OutFile "AMC Tools Setup x86.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\AMC Tools"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\AMC Tools" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

  Var FlashPlayerSetupError

;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
  !define MUI_HEADERIMAGE_BITMAP ".\Resources\Header.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP ".\Resources\WelcomeHeader.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\Resources\WelcomeHeader.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Flash Player Install" SecFlashPlayer
 SectionIn RO

 SetOutPath "$TEMP"
 File "..\redist\x86\install_flashplayer11x32ax_gtbd_aih.exe"
 DetailPrint "Running Flash Player ActiveX Setup..."
 ExecWait '"$TEMP\install_flashplayer11x32ax_gtbd_aih.exe"' $FlashPlayerSetupError
 DetailPrint "Finished Flash ActiveX Setup"

 SetOutPath "$INSTDIR"
SectionEnd

Section "Install Section" SecInstall

  SetOutPath "$INSTDIR"

  FILE "..\redist\x86\*.dll"
  FILE "..\bin\Win32\Release\AES_CPU.amc"
  FILE "..\bin\Win32\Release\AES_GPU_DX10.amc"
  FILE "..\bin\Win32\Release\AMCLibrary.dll"
  FILE "..\bin\Win32\Release\AMC Tool.exe"
  FILE "..\bin\Win32\Release\AMC Tool Results Viewer.exe"

  CreateDirectory "$SMPROGRAMS\AMC Tools"
  createShortCut "$SMPROGRAMS\AMC Tools\AMC Tool.lnk" "$INSTDIR\AMC Tool.exe"
  createShortCut "$SMPROGRAMS\AMC Tools\Results Viewer.lnk" "$INSTDIR\AMC Tool Results Viewer.exe"
  createShortCut "$SMPROGRAMS\AMC Tools\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  ;Store installation folder
  WriteRegStr HKCU "Software\AMC Tools" "" $INSTDIR

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools" "DisplayName" "AMC Tools"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"  "DisplayIcon" "$INSTDIR\AMC Tool.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"  "Publisher" "CAM Software"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"  "DisplayVersion" "1.0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"  "Comments" "These tools consists of encryption/decryption and benchmarking using AES algorithm."
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"  "EstimatedSize" 2048
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.amc"
  Delete "$INSTDIR\*.exe"

  RMDir "$INSTDIR"
  RMDir /r "$SMPROGRAMS\AMC Tools"

  DeleteRegKey /ifempty HKCU "Software\AMC Tools"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AMC Tools"

SectionEnd