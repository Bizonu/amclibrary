////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  File:        platform.h
///  Description: Platform definitions.
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
#ifndef __INCLUDED_PLATFORM_H__
#define __INCLUDED_PLATFORM_H__

// Change these values to use different versions
#define WINVER          0x0600
#define _WIN32_WINNT    0x0600
#define _WIN32_IE       0x0700
#define _RICHEDIT_VER   0x0200

#ifdef _DEBUG
    #define VERIFY(f)           ATLASSERT(f)
    #define COM_VERIFY(x)       ATLASSERT(SUCCEEDED(x))
#else
    #define VERIFY(f)           ((void)(f))
    #define COM_VERIFY(x)       ((void)(x))
#endif

#define _WTL_USE_CSTRING

#include "atlbase.h"
#include "atlapp.h"

#include "atlcom.h"
#include "atlhost.h"
#include "atlwin.h"
#include "atlctl.h"

#include "atlframe.h"
#include "atlctrls.h"
#include "atldlgs.h"

#include "atlmisc.h"
#include "atlcrack.h"
#include "atlctrls.h"

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
    #define SAFE_DELETE_ARRAY(p) { if(p) { delete [](p); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL; } }
#endif

#if defined _M_IX86
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
    #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

extern CAppModule _Module;
extern HINSTANCE g_hInstance;

#include "resource.h"

#endif // __INCLUDED_PLATFORM_H__
