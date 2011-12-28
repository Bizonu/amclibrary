////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        CBenchResults.h
///  Description: This class saves or loads the AES benchmark results to/from a file.
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
#ifndef __INCLUDED_CBENCHRESULTS_H__
#define __INCLUDED_CBENCHRESULTS_H__

#include "platform.h"
#include <vector>

#define BENCH_RESULTS_VERSION '1SEA'

#define AES_DEVICE_CPU 0
#define AES_DEVICE_GPU 1

#define BENCH_NB_DATA_SIZE 19
#define BENCH_START_DATA_SIZE 256

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, r1, 2)
class BenchResult
{
public:
    // The ID of the device
    UINT32 mDeviceID;
    // The sub ID of the device ( for CPU is the number of threads used, for GPU is 0 )
    UINT32 mSubDeviceID;
    // The mode of AES : 0 ECB, 1 CBC, 2 CTR
    UINT32 mAESMode;
    // The key size of the AES: 128, 192, 256
    UINT32 mAESKeySize;
    // The speed values (in MBytes/sec) for data size of:
    // 256, 512, 1K, 2K, 4K, 8K, 16K, 32K, 64K, 128K, 256K, 512K, 1M, 2M, 4M, 8M, 16M, 32M, 64M
    float  mSpeedValues[19];
    // Is TRUE if the results are OK, else FALSE
    BOOL   mOK[19];
};
#pragma pack(pop, r1)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CBenchResults
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CBenchResults() { mVersion = BENCH_RESULTS_VERSION; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Clears the results
    void clear()
    {
        mDevices.clear();
        mResults.clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Loads the results from the given file name
    BOOL Load(CString fileName)
    {
        clear();

        FILE *fin;
        if( _tfopen_s(&fin, fileName, _T("rb")) )
            return FALSE;

        UINT32 version;
        if( fread(&version, sizeof(UINT32), 1, fin) != 1 )
            goto _LOAD_FAILED_;
        if( version != mVersion )
            goto _LOAD_FAILED_;

        UINT32 nbDevices;
        if( fread(&nbDevices, sizeof(UINT32), 1, fin) != 1 )
            goto _LOAD_FAILED_;

        for( UINT32 i = 0; i < nbDevices; i++ )
        {
            UINT32 deviceNameSize;
            if( fread(&deviceNameSize, sizeof(UINT32), 1, fin) != 1 )
                goto _LOAD_FAILED_;

            TCHAR *deviceName = new TCHAR[deviceNameSize];
            if( fread(deviceName, deviceNameSize, 1, fin) != 1 )
            {
                delete [] deviceName;
                goto _LOAD_FAILED_;
            }

            CString cdeviceName = deviceName;
            mDevices.push_back(cdeviceName);
            delete [] deviceName;
        }

        UINT32 nbResults;
        if( fread(&nbResults, sizeof(UINT32), 1, fin) != 1 )
            goto _LOAD_FAILED_;

        for( UINT32 i = 0; i < nbResults; i++ )
        {
            BenchResult result;
            if( fread(&result, sizeof(BenchResult), 1, fin) != 1 )
                goto _LOAD_FAILED_;
            mResults.push_back(result);
        }

        fclose(fin);

        return TRUE;

    _LOAD_FAILED_:
        fclose(fin);
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Save the results to the given file name
    BOOL Save(CString fileName)
    {
        if( mDevices.size() == 0 || mResults.size() == 0 )
            return FALSE;

        FILE *fout;
        if( _tfopen_s(&fout, fileName, _T("wb")) )
            return FALSE;

        if( fwrite(&mVersion, sizeof(UINT32), 1, fout) != 1 )
            goto _SAVE_FAILED_;

        UINT32 nbDevices = (UINT32)mDevices.size();
        if( fwrite(&nbDevices, sizeof(UINT32), 1, fout) != 1 )
            goto _SAVE_FAILED_;

        for( UINT32 i = 0; i < nbDevices; i++ )
        {
            UINT32 deviceNameSize = (mDevices[i].GetLength() + 1) * sizeof(TCHAR);
            if( fwrite(&deviceNameSize, sizeof(UINT32), 1, fout) != 1 )
                goto _SAVE_FAILED_;

            if( fwrite((LPCTSTR)mDevices[i], deviceNameSize, 1, fout) != 1 )
                goto _SAVE_FAILED_;
        }

        UINT32 nbResults = (UINT32)mResults.size();
        if( fwrite(&nbResults, sizeof(UINT32), 1, fout) != 1 )
            goto _SAVE_FAILED_;

        for( UINT32 i = 0; i < nbResults; i++ )
            if( fwrite(&mResults[i], sizeof(BenchResult), 1, fout) != 1 )
                goto _SAVE_FAILED_;

        fclose(fout);

        return TRUE;

    _SAVE_FAILED_:
        fclose(fout);
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Return the format version of the results
    UINT32 GetVersion() { return mVersion; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Return the number of devices
    UINT32 GetNbDevices() { return (UINT32)mDevices.size(); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Return the description of the device
    CString GetDeviceDescription(UINT32 deviceID)
    { 
        CString nullString;
        if( deviceID >= mDevices.size() )
            return nullString;
        else
            return mDevices[deviceID]; 
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Adds a new device description. Returns the index/ID of the device.
    UINT32 AddDeviceDescription(CString description)
    {
        mDevices.push_back(description);
        return (UINT32)mDevices.size() - 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Returns the number of results
    UINT32 size() { return (UINT32)mResults.size(); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Adds a new result
    void Push(BenchResult &result) { mResults.push_back(result); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Return the reference to the result with the given index
    BenchResult& operator [](int index) { return mResults[index]; }

private:
    std::vector<BenchResult> mResults;
    std::vector<CString>     mDevices;
    UINT32                   mVersion;
};

#endif // __INCLUDED_CBENCHRESULTS_H__
