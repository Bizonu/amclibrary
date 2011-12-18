////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        CAboutDlg.h
///  Description: The implementation for the About Dialog.
///  Author:      Chiuta Adrian Marius
///  Created:     23-01-2010
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
#ifndef __INCLUDED_CABOUTDLG_H__
#define __INCLUDED_CABOUTDLG_H__

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
    BEGIN_MSG_MAP(CGraphDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnClose)
        COMMAND_ID_HANDLER(IDOK, OnClose)
        NOTIFY_HANDLER(IDC_SYSLINK, NM_CLICK, OnNMClickSyslink)
    END_MSG_MAP()

    enum { IDD = IDD_ABOUTBOX };

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnNMClickSyslink(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};

#endif // __INCLUDED_CABOUTDLG_H__
