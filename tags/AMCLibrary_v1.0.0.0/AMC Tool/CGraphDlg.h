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
#ifndef __INCLUDED_CGRAPHDLG_H__
#define __INCLUDED_CGRAPHDLG_H__

#include "platform.h"
#include "CBenchResults.h"

#import "progid:ShockwaveFlash.ShockwaveFlash" no_namespace raw_interfaces_only exclude("IDispatchEx","IServiceProvider")

#define DLG_TITLE _T("AMC Library Benchmark Results")

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class COpenDialog : public CShellFileOpenDialogImpl<COpenDialog>
{
public:
    COpenDialog()
    {
        COMDLG_FILTERSPEC fileTypes[] = { { L"AES Benchmark Files", L"*.aesbench" }, { L"All Files", L"*.*" }   };
        COM_VERIFY(GetPtr()->SetFileTypes(_countof(fileTypes), fileTypes));
        COM_VERIFY(GetPtr()->SetTitle(L"Open AES benchmark file..."));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSaveDialog : public CShellFileSaveDialogImpl<CSaveDialog>
{
public:
    CSaveDialog() : CShellFileSaveDialogImpl(NULL, FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT | FOS_STRICTFILETYPES,
                                             _T(".aesbench"), NULL, 0U)
    {
        COMDLG_FILTERSPEC fileTypes[] = { { L"AES Benchmark Files", L"*.aesbench" } };
        COM_VERIFY(GetPtr()->SetFileTypes(_countof(fileTypes), fileTypes));
        COM_VERIFY(GetPtr()->SetTitle(L"Save as AES benchmark file..."));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CGraphDlg : public CAxDialogImpl<CGraphDlg>
{
public:
    BEGIN_MSG_MAP(CGraphDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_SIZING, OnSizing)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        COMMAND_HANDLER(IDC_COMBO_GRAPH, CBN_SELCHANGE, OnCbnSelchangeComboGraph)
    END_MSG_MAP()

    enum { IDD = IDD_GRAPHDIALOG };

    CGraphDlg(){ mDlgTitle = DLG_TITLE; }

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSizing(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCbnSelchangeComboGraph(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BOOL    LoadBenchResults(CString fileName);

private:
    void    DisplayGraph(int index);
    void    InitNewBenchResults();

    // The results from the benchmark
    CBenchResults           mBenchResults;

    // The data for the graphs
    std::vector<CString>    mGraphs;

    // The title of the graphs
    std::vector<CString>    mGraphsTitle;

    CString                 mDlgTitle;

    CString                 mBlankSWFPath;
    CString                 mOFCSWFPath;

    // The minimum size of the dialog
    static const int        mMinDlgSizeX = 550;
    static const int        mMinDlgSizeY = 412;

    static LRESULT flashWndProc(HWND, UINT, WPARAM, LPARAM);
    static BOOL CALLBACK FindFlashWindowProc(HWND hwnd, LPARAM lParam);
};

#endif // __INCLUDED_CGRAPHDLG_H__
