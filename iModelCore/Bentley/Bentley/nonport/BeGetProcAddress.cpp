/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeGetProcAddress.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__unix__)
    #include <dlfcn.h>
#else
    #error "GetProcAddress not supported on this platform"
#endif

#include <Bentley/BeGetProcAddress.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void BeGetProcAddress::SetLibrarySearchPath(BeFileNameCR pathname)
    {
#if defined(_WIN32)
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

#else

    Utf8String ldpath(L"LD_LIBRARY_PATH=");
    ldpath.append(Utf8String(pathname).c_str());
    putenv(strdup(ldpath.c_str()));

#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void* BeGetProcAddress::LoadLibrary(BeFileNameCR libName)
    {
#ifdef _WIN32
    return LoadLibraryW(libName.c_str());
#else
    return dlopen(Utf8String(libName).c_str(), RTLD_LAZY);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void BeGetProcAddress::UnloadLibrary(void* libhandle)
    {
#ifdef _WIN32
    FreeLibrary((HMODULE)libhandle);
#else
    dlclose(libhandle);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson      09/2017
//---------------------------------------------------------------------------------------
void* BeGetProcAddress::GetProcAddress(void* libhandle, Utf8CP procName)
    {
#ifdef _WIN32
    return ::GetProcAddress((HMODULE)libhandle, procName);
#else
    return dlsym(libhandle, procName);
#endif
    }

