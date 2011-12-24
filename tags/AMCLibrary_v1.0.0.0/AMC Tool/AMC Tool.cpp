////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AMCTool.cpp
///  Description: AMC Tool application starting code.
///  Author:      Chiuta Adrian Marius
///  Created:     07-01-2010
///
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  http://www.apache.org/licenses/LICENSE-2.0
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "platform.h"
#include "AESWizard.h"

CAppModule _Module;
HINSTANCE g_hInstance;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString
getAppFolder(HINSTANCE hInstance)
{
    TCHAR moduleFileName[MAX_PATH];
    if( GetModuleFileName(hInstance, moduleFileName, MAX_PATH) == 0 )
        return CString(_T(""));

    CString cPath = moduleFileName;
    return CString( cPath.Left(cPath.ReverseFind('\\')) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool
IsWindowsVistaOrLater()
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    return osvi.dwMajorVersion >= 6;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI
_tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    HRESULT hRes = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_BAR_CLASSES);

    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    AtlAxWinInit();

    if( IsWindowsVistaOrLater() )
    {
        AESWizard dlgMain;
        dlgMain.SetNoMargin();
        dlgMain.DoModal(NULL);

        if( dlgMain.mBenchmarkHaveResults )
        {
            CString resultFileName;

            CString appFolder = getAppFolder(hInstance);

            // Find a temporary name for the file where the results are saved
            DWORD dwRetVal = 0;
            TCHAR szTempFileName[MAX_PATH];
            TCHAR lpTempPathBuffer[MAX_PATH];

            // Gets the temp path env string (no guarantee it's a valid path).
            dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
            if( dwRetVal > MAX_PATH || (dwRetVal == 0) )
                _tcscpy_s(lpTempPathBuffer, MAX_PATH, appFolder);

            //  Generates a temporary file name.
            if( GetTempFileName(lpTempPathBuffer, _T("AMCResults"), 0, szTempFileName) == 0 )
            {
                _tcscpy_s(szTempFileName, MAX_PATH, lpTempPathBuffer);
                _tcscat_s(szTempFileName, MAX_PATH, _T("\\AMC_Results.tmp"));
            }

            resultFileName = szTempFileName;
            if( dlgMain.mAESHelper->SaveBenchResults(resultFileName) )
            {
                CString viewerPath = appFolder + _T("\\AMC Tool Results Viewer.exe");

                TCHAR szArguments[MAX_PATH];
                _tcscpy_s(szArguments, MAX_PATH, _T("\""));
                _tcscat_s(szArguments, MAX_PATH, szTempFileName);
                _tcscat_s(szArguments, MAX_PATH, _T("\""));

                if( (int)ShellExecute(GetDesktopWindow(), _T("open"), viewerPath, szArguments, NULL, SW_SHOWNORMAL) <= 32 )
                    _tunlink(szTempFileName);
            }
        }
    }
    else
        MessageBox(NULL, _T("This application can run only on Windows Vista or newer!"),
                         _T("AMC Tool: Error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

    _Module.Term();
    ::CoUninitialize();

    return 0;
}
