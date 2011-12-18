////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_Helper.h
///  Description: An utility class that permits easy AES encryption/decryption and benchmarking.
///  Author:      Chiuta Adrian Marius
///  Created:     17-12-2009
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
#ifndef __INCLUDED_AES_HELPER_H__
#define __INCLUDED_AES_HELPER_H__

#include "platform.h"
#include "IAlgorithmAES.h"
#include "AES_CPU.h"
#include "AES_GPU_DX10.h"
#include "AESWizard.h"
#include "CBenchResults.h"
#include <list>

class AES_Helper
{
public:
    AES_Helper(class IAESProcessing *task);
    ~AES_Helper();
    void Start();
    void Stop();
    void SetTask(IAESProcessing *task) { if(mIsRunning) Stop(); mTask = task; }
    bool IsRunning(){ return mIsRunning; }
    bool IsCPUCapable(){ return mAES_CPU != NULL; }
    bool IsGPUCapable(){ return mAES_GPU != NULL; }

    CString GetCPUDescription()
    {
        if( mAES_CPU != NULL )
            return CString(mAES_CPU->GetDeviceName());
        else
            return CString(_T(""));
    }

    CString GetGPUDescription() 
    { 
        if( mAES_GPU != NULL )
            return CString(mAES_GPU->GetDeviceName());
        else
            return CString(_T(""));
    }

    BOOL SaveBenchResults(CString fileName) { return mBenchResults.Save(fileName); }

private:
    static void RunBenchmark(void *pThis);
    static void RunEncryption(void *pThis);
    static BOOL bMemIsEqual(const void * buf1, const void * buf2, UINT32 count);

    IAESProcessing *mTask;
    volatile bool   mIsRunning;
    HANDLE          mRunningEvent;
    AES_CPU         *mAES_CPU;
    AES_GPU_DX10    *mAES_GPU;
    CString         mLog;
    CBenchResults   mBenchResults;
};

#endif // __INCLUDED_AES_HELPER_H__
