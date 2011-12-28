////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        CGraphDlg.h
///  Description: The immplementation of the application that display the AES Benchmark graphs.
///  Author:      Chiuta Adrian Marius
///  Created:     28-01-2010
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
#include "CGraphDlg.h"
#include "CAboutDlg.h"

static WNDPROC gOriginalFlashWindowProc;

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
void
ExtractSWFResource( CString strCustomResName, int nResourceId, CString strOutputName)
{
    HGLOBAL hResourceLoaded;   // handle to loaded resource
    HRSRC   hRes;              // handle/ptr to res. info.
    UINT8   *lpResLock;        // pointer to resource data
    DWORD   dwSizeRes;

    hRes            = FindResource(NULL, MAKEINTRESOURCE(nResourceId), strCustomResName);
    hResourceLoaded = LoadResource(NULL, hRes);
    lpResLock       = (UINT8*)LockResource(hResourceLoaded);
    dwSizeRes       = SizeofResource(NULL, hRes);

    FILE *fout;
    if( _tfopen_s(&fout, strOutputName, _T("wb")) == 0 )
    {
        fwrite(lpResLock, dwSizeRes, 1, fout);
        fclose(fout);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    this->SetWindowText(mDlgTitle);

    // center the dialog on the screen
    CenterWindow();

    // set icons
    HICON hIcon = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                      MAKEINTRESOURCE(IDR_MAINFRAME),
                                      IMAGE_ICON,
                                      ::GetSystemMetrics(SM_CXICON),
                                      ::GetSystemMetrics(SM_CYICON),
                                      LR_DEFAULTCOLOR );
    SetIcon(hIcon, TRUE);

    HICON hIconSmall = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                           MAKEINTRESOURCE(IDR_MAINFRAME),
                                           IMAGE_ICON,
                                           ::GetSystemMetrics(SM_CXSMICON),
                                           ::GetSystemMetrics(SM_CYSMICON),
                                           LR_DEFAULTCOLOR );
    SetIcon(hIconSmall, FALSE);

    // Subclass the flash window, to catch the right click messages
    HWND flashHWND = NULL;
    EnumChildWindows(this->m_hWnd, FindFlashWindowProc, (LPARAM)&flashHWND);
    if( flashHWND != NULL )
    {
        gOriginalFlashWindowProc = (WNDPROC)::GetWindowLongPtr(flashHWND, GWLP_WNDPROC);
        ::SetWindowLongPtr(flashHWND, GWLP_WNDPROC, (LONG_PTR)flashWndProc);
    }

    // Find a temporary names for the SWF files needed by this dialog
    DWORD dwRetVal = 0;
    TCHAR szTempFileName[MAX_PATH];
    TCHAR lpTempPathBuffer[MAX_PATH];
    CString appFolder = getAppFolder(g_hInstance);

    // Gets the temp path env string (no guarantee it's a valid path).
    dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
    if( dwRetVal > MAX_PATH || (dwRetVal == 0) )
        _tcscpy_s(lpTempPathBuffer, MAX_PATH, appFolder);

    // Generates a temporary file name for the first SWF file
    if( GetTempFileName(lpTempPathBuffer, _T("AMCSWF0"), 0, szTempFileName) == 0 )
    {
        _tcscpy_s(szTempFileName, MAX_PATH, lpTempPathBuffer);
        _tcscat_s(szTempFileName, MAX_PATH, _T("\\AMCSWF0.tmp"));
    }
    mBlankSWFPath = szTempFileName;
    ExtractSWFResource( _T("SWF"), IDR_SWF_BLANK, mBlankSWFPath );

    // Generates a temporary file name for the second SWF file
    if( GetTempFileName(lpTempPathBuffer, _T("AMCSWF1"), 0, szTempFileName) == 0 )
    {
        _tcscpy_s(szTempFileName, MAX_PATH, lpTempPathBuffer);
        _tcscat_s(szTempFileName, MAX_PATH, _T("\\AMCSWF1.tmp"));
    }
    mOFCSWFPath = szTempFileName;
    ExtractSWFResource( _T("SWF"), IDR_SWF_OFC, mOFCSWFPath );

    // Add new menus to the system menu
    CMenu SysMenu = GetSystemMenu(FALSE);
    if( ::IsMenu(SysMenu) )
    {
        SysMenu.AppendMenu(MF_SEPARATOR);
        SysMenu.AppendMenu(MF_STRING, IDM_OPENBENCHRESULTS, _T("Load results from file"));
        SysMenu.AppendMenu(MF_STRING, IDM_SAVEBENCHRESULTS, _T("Save results to file"));
        SysMenu.AppendMenu(MF_SEPARATOR);
        SysMenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, _T("About..."));
    }
    SysMenu.Detach();

    // Initialize the benchmark results
    InitNewBenchResults();

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
CGraphDlg::InitNewBenchResults()
{
    mGraphs.clear();
    mGraphsTitle.clear();

    if( mBenchResults.size() > 0 )
    {
        static const TCHAR *Colors[] = { _T("#696969"), _T("#EE9A00"), _T("#9B30FF"), _T("#BCEE68"),
                                         _T("#00CD00"), _T("#00EEEE"), _T("#C6E2FF"), _T("#7A67EE"),
                                         _T("#FFE4E1"), _T("#8B795E"), _T("#FFFF00"), _T("#2F4F4F"),
                                         _T("#483D8B"), _T("#556B2F"), _T("#7CFC00"), _T("#BDB76B"),
                                         _T("#800080")
        };

        for( int iMode = 0; iMode < 3; iMode++ )
        {
            for(int iKeySize = 0; iKeySize < 3; iKeySize++)
            {
                UINT32 keySize = iKeySize == 0 ? 128 : (iKeySize == 1? 192 : 256);
                float ymin = 100000.0f, ymax = -100000.0f;
                TCHAR values[1024];
                CString newGraph;
                newGraph =  _T("variables=true&")
                            _T("bg_colour=#ffffff&")
                            _T("y_legend=Speed MByes/sec,12,#6959CD&")
                            _T("x_legend=Data size,12,#736AFF&")
                            _T("y_label_size=15&")
                            _T("x_axis_steps=1&")
                            _T("y_ticks=5,0,20&")
                            _T("x_labels=256,512,1K,2K,4K,8K,16K,32K,64K,128K,256K,512K,1M,2M,4M,8M,16M,32M,64M&");

                CString title;
                title.Format(_T("title=AES %d in %s mode,{font-size: 20;color: #D2691E}&"), (int)keySize, iMode == 0? _T("ECB") : (iMode == 1? _T("CBC"):_T("CTR")));
                newGraph += title;

                int lineIndex = 0;
                bool graphValid = false;
                for( UINT32 i = 0; i < mBenchResults.size(); i++ )
                {
                    BenchResult *result = &mBenchResults[i];

                    if( result->mAESMode == iMode && result->mAESKeySize == keySize )
                    {
                        CString lineData;
                        CString lineValues;

                        if( lineIndex == 0 )
                        {
                            if( result->mDeviceID == AES_DEVICE_CPU )
                                lineData.Format(_T("area_hollow=2,3,25,%s,%d CPUs,10&"), Colors[lineIndex], result->mSubDeviceID);
                            else
                                lineData.Format(_T("area_hollow=2,3,25,#ff0000,GPU,10&"));

                            lineValues.Format(_T("values="));
                        }
                        else
                        {
                            if( result->mDeviceID == AES_DEVICE_CPU )
                                lineData.Format(_T("area_hollow_%d=2,3,25,%s,%d CPUs,10&"), lineIndex+1, Colors[lineIndex], result->mSubDeviceID);
                            else
                                lineData.Format(_T("area_hollow_%d=2,3,25,#ff0000,GPU,10&"), lineIndex+1);

                            lineValues.Format(_T("values_%d="), lineIndex+1);
                        }

                        _stprintf_s(values, 1024, _T("%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,")
                                                  _T("%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f&"),
                                                  result->mSpeedValues[0],result->mSpeedValues[1],result->mSpeedValues[2],result->mSpeedValues[3],
                                                  result->mSpeedValues[4],result->mSpeedValues[5],result->mSpeedValues[6],result->mSpeedValues[7],
                                                  result->mSpeedValues[8],result->mSpeedValues[9],result->mSpeedValues[10],result->mSpeedValues[11],
                                                  result->mSpeedValues[12],result->mSpeedValues[13],result->mSpeedValues[14],result->mSpeedValues[15],
                                                  result->mSpeedValues[16],result->mSpeedValues[17],result->mSpeedValues[18]
                        );

                        lineValues += values;
                        newGraph += lineData;
                        newGraph += lineValues;
                        lineIndex++;
                        graphValid = true;

                        for( int j = 0; j < 19; j++ )
                        {
                            if( ymin > result->mSpeedValues[j] )
                                ymin = result->mSpeedValues[j];
                            if( ymax < result->mSpeedValues[j] )
                                ymax = result->mSpeedValues[j];
                        }
                    }
                }

                if( graphValid )
                {
                    _stprintf_s(values, 1024, _T("y_max=%.1f&y_min=%.1f&"), ymax, ymin);
                    newGraph += values;
                    mGraphs.push_back(newGraph);

                    title.Format(_T("AES %d in %s mode"), (int)keySize, iMode == 0? _T("ECB") : (iMode == 1? _T("CBC"):_T("CTR")));
                    mGraphsTitle.push_back(title);
                }
            }
        }
    }

    CComboBox comboTitle = this->GetDlgItem(IDC_COMBO_GRAPH);
    { // Clear the combo box
        int count = comboTitle.GetCount();
        for(int i = 0; i < count; i++)
            comboTitle.DeleteString(0);
    }

    if( mGraphsTitle.size() )
    {
        comboTitle.ShowWindow(SW_SHOW);
        for( std::vector<CString>::iterator i = this->mGraphsTitle.begin(); i != this->mGraphsTitle.end(); ++i )
            comboTitle.AddString(*i);
        comboTitle.SetCurSel(0);
    }
    else
        comboTitle.ShowWindow(SW_HIDE);

    if( mBenchResults.GetDeviceDescription(AES_DEVICE_CPU).GetLength() > 0 )
    {
        CString staticCPUText = _T("CPU Description : ");
        staticCPUText += mBenchResults.GetDeviceDescription(AES_DEVICE_CPU);
        CStatic staticCPU = this->GetDlgItem(IDC_STATIC_CPUDESC);
        staticCPU.SetWindowText(staticCPUText);
    }
    else
    {
        CStatic staticCPU = this->GetDlgItem(IDC_STATIC_CPUDESC);
        staticCPU.SetWindowText(_T(""));
    }

    if( mBenchResults.GetDeviceDescription(AES_DEVICE_GPU).GetLength() > 0 )
    {
        CString staticGPUText = _T("GPU Description : ");
        staticGPUText += mBenchResults.GetDeviceDescription(AES_DEVICE_GPU);
        CStatic staticGPU = this->GetDlgItem(IDC_STATIC_GPUDESC);
        staticGPU.SetWindowText(staticGPUText);
    }
    else
    {
        CStatic staticGPU = this->GetDlgItem(IDC_STATIC_GPUDESC);
        staticGPU.SetWindowText(_T(""));
    }
    this->Invalidate(TRUE);

    // Add new menus to the system menu
    CMenu SysMenu = GetSystemMenu(FALSE);
    if( ::IsMenu(SysMenu) )
        SysMenu.EnableMenuItem(IDM_SAVEBENCHRESULTS, MF_BYCOMMAND | ((mGraphsTitle.size() != 0) ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
    SysMenu.Detach();

    DisplayGraph(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
CGraphDlg::DisplayGraph(int index)
{
    // Get an IShockwaveFlash interface on flash player control
    CAxWindow wndFlash = GetDlgItem(IDC_SHOCKWAVEFLASH);
    CComPtr<IShockwaveFlash> Graph;
    wndFlash.QueryControl ( &Graph );

    if( ::PathFileExists( mBlankSWFPath ) == FALSE )
        ExtractSWFResource( _T("SWF"), IDR_SWF_BLANK, mBlankSWFPath );
    Graph->put_Movie( (BSTR)((LPCTSTR)mBlankSWFPath) );

    if( index < (int)mGraphs.size() )
    {
        Graph->put_FlashVars((BSTR)((LPCTSTR)mGraphs[index]));
    }
    else
    {
        Graph->put_FlashVars(_T("variables=true&")
                            _T("bg_colour=#ffffff&")
                            _T("y_legend=Speed MByes/sec,12,#6959CD&")
                            _T("x_legend=Data size,12,#736AFF&")
                            _T("y_label_size=15&")
                            _T("x_axis_steps=1&")
                            _T("y_ticks=5,0,20&")
                            _T("x_labels=256,512,1K,2K,4K,8K,16K,32K,64K,128K,256K,512K,1M,2M,4M,8M,16M,32M,64M&")
                            _T("title=No graph data found,{font-size: 20;color: #D2691E}&")
                            );
    }

    if( ::PathFileExists( mOFCSWFPath ) == FALSE )
        ExtractSWFResource( _T("SWF"), IDR_SWF_OFC, mOFCSWFPath );
    Graph->put_Movie( (BSTR)((LPCTSTR)mOFCSWFPath) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CGraphDlg::LoadBenchResults(CString fileName)
{ 
    mDlgTitle = DLG_TITLE;

    CString file = fileName;
    CString ext = file.Right(9);
    ext.MakeLower();
    if( ext == _T(".aesbench") )
        mDlgTitle += _T(" - ") + fileName.Right(fileName.GetLength() - file.ReverseFind('\\') - 1);

    if( this->m_hWnd != NULL )
        this->SetWindowText(mDlgTitle);

    return mBenchResults.Load(fileName); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnCbnSelchangeComboGraph(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CComboBox comboTitle = this->GetDlgItem(IDC_COMBO_GRAPH);
    DisplayGraph(comboTitle.GetCurSel());   
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    _tunlink(mBlankSWFPath);
    _tunlink(mOFCSWFPath);

    EndDialog(wID);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    UINT uCmdType = (UINT)wParam;
    if( uCmdType == IDM_ABOUTBOX )
    {
        CAboutDlg dlg;
        dlg.DoModal();
        bHandled = TRUE;
    }
    else if( uCmdType == IDM_OPENBENCHRESULTS )
    {
        COpenDialog dialog;
        if ( IDOK == dialog.DoModal(this->m_hWnd) )
        {
            CString filePath;
            COM_VERIFY(dialog.GetFilePath(filePath));
            this->LoadBenchResults(filePath);
            this->InitNewBenchResults();
        }
    }
    else if( uCmdType == IDM_SAVEBENCHRESULTS )
    {
        CSaveDialog dialog;
        if ( IDOK == dialog.DoModal(this->m_hWnd) )
        {
            CString filePath;
            COM_VERIFY(dialog.GetFilePath(filePath));
            this->mBenchResults.Save(filePath);

            mDlgTitle = DLG_TITLE;
            mDlgTitle += _T(" - ") + filePath.Right(filePath.GetLength() - filePath.ReverseFind('\\') - 1);
            if( this->m_hWnd != NULL)
                this->SetWindowText(mDlgTitle);
        }
    }
    else
        bHandled = FALSE;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    this->BeginPaint(&ps);

    RECT cRect;
    this->GetClientRect(&cRect);

    SelectObject (ps.hdc, GetStockObject (WHITE_BRUSH));
    SelectObject (ps.hdc, GetStockObject (NULL_PEN)) ;
    Rectangle(ps.hdc, 0, 0, cRect.right, cRect.bottom);

    this->EndPaint(&ps);
    bHandled = TRUE;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CComboBox comboGraph = this->GetDlgItem(IDC_COMBO_GRAPH);
    CWindow wndFlash = this->GetDlgItem(IDC_SHOCKWAVEFLASH);
    RECT thisRect, comboRect, flashRect;

    this->GetClientRect(&thisRect);

    comboGraph.GetWindowRect(&comboRect);
    ScreenToClient(&comboRect);

    wndFlash.GetWindowRect(&flashRect);
    ScreenToClient(&flashRect);

    UINT32 thisWidth = thisRect.right - thisRect.left, thisHeight = thisRect.bottom - thisRect.top,
           comboWidth = comboRect.right - comboRect.left, comboHeight = comboRect.bottom - comboRect.top;

    comboGraph.MoveWindow(thisWidth - comboWidth, comboRect.top, comboWidth, comboHeight);
    wndFlash.MoveWindow(flashRect.left, flashRect.top, thisWidth - flashRect.left, thisHeight - flashRect.top);

    comboGraph.GetWindowRect(&comboRect);
    ScreenToClient(&comboRect);

    wndFlash.GetWindowRect(&flashRect);
    ScreenToClient(&flashRect);

    // Compute the region that needs to be invalidated
    CRgn invRegion, notInvRegion;
    invRegion.CreateRectRgnIndirect(&thisRect);

    if( comboGraph.IsWindowVisible() )
    {
        notInvRegion.CreateRectRgnIndirect(&comboRect);
        invRegion.CombineRgn(invRegion, notInvRegion, RGN_XOR);
        notInvRegion.DeleteObject();
    }

    notInvRegion.CreateRectRgnIndirect(&flashRect);
    invRegion.CombineRgn(invRegion, notInvRegion, RGN_XOR);
    notInvRegion.DeleteObject();

    this->InvalidateRgn(invRegion);
    invRegion.DeleteObject();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnSizing(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPRECT wndRect = (LPRECT)lParam;
    if( wndRect->right - wndRect->left < mMinDlgSizeX )
    {
        if( wParam == WMSZ_BOTTOMRIGHT || wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT )
            wndRect->right = wndRect->left + mMinDlgSizeX;
        else
            wndRect->left = wndRect->right - mMinDlgSizeX;
    }

    if( wndRect->bottom - wndRect->top < mMinDlgSizeY )
    {
        if( wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT )
            wndRect->bottom = wndRect->top + mMinDlgSizeY;
        else
            wndRect->top = wndRect->bottom - mMinDlgSizeY;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::SetBkMode((HDC)wParam, TRANSPARENT);
    return (LRESULT)GetStockObject( NULL_BRUSH );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphDlg::flashWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if( msg == WM_RBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK )
        return 0; // we don't want these messages to arrive to flash player

    return CallWindowProc(gOriginalFlashWindowProc, hWnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
CALLBACK CGraphDlg::FindFlashWindowProc(HWND hwnd, LPARAM lParam)
{
    TCHAR className[256];
    GetClassName(hwnd, className, _countof(className));
    if( _tcscmp(className, _T("MacromediaFlashPlayerActiveX")) == 0 )
    {
        *((HWND*)lParam) = hwnd;
        return FALSE;
    }

    return TRUE;
}
