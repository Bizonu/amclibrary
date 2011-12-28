////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AES_Helper.cpp
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
#include "platform.h"
#include <process.h>
#include <time.h>
#include <stdio.h>
#include "AES_Helper.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BENCHMARK_START                             \
{                                                   \
    LARGE_INTEGER liFrequency;                      \
    LARGE_INTEGER liStartTime;                      \
    LARGE_INTEGER liCurrentTime;                    \
    QueryPerformanceFrequency ( &liFrequency );     \
    QueryPerformanceCounter ( &liStartTime );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define BENCHMARK_END(timeMs)                       \
    QueryPerformanceCounter ( &liCurrentTime );     \
    (timeMs) = ((double)( (liCurrentTime.QuadPart - liStartTime.QuadPart)* (double)1000.0/(double)liFrequency.QuadPart ));\
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define printLog(format, ...)           \
do{                                     \
    CString __tmp;                      \
    __tmp.Format(format, __VA_ARGS__);  \
    *pLog = *pLog + __tmp;              \
    pTask->OnUpdateLog(pLog);           \
}while(0)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_Helper::AES_Helper(IAESProcessing *task)
{
    mTask           = task;
    mIsRunning      = false;
    mAES_CPU        = NULL;
    mAES_GPU        = NULL;
    mRunningEvent   = CreateEvent( NULL, TRUE, FALSE, NULL);

    IAlgorithmDescription *algDesc;
    algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, AES_CPU_ALGORITHM_CLASS_ID);
    if( algDesc != NULL )
        mAES_CPU = dynamic_cast<AES_CPU*>(algDesc->Create());

    algDesc = AMCGetAlgorithmDescription(AES_ALGORITHM_CLASS_ID, AES_GPU_DX10_ALGORITHM_CLASS_ID);
    if( algDesc != NULL )
        mAES_GPU = dynamic_cast<AES_GPU_DX10*>(algDesc->Create());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AES_Helper::~AES_Helper()
{
    if( mIsRunning )
        this->Stop();

    delete this->mAES_CPU;
    delete this->mAES_GPU;

    CloseHandle(mRunningEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_Helper::Start()
{
    if( mIsRunning )
        this->Stop();

    mLog.Empty();

    mIsRunning = true;
    ResetEvent(mRunningEvent);

    if( mTask->mBenchmark )
        _beginthread(RunBenchmark, 0, this);
    else
        _beginthread(RunEncryption, 0, this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_Helper::Stop()
{
    if( mIsRunning )
    {
        mIsRunning = false;
        WaitForSingleObject(mRunningEvent, INFINITE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL
AES_Helper::bMemIsEqual(const void * buf1, const void * buf2, UINT32 count)
{
    if (!count)
        return TRUE;

#if defined (_WIN64)
    const UINT64 *b1 = (UINT64*)buf1,
                 *b2 = (UINT64*)buf2;
    UINT32 c  = count >> 8;

    while ( --c && *b1 == *b2 ){ b1++; b2++; }
#else
    const UINT32 *b1 = (UINT32*)buf1,
                 *b2 = (UINT32*)buf2;
    UINT32 c  = count >> 4;

    while ( --c && *b1 == *b2 ){ b1++; b2++; }
#endif

    return *b1 == *b2 ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_Helper::RunBenchmark(void *pThis)
{
    AES_Helper *thiz = (AES_Helper*)pThis;
    IAESProcessing *pTask = thiz->mTask;
    CString *pLog = &thiz->mLog;
    TCHAR tmpString[256];

    thiz->mBenchResults.clear();

    if( thiz->mAES_CPU != NULL )
        thiz->mBenchResults.AddDeviceDescription( CString(thiz->mAES_CPU->GetDeviceName()) );
    else
        thiz->mBenchResults.AddDeviceDescription( CString(_T("")) );

    if( thiz->mAES_GPU != NULL )
        thiz->mBenchResults.AddDeviceDescription( CString(thiz->mAES_GPU->GetDeviceName()) );
    else
        thiz->mBenchResults.AddDeviceDescription( CString(_T("")) );

#define BENCH_DATA_SIZE (2048 * 2048 * 16)

    // Get the number of processors on this machine
    TCHAR sNbProc[5] = _T("1");
    GetEnvironmentVariable(_T("NUMBER_OF_PROCESSORS"), sNbProc, 5);
    int maxNbThreads = _tstoi(sNbProc);
    if(maxNbThreads <= 0)
        maxNbThreads = 1;

    // Compute the number of steps
    UINT32 maxNbSteps = 10, nbSteps = 0;
    if( thiz->mAES_CPU != NULL )
        maxNbSteps += 3 * 3 * 19 * maxNbThreads;

    if( thiz->mAES_GPU != NULL )
    {
        if( thiz->mAES_CPU != NULL )
            maxNbSteps += 3 * 3 * 19;
        else
            maxNbSteps += 3 * 2 * 19;
    }

    printLog(_T("Starting the benchmark! Please Wait...\r\n"));
    UINT8 *src = (UINT8*)malloc(BENCH_DATA_SIZE);
    UINT8 *tmp = (UINT8*)malloc(BENCH_DATA_SIZE);
    UINT8 *dst = (UINT8*)malloc(BENCH_DATA_SIZE);

    if( dst == NULL || src == NULL || tmp == NULL )
    {
        printLog(_T("Could not allocate %d MBytes of memory.\r\n"), 3 * BENCH_DATA_SIZE / (1024*1024));
        goto _BenchmarkEnded_;
    }

    // Touch the memory, to be sure the whole memory is allocated and no paging will occur when we measure the time
    for(int i = 0; i < BENCH_DATA_SIZE / 4; i++)
    {
        ((UINT32*)src)[i] = 0x2BADF00D;
        ((UINT32*)tmp)[i] = 0x2BADF00D;
        ((UINT32*)dst)[i] = 0x2BADF00D;
    }

    nbSteps += 2;
    pTask->OnProgress((nbSteps * 100) / maxNbSteps);

    printLog(_T("\r\nGenerating %d MBytes of random data...\r\n"), BENCH_DATA_SIZE / (1024*1024));
    srand( 0xAE5 );
    for(int i = 0; i < BENCH_DATA_SIZE / 2; i++)
        ((UINT16*)src)[i] = rand();

    nbSteps += 8;
    pTask->OnProgress((nbSteps * 100) / maxNbSteps);

    // Generate one key
    UINT8 keyData[16];
    srand( 0x1234 );
    for( UINT32 i = 0; i < 16; i++ )
        keyData[i] = rand();

    // Generate the IV
    UINT8 IV[16];
    srand( 0x4321 );
    for( UINT32 i = 0; i < 16; i++ )
        IV[i] = rand();

    static const TCHAR *aesModes[] = { _T("ECB"), _T("CBC"), _T("CTR") };
    int iDevice;        // 2
    int iNbThreads;     // maxNbThreads
    int iMode;          // 3
    int iKeySize;       // 3
    int iDataSize;      // 19

    if( thiz->mAES_CPU != NULL )
    {
        printLog(_T("\r\n###########################################################\r\n"));
        printLog(_T("Starting benchmark for device:\r\n%s\r\n"), thiz->mAES_CPU->GetDeviceName());
        iDevice = 1; // CPU

        //////////////////////////////////////////////////////////////////////////
        for( iMode = 0; iMode < 3; iMode++ )
        {
            IAlgorithmAES::AES_Modes mode = iMode == 0 ? IAlgorithmAES::AES_ECB : (iMode == 1 ? IAlgorithmAES::AES_CBC : IAlgorithmAES::AES_CTR);

            //////////////////////////////////////////////////////////////////////////
            for(iKeySize = 0; iKeySize < 3; iKeySize++)
            {
                AES_Key::AES_KeySize keySize = iKeySize == 0 ? AES_Key::AES_Key128 : (iKeySize == 1? AES_Key::AES_Key192 : AES_Key::AES_Key256);

                thiz->mAES_CPU->SetMode(mode, IV);
                AES_Key key(keyData, keySize);
                thiz->mAES_CPU->SetKey(&key);
                thiz->mAES_CPU->SetNbThreads(maxNbThreads, 0);
                thiz->mAES_CPU->Encrypt(tmp, src, BENCH_DATA_SIZE);

                //////////////////////////////////////////////////////////////////////////
                for( iNbThreads = 1; iNbThreads <= maxNbThreads; iNbThreads++ )
                {
                    thiz->mAES_CPU->SetNbThreads(iNbThreads);

                    BenchResult bResult;
                    bResult.mDeviceID       = AES_DEVICE_CPU;
                    bResult.mSubDeviceID    = iNbThreads;
                    bResult.mAESKeySize     = keySize;
                    bResult.mAESMode        = iMode;

                    printLog(_T("\r\n###########################################################\r\n"));
                    if( iNbThreads == 1 )
                        printLog(_T("Testing AES %d in %s mode using 1 CPU\r\n"), keySize, aesModes[iMode]);
                    else
                        printLog(_T("Testing AES %d in %s mode using %d CPUs\r\n"), keySize, aesModes[iMode], iNbThreads);

                    //////////////////////////////////////////////////////////////////////////
                    for( iDataSize = 0; iDataSize < 19; iDataSize++ )
                    {
                        const UINT32 dataSize = 256 << iDataSize;
                        UINT32 decDataSize;

                        if( iDataSize == 18 )
                            decDataSize = BENCH_DATA_SIZE;
                        else if( iDataSize == 17 )
                            decDataSize = BENCH_DATA_SIZE / 2;
                        else
                            decDataSize = BENCH_DATA_SIZE / 4;

                        UINT32 nbBlocks = decDataSize / dataSize;
                        decDataSize = nbBlocks * dataSize;

                        thiz->mAES_CPU->SetMode(mode, IV);

                        double timeMs;
                    BENCHMARK_START
                        for( UINT32 i = 0; i < nbBlocks; i++ )
                        {
                            UINT32 Offset = i*dataSize;
                            thiz->mAES_CPU->Decrypt(dst + Offset, tmp + Offset, dataSize);
                        }
                    BENCHMARK_END(timeMs)

                        if( thiz->mIsRunning == false )
                            goto _BenchmarkEnded_;

                        bResult.mSpeedValues[iDataSize] = (float)((1000.0f * decDataSize) / timeMs)/(1024*1024);
                        bResult.mOK[iDataSize] = bMemIsEqual(dst, src, decDataSize);

                        if( dataSize < 1024 )
                        {
                            if( bResult.mOK[iDataSize] )
                                _stprintf_s(tmpString, 256, _T("DataSize: %u B - Speed: %.1f MB/sec\r\n"),
                                                            dataSize, bResult.mSpeedValues[iDataSize]);
                            else
                                _stprintf_s(tmpString, 256, _T("DataSize: %u B - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                            dataSize, bResult.mSpeedValues[iDataSize]);
                        }
                        else if( dataSize < 1024 * 1024 )
                        {
                            if( bResult.mOK[iDataSize] )
                                _stprintf_s(tmpString, 256, _T("DataSize: %u KB - Speed: %.1f MB/sec\r\n"),
                                                            dataSize >> 10, bResult.mSpeedValues[iDataSize]);
                            else
                                _stprintf_s(tmpString, 256, _T("DataSize: %u KB - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                            dataSize >> 10, bResult.mSpeedValues[iDataSize]);
                        }else
                        {
                            if( bResult.mOK[iDataSize] )
                                _stprintf_s(tmpString, 256, _T("DataSize: %u MB - Speed: %.1f MB/sec\r\n"),
                                                            dataSize >> 20, bResult.mSpeedValues[iDataSize]);
                            else
                                _stprintf_s(tmpString, 256, _T("DataSize: %u MB - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                            dataSize >> 20, bResult.mSpeedValues[iDataSize]);
                        }
                        printLog(tmpString);

                        nbSteps++;
                        pTask->OnProgress((nbSteps * 100) / maxNbSteps);
                    }
                    thiz->mBenchResults.Push(bResult);
                }
            }
        }
    }

    if( thiz->mAES_GPU != NULL )
    {
        printLog(_T("\r\n###########################################################\r\n"));
        printLog(_T("Starting benchmark for device:\r\n%s\r\n"), thiz->mAES_GPU->GetDeviceName());
        iDevice = 0; // GPU

        //////////////////////////////////////////////////////////////////////////
        for( iMode = 0; iMode < 3; iMode++ )
        {
            IAlgorithmAES::AES_Modes mode = iMode == 0 ? IAlgorithmAES::AES_ECB : (iMode == 1 ? IAlgorithmAES::AES_CBC : IAlgorithmAES::AES_CTR);

            if( mode == IAlgorithmAES::AES_CBC && thiz->mAES_CPU == NULL )
                continue;

            //////////////////////////////////////////////////////////////////////////
            for( iKeySize = 0; iKeySize < 3; iKeySize++ )
            {
                AES_Key::AES_KeySize keySize = iKeySize == 0 ? AES_Key::AES_Key128 : (iKeySize == 1? AES_Key::AES_Key192 : AES_Key::AES_Key256);

                thiz->mAES_GPU->SetMode(mode, IV);
                AES_Key key(keyData, keySize);
                thiz->mAES_GPU->SetKey(&key);
                if( mode == IAlgorithmAES::AES_CBC )
                {
                    thiz->mAES_CPU->SetMode(mode, IV);
                    thiz->mAES_CPU->SetKey(&key);
                    thiz->mAES_CPU->Encrypt(tmp, src, BENCH_DATA_SIZE);
                }
                else
                    thiz->mAES_GPU->Encrypt(tmp, src, BENCH_DATA_SIZE);

                BenchResult bResult;
                bResult.mDeviceID       = AES_DEVICE_GPU;
                bResult.mSubDeviceID    = 0;
                bResult.mAESKeySize     = keySize;
                bResult.mAESMode        = iMode;

                printLog(_T("\r\n###########################################################\r\n"));
                printLog(_T("Testing AES %d in %s mode using the GPU\r\n"), keySize, aesModes[iMode]);

                //////////////////////////////////////////////////////////////////////////
                for( iDataSize = 0; iDataSize < 19; iDataSize++ )
                {
                    const UINT32 dataSize = 256 << iDataSize;
                    UINT32 decDataSize;

                    // Adjust the data that needs to be decrypted, so we will have less data to 
                    // decode in cases where we know that the decryption will take a long time 
                    if( iDataSize < 8 )
                        decDataSize = BENCH_DATA_SIZE >> (8 - iDataSize);
                    else
                        decDataSize = BENCH_DATA_SIZE;

                    UINT32 nbBlocks = decDataSize / dataSize;
                    decDataSize = nbBlocks * dataSize;

                    thiz->mAES_GPU->SetMode(mode, IV);

                    double timeMs;
                    BENCHMARK_START
                    for( UINT32 i = 0; i < nbBlocks; i++ )
                    {
                        UINT32 Offset = i*dataSize;
                        thiz->mAES_GPU->Decrypt(dst + Offset, tmp + Offset, dataSize);
                    }
                    BENCHMARK_END(timeMs)

                    if( thiz->mIsRunning == false )
                        goto _BenchmarkEnded_;

                    bResult.mSpeedValues[iDataSize] = (float)((1000.0f * decDataSize) / timeMs)/(1024*1024);
                    bResult.mOK[iDataSize] = bMemIsEqual(dst, src, decDataSize);

                    if( dataSize < 1024 )
                    {
                        if( bResult.mOK[iDataSize] )
                            _stprintf_s(tmpString, 256, _T("DataSize: %u B - Speed: %.1f MB/sec\r\n"),
                                                        dataSize, bResult.mSpeedValues[iDataSize]);
                        else
                            _stprintf_s(tmpString, 256, _T("DataSize: %u B - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                        dataSize, bResult.mSpeedValues[iDataSize]);
                    }
                    else if( dataSize < 1024 * 1024 )
                    {
                        if( bResult.mOK[iDataSize] )
                            _stprintf_s(tmpString, 256, _T("DataSize: %u KB - Speed: %.1f MB/sec\r\n"),
                                                        dataSize >> 10, bResult.mSpeedValues[iDataSize]);
                        else
                            _stprintf_s(tmpString, 256, _T("DataSize: %u KB - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                        dataSize >> 10, bResult.mSpeedValues[iDataSize]);
                    }else
                    {
                        if( bResult.mOK[iDataSize] )
                            _stprintf_s(tmpString, 256, _T("DataSize: %u MB - Speed: %.1f MB/sec\r\n"),
                                                        dataSize >> 20, bResult.mSpeedValues[iDataSize]);
                        else
                            _stprintf_s(tmpString, 256, _T("DataSize: %u MB - Speed: %.1f MB/sec - Incorrect results!!!\r\n"),
                                                        dataSize >> 20, bResult.mSpeedValues[iDataSize]);
                    }
                    printLog(tmpString);

                    nbSteps++;
                    pTask->OnProgress((nbSteps * 100) / maxNbSteps);
                }
                thiz->mBenchResults.Push(bResult);
            }
        }
    }
    
_BenchmarkEnded_:
    if( thiz->mIsRunning == false )
        printLog(_T("\r\nBenchmarking canceled...\r\n"));

    free(dst);
    free(src);
    free(tmp);

    pTask->OnFinish();
    thiz->mIsRunning = false;
    SetEvent(thiz->mRunningEvent);
    _endthread();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
AES_Helper::RunEncryption(void *pThis)
{
    AES_Helper *thiz = (AES_Helper*)pThis;
    IAESProcessing *pTask = thiz->mTask;
    CString *pLog = &thiz->mLog;
    IAlgorithmAES *aesAlg;

    if( pTask->mEncrypt )
        printLog(_T("Starting the encryption! Please Wait...\r\n"));
    else
        printLog(_T("Starting the decryption! Please Wait...\r\n"));

    if( pTask->mCPU == TRUE )
    {
        thiz->mAES_CPU->SetNbThreads(pTask->mCPUNbThreads);
        switch(pTask->mCPUOptimisation)
        {
        case 0:
            thiz->mAES_CPU->SetOptimization(AES_CPU::AES_CPU_O0); break;
        case 1:
            thiz->mAES_CPU->SetOptimization(AES_CPU::AES_CPU_O1); break;
        case 2:
            thiz->mAES_CPU->SetOptimization(AES_CPU::AES_CPU_O2); break;
        case 3:
            thiz->mAES_CPU->SetOptimization(AES_CPU::AES_CPU_O3); break;
        }

        aesAlg = thiz->mAES_CPU;
    }
    else
        aesAlg = thiz->mAES_GPU;

    if( aesAlg != NULL )
    {
        FILE *fout, *fin;

        if( _tfopen_s(&fin, *pTask->mInputFile, _T("rb")) )
        {
            printLog(_T("Could not open the file %s !\r\n"), (LPCTSTR)*pTask->mInputFile);
            goto _EncryptEnded_;
        }

        if( _tfopen_s(&fout, *pTask->mOutputFile, _T("wb")) )
        {
            fclose(fin);
            printLog(_T("Could not open the file %s !\r\n"), (LPCTSTR)*pTask->mOutputFile);
            goto _EncryptEnded_;
        }

        printLog(_T("\r\nUsing the device:\r\n%s\r\n"), aesAlg->GetDeviceName());

        UINT8 keyData[32] = {0};
        UINT8 ivData[16] = {0};

        INT32 nbKeyChars = (INT32)pTask->mKey->GetLength();
        nbKeyChars = min(nbKeyChars, 32);
        for( int i = 0; i < nbKeyChars; i++ )
            keyData[i] = (UINT8)(*pTask->mKey)[i];

        INT32 nbIVChars = (INT32)pTask->mIV->GetLength();
        nbIVChars = min(nbIVChars, 16);
        for( int i = 0; i < nbIVChars; i++ )
            ivData[i] = (UINT8)(*pTask->mIV)[i];

        switch( pTask->mMode )
        {
        case 0:
            aesAlg->SetMode(IAlgorithmAES::AES_ECB);break;
        case 1:
            aesAlg->SetMode(IAlgorithmAES::AES_CBC, ivData);break;
        case 2:
            aesAlg->SetMode(IAlgorithmAES::AES_CTR, ivData);break;
        }

        AES_Key key(keyData, (AES_Key::AES_KeySize)pTask->mKeySize);
        aesAlg->SetKey(&key);

    #define BUFF_SIZE (1024 * 1024 * 16)
        double timeMs = 0;
        INT64 fileSize, dataSize;
    BENCHMARK_START

        _fseeki64(fin, 0, SEEK_END);
        fileSize = _ftelli64(fin);
        _fseeki64(fin, 0, SEEK_SET);

        // Pad with zero the input data
        dataSize = (fileSize + 15) & (~15);
        fileSize = dataSize;
        UINT8 *buff = (UINT8*)malloc((size_t)min(dataSize, BUFF_SIZE) + 16);

        if(buff != NULL)
        {
            if( pTask->mEncrypt )
            {
                // We have to encrypt the data
                if( pTask->mMode != 2 )
                {
                    // We have the mode ECB or CBC, so we must perform padding on the data
                    while( dataSize > 0 && thiz->mIsRunning )
                    {
                        UINT32 blockSize = (UINT32)min(dataSize, BUFF_SIZE);
                        pTask->OnProgress((int)((100 * (fileSize - dataSize)) / fileSize));
                        dataSize -= blockSize;

                        if( dataSize != 0 )
                            fread(buff, 1, blockSize, fin);
                        else
                        {
                            // Must perform padding
                            UINT32 readBytes = (UINT32)fread(buff, 1, blockSize, fin);
                            readBytes = blockSize - readBytes;

                            if( readBytes != 0 )
                                for( UINT32 i = 1; i <= readBytes; i++ )
                                    buff[blockSize - i] = readBytes;
                            else
                                for( UINT32 i = 0; i <16; i++ )
                                    buff[blockSize + i] = 16;
                        }

                        aesAlg->Encrypt(buff, buff, blockSize);
                        fwrite(buff, 1, blockSize, fout);
                    }
                }
                else
                {
                    // We have CTR, so zero padding will be done, but in the file 
                    // will be written the number of bytes read from the input file
                    while( dataSize > 0 && thiz->mIsRunning )
                    {
                        UINT32 blockSize = (UINT32)min(dataSize, BUFF_SIZE);
                        pTask->OnProgress((int)((100 * (fileSize - dataSize)) / fileSize));
                        dataSize -= blockSize;

                        if( dataSize == 0 )
                            // Must perform zero padding
                            memset(buff + blockSize - 16, 0, 16);
                        UINT32 readBytes = (UINT32)fread(buff, 1, blockSize, fin);

                        aesAlg->Encrypt(buff, buff, blockSize);
                        fwrite(buff, 1, readBytes, fout);
                    }
                }
            }
            else
            {
                // We have to decrypt the date
                if( pTask->mMode != 2 )
                {
                    // We have the mode ECB or CBC, so we must perform padding on the data
                    while( dataSize > 0 && thiz->mIsRunning )
                    {
                        UINT32 blockSize = (UINT32)min(dataSize, BUFF_SIZE);
                        pTask->OnProgress((int)((100 * (fileSize - dataSize)) / fileSize));
                        dataSize -= blockSize;

                        UINT32 readBytes = (UINT32)fread(buff, 1, blockSize, fin);
                        aesAlg->Decrypt(buff, buff, blockSize);

                        if( dataSize == 0 )
                            // Must perform unpadding
                            readBytes -= buff[readBytes - 1];
                        fwrite(buff, 1, readBytes, fout);
                    }
                }
                else
                {
                    // We have CTR, so zero padding will be done, but in the file 
                    // will be written the number of bytes read from the input file
                    while( dataSize > 0 && thiz->mIsRunning )
                    {
                        UINT32 blockSize = (UINT32)min(dataSize, BUFF_SIZE);
                        pTask->OnProgress((int)((100 * (fileSize - dataSize)) / fileSize));
                        dataSize -= blockSize;

                        if( dataSize == 0 )
                            // Must perform zero padding
                            memset(buff + blockSize - 16, 0, 16);
                        UINT32 readBytes = (UINT32)fread(buff, 1, blockSize, fin);

                        aesAlg->Decrypt(buff, buff, blockSize);
                        fwrite(buff, 1, readBytes, fout);
                    }
                }
            }
            free(buff);
        }
        else
        {
            thiz->mIsRunning = false;
            printLog(_T("\r\nCpuld not allocate memory...\r\n"));
        }
    BENCHMARK_END(timeMs);
        pTask->OnProgress(100);

        fclose(fin);
        fclose(fout);
        if( thiz->mIsRunning == false )
        {
            if( pTask->mEncrypt )
                printLog(_T("\r\nEncryption canceled...\r\n"));
            else
                printLog(_T("\r\nDecryption canceled...\r\n"));
            _tremove(*pTask->mOutputFile);
        }
        else
        {
            if(timeMs == 0)
                timeMs = 1;
            TCHAR tmpString[256];

            if( pTask->mEncrypt )
                printLog(_T("\r\nEncryption finished in "));
            else
                printLog(_T("\r\nDecryption finished in "));

            if( timeMs < 1000 )
                _stprintf_s(tmpString, 256, _T("%d ms,\r\n"), (int)timeMs);
            else
                _stprintf_s(tmpString, 256, _T("%.3f s,\r\n"), timeMs/1000.0);
            printLog(tmpString);

            _stprintf_s(tmpString, 256, _T("at a speed of %.1f Mbytes/sec !\r\n"), ((1000.0 * fileSize)/timeMs)/(1024 * 1024));
            printLog(tmpString);
        }
    }

_EncryptEnded_:
    pTask->OnFinish();
    thiz->mIsRunning = false;
    SetEvent(thiz->mRunningEvent);
    _endthread();
}
