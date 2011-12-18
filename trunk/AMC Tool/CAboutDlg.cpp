////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        CAboutDlg.cpp
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
#include "platform.h"
#include "CAboutDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // center the dialog on the screen
    CenterWindow();

    // set icons
    HICON hIcon = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                      MAKEINTRESOURCE(IDR_MAINFRAME),
                                      IMAGE_ICON,
                                      ::GetSystemMetrics(SM_CXICON),
                                      ::GetSystemMetrics(SM_CYICON),
                                      LR_DEFAULTCOLOR);
    SetIcon(hIcon, TRUE);

    HICON hIconSmall = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                           MAKEINTRESOURCE(IDR_MAINFRAME),
                                           IMAGE_ICON,
                                           ::GetSystemMetrics(SM_CXSMICON),
                                           ::GetSystemMetrics(SM_CYSMICON),
                                           LR_DEFAULTCOLOR);
    SetIcon(hIconSmall, FALSE);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CAboutDlg::OnClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
CAboutDlg::OnNMClickSyslink(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
    PNMLINK pNMLink = (PNMLINK)pNMHDR;
    ShellExecute(NULL, _T("open"), pNMLink->item.szUrl, NULL, NULL, SW_SHOW);
    return 0;
}
