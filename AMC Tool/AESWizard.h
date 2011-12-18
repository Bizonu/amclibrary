////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AESWizard.h
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
#ifndef __INCLUDED_AESWIZARD_H__
#define __INCLUDED_AESWIZARD_H__

#include "platform.h"
#include "AES_Helper.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class COpenDialog : public CShellFileOpenDialogImpl<COpenDialog>
{
public:
    COpenDialog()
    {
        COMDLG_FILTERSPEC fileTypes[] = { { L"All Files", L"*.*" }  };
        COM_VERIFY(GetPtr()->SetFileTypes(_countof(fileTypes), fileTypes));
        COM_VERIFY(GetPtr()->SetTitle(L"Open file..."));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSaveDialog : public CShellFileSaveDialogImpl<CSaveDialog>
{
public:
    CSaveDialog()
    {
        COMDLG_FILTERSPEC fileTypes[] = { { L"All Files", L"*.*" }  };
        COM_VERIFY(GetPtr()->SetFileTypes(_countof(fileTypes), fileTypes));
        COM_VERIFY(GetPtr()->SetTitle(L"Save file as..."));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IAESProcessing
{
public:
    virtual void OnProgress(int progress) = 0;
    virtual void OnUpdateLog(CString *log) = 0;
    virtual void OnFinish() = 0;

public:
    BOOL        mBenchmark;
    BOOL        mEncrypt;
    BOOL        mCPU;
    CString     *mInputFile;
    CString     *mOutputFile;
    CString     *mKey;
    CString     *mIV;
    int         mKeySize;
    int         mMode;
    int         mCPUNbThreads;
    int         mCPUOptimisation;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESWizard : public CAeroWizardFrameImpl<AESWizard>
{
public:
    BEGIN_MSG_MAP(AESWizard)
        CHAIN_MSG_MAP(__super)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
    END_MSG_MAP()

    AESWizard();
    ~AESWizard();

    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    void    OnSheetInitialized();

    BOOL                        mBenchmark;
    BOOL                        mBenchmarkHaveResults;
    BOOL                        mEncrypt;
    BOOL                        mCPU;
    CString                     *mInputFile;
    CString                     *mOutputFile;
    CString                     *mKey;
    CString                     *mIV;
    int                         mKeySize;
    int                         mMode;
    int                         mCPUNbThreads;
    int                         mCPUOptimisation;
    class AES_Helper            *mAESHelper;

private:
    class AESPage_Start         *mPage1;
    class AESPage_Properties    *mPage2;
    class AESPage_Benchmark     *mPage3;
    class AESPage_Device        *mPage4;
    class AESPage_Process       *mPage5;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESPage_Start : public CAeroWizardPageImpl<AESPage_Start>
{
public:
    BEGIN_MSG_MAP(AESPage_Start)
        COMMAND_HANDLER(IDC_COMMAND_BENCHMARK, BN_CLICKED, OnBnClickedCommandBenchmark)
        COMMAND_HANDLER(IDC_COMMAND_ENCRYPT, BN_CLICKED, OnBnClickedCommandEncrypt)
        COMMAND_HANDLER(IDC_COMMAND_DECRYPT, BN_CLICKED, OnBnClickedCommandDecrypt)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP()

    enum { IDD = IDD_PAGE1 };

    AESPage_Start(AESWizard *wizard);

    LRESULT OnBnClickedCommandBenchmark(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCommandEncrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCommandDecrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BOOL    OnSetActive();

private:
    AESWizard   *mWizard;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESPage_Properties : public CAeroWizardPageImpl<AESPage_Properties>
{
public:
    BEGIN_MSG_MAP(AESPage_Properties)
        MESSAGE_HANDLER_EX(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_COMBO_MODE, CBN_SELCHANGE, OnCbnSelchangeComboMode)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP()

    enum { IDD = IDD_PAGE2 };

    AESPage_Properties(AESWizard *wizard);

    LRESULT OnCbnSelchangeComboMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BOOL    OnSetActive();
    int     OnWizardBack();
    int     OnWizardNext();

private:
    LRESULT OnInitDialog(UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    AESWizard   *mWizard;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESPage_Device : public CAeroWizardPageImpl<AESPage_Device>
{
public:
    BEGIN_MSG_MAP(AESPage_Device)
        MESSAGE_HANDLER_EX(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_COMMAND_USECPU, BN_CLICKED, OnBnClickedCommandUsecpu)
        COMMAND_HANDLER(IDC_COMMAND_USEGPU, BN_CLICKED, OnBnClickedCommandUsegpu)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP()

    enum { IDD = IDD_PAGE4 };

    AESPage_Device(AESWizard *wizard);

    LRESULT OnBnClickedCommandUsecpu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCommandUsegpu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    BOOL    OnSetActive();
    int     OnWizardBack();

private:
    LRESULT OnInitDialog(UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
    AESWizard   *mWizard;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESPage_Benchmark : public CAeroWizardPageImpl<AESPage_Benchmark>, public IAESProcessing
{
public:
    BEGIN_MSG_MAP(AESPage_Benchmark)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP()

    enum { IDD = IDD_PAGE3 };

    AESPage_Benchmark(AESWizard *wizard);


    virtual void OnProgress(int progress);
    virtual void OnUpdateLog(CString *log);
    virtual void OnFinish();

    BOOL    OnSetActive();
    BOOL    OnQueryCancel();
    BOOL    OnWizardFinish();
    int     OnWizardBack();

private:
    AESWizard   *mWizard;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AESPage_Process : public CAeroWizardPageImpl<AESPage_Process>, public IAESProcessing
{
public:
    BEGIN_MSG_MAP(AESPage_Process)
        CHAIN_MSG_MAP(__super)
    END_MSG_MAP()

    enum { IDD = IDD_PAGE3 };

    AESPage_Process(AESWizard *wizard);

    virtual void OnProgress(int progress);
    virtual void OnUpdateLog(CString *log);
    virtual void OnFinish();

    BOOL    OnSetActive();
    BOOL    OnQueryCancel();
    int     OnWizardBack();

private:
    AESWizard   *mWizard;
};

#endif // __INCLUDED_AESWIZARD_H__
