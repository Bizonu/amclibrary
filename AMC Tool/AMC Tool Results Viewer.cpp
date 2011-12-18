#include "platform.h"
#include "CGraphDlg.h"

CAppModule _Module;
HINSTANCE g_hInstance;

bool IsWindowsVistaOrLater()
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return osvi.dwMajorVersion >= 6;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	g_hInstance = hInstance;

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	AtlAxWinInit();

	LPTSTR lpFileName = lpstrCmdLine;
	if( lpFileName != NULL )
	{
		if(lpFileName[0] == '\"')
			lpFileName++;
		int fnLen = (int)_tcsnlen(lpFileName, MAX_PATH) - 1;
		if( lpFileName[fnLen] == '\"' )
			lpFileName[fnLen] = '\0';
	}

	if( IsWindowsVistaOrLater() )
	{
		HKEY hKey;
		// Check if flash player is installed
		bool isFlashInstalled = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
											  _T("SOFTWARE\\Classes\\TypeLib\\{D27CDB6B-AE6D-11CF-96B8-444553540000}"), 
											  0, 
											  KEY_QUERY_VALUE, 
											  &hKey 
											) == ERROR_SUCCESS;
		if (isFlashInstalled)
		{
			// Close key
			RegCloseKey( hKey );

			CGraphDlg dlgResults;
			if( lpFileName != NULL )
				dlgResults.LoadBenchResults(lpFileName);
			dlgResults.DoModal(NULL);
		}
		else
		{
			// Start the internet explorer to allow the user to download and install flash player
			TCHAR iexplore[MAX_PATH];
			GetEnvironmentVariable(_T("ProgramFiles"), iexplore, MAX_PATH);
			_tcscat_s(iexplore, MAX_PATH, _T("\\Internet Explorer\\iexplore.exe"));
			TCHAR *szArguments = _T("http://get.adobe.com/flashplayer/");
			ShellExecute(GetDesktopWindow(), _T("open"), iexplore, szArguments, NULL, SW_SHOWNORMAL);

			// Show a error message because flash player is not installed
			MessageBox(NULL, _T("Flash Player ActiveX is not installed. Please install the Flash Player, and run the application again!"), _T("AMC Tool Results Viewer: Error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		}
	}
	else
		MessageBox(NULL, _T("This application can run only on Windows Vista or newer!"), _T("AMC Tool Results Viewer: Error"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

	if( lpFileName != NULL )
	{
		CString file = lpFileName;
		CString ext = file.Right(4);
		ext.MakeLower();
		if( ext == _T(".tmp") )
			_tunlink(lpFileName);
	}

	_Module.Term();
	::CoUninitialize();

	return 0;
}
