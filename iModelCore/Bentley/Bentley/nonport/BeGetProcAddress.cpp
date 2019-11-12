/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__unix__)
    #include <dlfcn.h>
#else
    #error "GetProcAddress not supported on this platform"
#endif

#include <Bentley/Bentley.h>
#include <Bentley/BentleyConfig.h>
#include <Bentley/BeGetProcAddress.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void BeGetProcAddress::SetLibrarySearchPath(BeFileNameCR pathname)
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    /*
    * Note: We use two mechanisms to setup the PATH - it otherwise results in hard to find and/or reproduce bugs 
    * with the converter:   
    *
    * 1. ::SetDllDirectoryW() by itself was unreliable on some machines - ::SetDllDirectoryA() was getting called 
    * from other Dlls not under our control clobbering up the DgnV8 path setup here. So we ended up setting 
    * system PATH environment for these cases (as is done in V8 Microstation)
    * 
    * 2. System PATH by itself wasn't sufficient to load some DLLs like SPAXAcis.dll. I couldn't find a reasonable 
    * explanation for why this is the case. 
    * 
    * See description @ https://msdn.microsoft.com/en-us/library/ms686203(VS.85).aspx
    */
    ::SetDllDirectoryW(pathname.c_str());

    WString newPath(L"PATH=");
    newPath.append(pathname);
    newPath.append(L";");
    newPath.append(::_wgetenv(L"PATH"));
    _wputenv(newPath.c_str());

#elif defined(__unix__)

    Utf8String ldpath(L"LD_LIBRARY_PATH=");
    ldpath.append(Utf8String(pathname).c_str());
    putenv(strdup(ldpath.c_str()));

#else
    BeAssert(false && "platform not supported");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void* BeGetProcAddress::LoadLibrary(BeFileNameCR libName)
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    return LoadLibraryW(libName.c_str());
#elif defined(__unix__)
    return dlopen(Utf8String(libName).c_str(), RTLD_LAZY);
#else
    BeAssert(false && "platform not supported");
    return nullptr;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void BeGetProcAddress::UnloadLibrary(void* libhandle)
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    FreeLibrary((HMODULE)libhandle);
#elif defined(__unix__)
    dlclose(libhandle);
#else
    BeAssert(false && "platform not supported");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void* BeGetProcAddress::GetProcAddress(void* libhandle, Utf8CP procName)
    {
#if defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)
    return ::GetProcAddress((HMODULE)libhandle, procName);
#elif defined(__unix__)
    return dlsym(libhandle, procName);
#else
    BeAssert(false && "platform not supported");
    return nullptr;
#endif
    }

