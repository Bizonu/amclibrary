////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        AMCLibrary.cpp
///  Description: Advanced Modular Cryptographic Library main functions.
///  Author:      Chiuta Adrian Marius
///  Created:     24-10-2009
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
#include "AMCLibrary.h"
#include <io.h>

#ifdef _WIN64
    static const char *functionGetDescription = "?GetDescription@@YAPEAVIAlgorithmDescription@@I@Z";
#else
    static const char *functionGetDescription = "?GetDescription@@YAPAVIAlgorithmDescription@@I@Z";
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @return The version of the library. The library version is in the form:
///         - HIWORD(version) represent the major version
///         - LOWORD(version) represent the minor version
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AMCLIBRARY_API UINT32
AMCGetVersion()
{
    return AMCLIB_VERSION;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Starts the enumeration of all the supported algorithms.
///
/// @param Callback
///                 The pointer to the function that will be called for every supported algorithm.
///                 The callback will have the parameters set to the class that describe the algorithm.
///                 If the callback returns FALSE, then the enumeration will stop even 
///                 if there are more algorithms to enumerate.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AMCLIBRARY_API void
AMCEnumerateAlgorithms(ACMEnumerationCallback Callback)
{
    if( Callback == NULL )
        return;

    _tfinddata_t findData;
    intptr_t findHandle = _tfindfirst(_T("*.amc"), &findData);
    if( findHandle != -1 )
    {
        BOOL aborted = FALSE;
        do
        {
            HMODULE hModule = LoadLibrary(findData.name);
            if( hModule != INVALID_HANDLE_VALUE )
            {
                GetDescriptionProc getDescription = (GetDescriptionProc)GetProcAddress(hModule, functionGetDescription);
                if( getDescription != NULL )
                {
                    IAlgorithmDescription *moduleDescription;
                    moduleDescription = getDescription(AMCLIB_VERSION);
                    if( moduleDescription != NULL )
                        aborted = Callback(*moduleDescription);
                }
                else
                    FreeLibrary(hModule);
            }
        }while( _tfindnext(findHandle, &findData) == 0 && !aborted );
        _findclose(findHandle);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Returns the description for the algorithm identified by the given ID.
///
/// @param classID
///                 The unique ID that identifies the algorithm.
/// @param interfaceID
///                 The unique ID that identifies the interface of the algorithm.
/// @return
///                 The pointer to the algorithm descriptor. If the algorithm is not found, returns NULL.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AMCLIBRARY_API IAlgorithmDescription*
AMCGetAlgorithmDescription(CLASS_ID interfaceID, CLASS_ID classID)
{
    IAlgorithmDescription *moduleDescription = NULL;
    _tfinddata_t findData;
    intptr_t findHandle = _tfindfirst(_T("*.amc"), &findData);
    if( findHandle != -1 )
    {
        BOOL aborted = FALSE;
        do
        {
            HMODULE hModule = LoadLibrary(findData.name);
            if( hModule != INVALID_HANDLE_VALUE )
            {
                GetDescriptionProc getDescription = (GetDescriptionProc)GetProcAddress(hModule, functionGetDescription);
                if( getDescription != NULL )
                {
                    moduleDescription = getDescription(AMCLIB_VERSION);
                    if( moduleDescription != NULL && moduleDescription )
                        aborted = (moduleDescription->ClassID() == classID) && (moduleDescription->InterfaceID() == interfaceID);
                }
                else
                    FreeLibrary(hModule);
            }
        }while( _tfindnext(findHandle, &findData) == 0 && !aborted );
        _findclose(findHandle);
        if( aborted == FALSE )
            moduleDescription = NULL;
    }

    return moduleDescription;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Defines the entry point for the DLL application.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY
DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
