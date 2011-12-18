////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AESWizard.cpp
///  Description: The implementation for the wizard interface of the AMC Tool.
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
#include "CAboutDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESPage_Start
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESPage_Start::AESPage_Start(AESWizard *wizard)
{
    mWizard = wizard;
    SetHeaderTitle(_T("AES Encryption Tool"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Start::OnSetActive()
{
    ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL, PSWIZB_CANCEL );

    if( mWizard->mAESHelper->IsCPUCapable() == false && mWizard->mAESHelper->IsGPUCapable() == false )
    {
        CWindow control = this->GetDlgItem(IDC_COMMAND_BENCHMARK);
        control.EnableWindow(FALSE);
        control = this->GetDlgItem(IDC_COMMAND_ENCRYPT);
        control.EnableWindow(FALSE);
        control = this->GetDlgItem(IDC_COMMAND_DECRYPT);
        control.EnableWindow(FALSE);
        MessageBox(_T("No working AES plugin have been found !"), _T("AMC Tool: Error"), MB_OK | MB_ICONERROR);
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Start::OnBnClickedCommandBenchmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    this->mWizard->mBenchmark   = TRUE;
    this->mWizard->mEncrypt     = FALSE;

    this->mWizard->SetActivePage(1);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Start::OnBnClickedCommandEncrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    COpenDialog dialog;
    if( IDOK == dialog.DoModal(mWizard->m_hWnd) )
    {
        CString filePath;
        COM_VERIFY(dialog.GetFilePath(filePath));

        this->mWizard->mBenchmark   = FALSE;
        this->mWizard->mEncrypt     = TRUE;
        SAFE_DELETE(this->mWizard->mInputFile);
        this->mWizard->mInputFile   = new CString(filePath);

        this->mWizard->SetActivePage(2);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Start::OnBnClickedCommandDecrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    COpenDialog dialog;
    if( IDOK == dialog.DoModal(mWizard->m_hWnd) )
    {
        CString filePath;
        COM_VERIFY(dialog.GetFilePath(filePath));

        this->mWizard->mBenchmark   = FALSE;
        this->mWizard->mEncrypt     = FALSE;
        SAFE_DELETE(this->mWizard->mInputFile);
        this->mWizard->mInputFile   = new CString(filePath);

        this->mWizard->SetActivePage(2);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESPage_Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESPage_Properties::AESPage_Properties(AESWizard *wizard)
{
    mWizard = wizard;
    SetHeaderTitle(_T("AES Properties"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Properties::OnInitDialog(UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    this->SendDlgItemMessage(IDC_COMBO_KEYSIZE, CB_ADDSTRING, 0, (LPARAM)_T("128 bits"));
    this->SendDlgItemMessage(IDC_COMBO_KEYSIZE, CB_ADDSTRING, 0, (LPARAM)_T("192 bits"));
    this->SendDlgItemMessage(IDC_COMBO_KEYSIZE, CB_ADDSTRING, 0, (LPARAM)_T("256 bits"));
    this->SendDlgItemMessage(IDC_COMBO_KEYSIZE, CB_SETCURSEL, 0, 0);

    this->SendDlgItemMessage(IDC_COMBO_MODE, CB_ADDSTRING, 0, (LPARAM)_T("ECB"));
    this->SendDlgItemMessage(IDC_COMBO_MODE, CB_ADDSTRING, 0, (LPARAM)_T("CBC"));
    this->SendDlgItemMessage(IDC_COMBO_MODE, CB_ADDSTRING, 0, (LPARAM)_T("CTR"));
    this->SendDlgItemMessage(IDC_COMBO_MODE, CB_SETCURSEL, 0, 0);

    this->GetDlgItem(IDC_EDIT_IV).EnableWindow(FALSE);

    CEdit editDlg = this->GetDlgItem(IDC_EDIT_KEY);
    editDlg.SetCueBannerText(_T("Inset here the key"));

    editDlg = this->GetDlgItem(IDC_EDIT_IV);
    editDlg.SetCueBannerText(_T("Inset here the Initial Vector"));

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Properties::OnSetActive()
{
    ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL,
                       PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_CANCEL );
    EnableWizardButtons( PSWIZB_BACK, 1 );
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
AESPage_Properties::OnWizardBack()
{
    this->mWizard->SetActivePage(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
AESPage_Properties::OnWizardNext()
{
    TCHAR tmpText[33];

    SAFE_DELETE(this->mWizard->mKey);
    this->GetDlgItemText(IDC_EDIT_KEY, tmpText, 33);
    this->mWizard->mKey = new CString(tmpText);

    SAFE_DELETE(this->mWizard->mIV);
    this->GetDlgItemText(IDC_EDIT_IV, tmpText, 17);
    this->mWizard->mIV = new CString(tmpText);

    this->mWizard->mKeySize = this->GetDlgItemInt(IDC_COMBO_KEYSIZE);
    this->GetDlgItemText(IDC_COMBO_MODE, tmpText, 4);
    if( _tcscmp(tmpText, _T("ECB") ) == 0)
        this->mWizard->mMode = 0;
    else if( _tcscmp(tmpText, _T("CBC") ) == 0)
            this->mWizard->mMode = 1;
    else if( _tcscmp(tmpText, _T("CTR") ) == 0)
        this->mWizard->mMode = 2;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Properties::OnCbnSelchangeComboMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // If we have ECB mode selected, we disable the Initial Vector edit box
    CWindow editIV = this->GetDlgItem(IDC_EDIT_IV);
    editIV.EnableWindow(this->SendDlgItemMessage(IDC_COMBO_MODE, CB_GETCURSEL) != 0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESPage_Device
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESPage_Device::AESPage_Device(AESWizard *wizard)
{
    mWizard = wizard;
    SetHeaderTitle(_T("Device Selection"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Device::OnInitDialog(UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    // Get the number of processors on this machine
    TCHAR sNbProc[5] = _T("1");
    GetEnvironmentVariable(_T("NUMBER_OF_PROCESSORS"), sNbProc, 5);
    int maxNbThreads = _tstoi(sNbProc);
    if(maxNbThreads <= 0)
        maxNbThreads = 1;

    for( int i = 1; i <= maxNbThreads; i++ )
    {
        _itot_s(i, sNbProc, 4, 10);
        this->SendDlgItemMessage(IDC_COMBO_CPUTHREADS, CB_ADDSTRING, 0, (LPARAM)sNbProc);
    }
    this->SendDlgItemMessage(IDC_COMBO_CPUTHREADS, CB_SETCURSEL, maxNbThreads - 1, 0);

    this->SendDlgItemMessage(IDC_COMBO_CPUOPT, CB_ADDSTRING, 0, (LPARAM)_T("O0"));
    this->SendDlgItemMessage(IDC_COMBO_CPUOPT, CB_ADDSTRING, 0, (LPARAM)_T("O1"));
    this->SendDlgItemMessage(IDC_COMBO_CPUOPT, CB_ADDSTRING, 0, (LPARAM)_T("O2"));
    this->SendDlgItemMessage(IDC_COMBO_CPUOPT, CB_ADDSTRING, 0, (LPARAM)_T("O3"));
    this->SendDlgItemMessage(IDC_COMBO_CPUOPT, CB_SETCURSEL, 3, 0);

    BOOL CPU_State = this->mWizard->mAESHelper->IsCPUCapable() ? TRUE : FALSE;
    CWindow control = this->GetDlgItem(IDC_COMBO_CPUTHREADS);
    control.EnableWindow(CPU_State);
    control = this->GetDlgItem(IDC_COMBO_CPUOPT);
    control.EnableWindow(CPU_State);
    control = this->GetDlgItem(IDC_COMMAND_USECPU);
    control.EnableWindow(CPU_State);
    Button_SetNote(control.m_hWnd, (LPCTSTR)this->mWizard->mAESHelper->GetCPUDescription());

    BOOL GPU_State = this->mWizard->mAESHelper->IsGPUCapable() ? TRUE : FALSE;
    control = this->GetDlgItem(IDC_COMMAND_USEGPU);
    control.EnableWindow(GPU_State);
    Button_SetNote(control.m_hWnd, (LPCTSTR)this->mWizard->mAESHelper->GetGPUDescription());

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Device::OnSetActive()
{
    ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL,
                       PSWIZB_BACK | PSWIZB_CANCEL );
    EnableWizardButtons( PSWIZB_BACK, 1 );

    if( this->mWizard->mMode == 1 && this->mWizard->mEncrypt == TRUE )
        this->GetDlgItem(IDC_COMMAND_USEGPU).EnableWindow(FALSE);
    else
        this->GetDlgItem(IDC_COMMAND_USEGPU).EnableWindow(TRUE);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
AESPage_Device::OnWizardBack()
{
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Device::OnBnClickedCommandUsecpu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CSaveDialog dialog;
    if( this->mWizard->mEncrypt == TRUE )
        dialog.GetPtr()->SetFileName(*this->mWizard->mInputFile + _T(".aes"));
    else
    {
        CString *ext = new CString(this->mWizard->mInputFile->Right(4));
        ext->MakeLower();
        if( *ext == _T(".aes") )
        {
            CString newName = this->mWizard->mInputFile->Left(this->mWizard->mInputFile->GetLength() - 4);
            dialog.GetPtr()->SetFileName(newName);
        }
    }

    if( IDOK == dialog.DoModal(mWizard->m_hWnd) )
    {
        CString filePath;
        COM_VERIFY(dialog.GetFilePath(filePath));
        SAFE_DELETE(this->mWizard->mOutputFile);
        this->mWizard->mOutputFile  = new CString(filePath);

        this->mWizard->mCPU = TRUE;
        this->mWizard->mCPUNbThreads = this->GetDlgItemInt(IDC_COMBO_CPUTHREADS);

        TCHAR tmpText[5];
        this->GetDlgItemText(IDC_COMBO_CPUOPT, tmpText, 4);
        if( _tcscmp(tmpText, _T("O0") ) == 0)
            this->mWizard->mCPUOptimisation = 0;
        else if( _tcscmp(tmpText, _T("O1") ) == 0)
            this->mWizard->mCPUOptimisation = 1;
        else if( _tcscmp(tmpText, _T("O2") ) == 0)
            this->mWizard->mCPUOptimisation = 2;
        else if( _tcscmp(tmpText, _T("O3") ) == 0)
            this->mWizard->mCPUOptimisation = 3;

        this->mWizard->SetActivePage(4);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESPage_Device::OnBnClickedCommandUsegpu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CSaveDialog dialog;

    if( this->mWizard->mEncrypt == TRUE )
        dialog.GetPtr()->SetFileName(*this->mWizard->mInputFile + _T(".aes"));
    else
    {
        CString *ext = new CString(this->mWizard->mInputFile->Right(4));
        ext->MakeLower();
        if( *ext == _T(".aes") )
        {
            CString newName = this->mWizard->mInputFile->Left(this->mWizard->mInputFile->GetLength() - 4);
            dialog.GetPtr()->SetFileName(newName);
        }
    }

    if( IDOK == dialog.DoModal(mWizard->m_hWnd) )
    {
        CString filePath;
        COM_VERIFY(dialog.GetFilePath(filePath));
        SAFE_DELETE(this->mWizard->mOutputFile);
        this->mWizard->mOutputFile  = new CString(filePath);

        this->mWizard->mCPU = FALSE;
        this->mWizard->mCPUNbThreads = -1;
        this->mWizard->mCPUOptimisation = -1;

        this->mWizard->SetActivePage(4);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESPage_Benchmark
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESPage_Benchmark::AESPage_Benchmark(AESWizard *wizard)
{
    mWizard = wizard;
    SetHeaderTitle(_T("Benchmarking..."));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Benchmark::OnSetActive()
{
    ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL,
                       PSWIZB_BACK | PSWIZB_CANCEL );

    EnableWizardButtons( PSWIZB_BACK, 1 );
    this->SetDlgItemText(IDC_EDIT_LOG, _T(""));
    this->SendDlgItemMessage(IDC_PROGRESS, PBM_SETPOS, 0, 0);

    this->mBenchmark    = mWizard->mBenchmark;
    this->mEncrypt      = mWizard->mEncrypt;
    this->mCPU          = mWizard->mCPU;
    this->mInputFile    = mWizard->mInputFile;
    this->mOutputFile   = mWizard->mOutputFile;
    this->mKey          = mWizard->mKey;
    this->mIV           = mWizard->mIV;
    this->mKeySize      = mWizard->mKeySize;
    this->mMode         = mWizard->mMode;
    this->mCPUNbThreads = mWizard->mCPUNbThreads;
    this->mCPUOptimisation  = mWizard->mCPUOptimisation;

    mWizard->mAESHelper->SetTask(this);
    mWizard->mAESHelper->Start();

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
AESPage_Benchmark::OnWizardBack()
{
    mWizard->mAESHelper->Stop();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Benchmark::OnWizardFinish()
{
    mWizard->mBenchmarkHaveResults = true;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Benchmark::OnQueryCancel()
{
    if( mWizard->mAESHelper->IsRunning() )
    {
        mWizard->mAESHelper->Stop();
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Benchmark::OnProgress(int progress)
{
    if( !mWizard->mAESHelper->IsRunning() )
        return;

    CWindow prgsWindow = this->GetDlgItem(IDC_PROGRESS);
    prgsWindow.PostMessage(PBM_SETPOS, progress, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Benchmark::OnUpdateLog(CString *log)
{
    if( !mWizard->mAESHelper->IsRunning() )
        return;

    CEdit logWindow = this->GetDlgItem(IDC_EDIT_LOG);
    logWindow.SetWindowText(*log);

    int lineCount = logWindow.GetLineCount();
    logWindow.LineScroll(lineCount, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Benchmark::OnFinish()
{
    if( this->mWizard->mAESHelper->IsRunning() )
    {
        // If not canceled
        ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL,
                           PSWIZB_BACK | PSWIZB_FINISH | PSWIZB_CANCEL );

        this->PostMessage(PSM_SETBUTTONTEXT, (WPARAM)PSWIZB_CANCEL, (LPARAM)_T("Close"));
        this->PostMessage(PSM_SETBUTTONTEXT, (WPARAM)PSWIZB_FINISH, (LPARAM)_T("View Results"));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESPage_Process
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESPage_Process::AESPage_Process(AESWizard *wizard)
{
    mWizard = wizard;
    SetHeaderTitle(_T("Processing..."));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Process::OnSetActive()
{
    ShowWizardButtons( PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH | PSWIZB_CANCEL,
                       PSWIZB_BACK | PSWIZB_CANCEL );

    EnableWizardButtons( PSWIZB_BACK, 1 );
    this->SetDlgItemText(IDC_EDIT_LOG, _T(""));
    this->SendDlgItemMessage(IDC_PROGRESS, PBM_SETPOS, 0, 0);
    this->PostMessage(PSM_SETBUTTONTEXT, (WPARAM)PSWIZB_CANCEL, (LPARAM)_T("Cancel"));

    this->mBenchmark        = mWizard->mBenchmark;
    this->mEncrypt          = mWizard->mEncrypt;
    this->mCPU              = mWizard->mCPU;
    this->mInputFile        = mWizard->mInputFile;
    this->mOutputFile       = mWizard->mOutputFile;
    this->mKey              = mWizard->mKey;
    this->mIV               = mWizard->mIV;
    this->mKeySize          = mWizard->mKeySize;
    this->mMode             = mWizard->mMode;
    this->mCPUNbThreads     = mWizard->mCPUNbThreads;
    this->mCPUOptimisation  = mWizard->mCPUOptimisation;

    mWizard->mAESHelper->SetTask(this);
    mWizard->mAESHelper->Start();

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AESPage_Process::OnQueryCancel()
{
    if( mWizard->mAESHelper->IsRunning() )
    {
        mWizard->mAESHelper->Stop();
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int
AESPage_Process::OnWizardBack()
{
    mWizard->mAESHelper->Stop();
    return IDD_PAGE4;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Process::OnProgress(int progress)
{
    if( !mWizard->mAESHelper->IsRunning() )
        return;

    CWindow prgsWindow = this->GetDlgItem(IDC_PROGRESS);
    prgsWindow.PostMessage(PBM_SETPOS, progress, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Process::OnUpdateLog(CString *log)
{
    if( !mWizard->mAESHelper->IsRunning() )
        return;

    CEdit logWindow = this->GetDlgItem(IDC_EDIT_LOG);
    logWindow.SetWindowText(*log);

    int lineCount = logWindow.GetLineCount();
    logWindow.LineScroll(lineCount, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESPage_Process::OnFinish()
{
    this->PostMessage(PSM_SETBUTTONTEXT, (WPARAM)PSWIZB_CANCEL, (LPARAM)_T("Close"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AESWizard
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESWizard::AESWizard() : CAeroWizardFrameImpl<AESWizard>(IDR_MAINFRAME)
{
    this->mBenchmark            = FALSE;
    this->mBenchmarkHaveResults = FALSE;
    this->mEncrypt              = FALSE;
    this->mCPU                  = FALSE;
    this->mInputFile            = NULL;
    this->mOutputFile           = NULL;
    this->mKey                  = NULL;
    this->mIV                   = NULL;
    this->mKeySize              = -1;
    this->mMode                 = -1;
    this->mCPUNbThreads         = -1;
    this->mCPUOptimisation      = -1;

    mPage1 = new AESPage_Start(this);
    mPage2 = new AESPage_Properties(this);
    mPage3 = new AESPage_Benchmark(this);
    mPage4 = new AESPage_Device(this);
    mPage5 = new AESPage_Process(this);

    VERIFY(AddPage(*mPage1));
    VERIFY(AddPage(*mPage3));
    VERIFY(AddPage(*mPage2));
    VERIFY(AddPage(*mPage4));
    VERIFY(AddPage(*mPage5));

    mAESHelper = new AES_Helper(mPage3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AESWizard::OnSheetInitialized()
{
    // set icons
    HICON hIcon = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                      MAKEINTRESOURCE(IDR_MAINFRAME),
                                      IMAGE_ICON,
                                      ::GetSystemMetrics(SM_CXICON),
                                      ::GetSystemMetrics(SM_CYICON),
                                      LR_DEFAULTCOLOR );
    this->SetIcon(hIcon, TRUE);

    HICON hIconSmall = (HICON)::LoadImage( _Module.GetResourceInstance(),
                                           MAKEINTRESOURCE(IDR_MAINFRAME),
                                           IMAGE_ICON,
                                           ::GetSystemMetrics(SM_CXSMICON),
                                           ::GetSystemMetrics(SM_CYSMICON),
                                           LR_DEFAULTCOLOR );
    this->SetIcon(hIconSmall, FALSE);

    CMenu SysMenu = GetSystemMenu(FALSE);
    if( ::IsMenu(SysMenu) )
    {
        SysMenu.AppendMenu(MF_SEPARATOR);
        SysMenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, _T("About..."));
    }

    SysMenu.Detach();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT
AESWizard::OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    UINT uCmdType = (UINT)wParam;
    if( uCmdType == IDM_ABOUTBOX )
    {
        CAboutDlg dlg;
        dlg.DoModal();
        bHandled = TRUE;
    }
    else
        bHandled = FALSE;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AESWizard::~AESWizard()
{
    delete mPage1;
    delete mPage2;
    delete mPage3;
    delete mPage4;
    delete mPage5;
    delete mAESHelper;
    delete mInputFile;
    delete mOutputFile;
    delete mKey;
    delete mIV;
}
