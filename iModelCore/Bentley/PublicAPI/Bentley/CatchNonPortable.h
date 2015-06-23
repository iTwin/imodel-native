/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/CatchNonPortable.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
/// @cond BENTLEY_SDK_Internal

#ifndef NDEBUG  // These checks are only for a developer build

#if !defined (BENTLEY_NON_PORTABLE)

#if defined (BENTLEY_WIN32)

    // These strange-looking function definitions are designed to catch calls to functions that are defined only by MSVC. They only catch callers if the Bentley namespace is shifted in.
    BEGIN_BENTLEY_NAMESPACE

    struct __this_function_is_not_portable__use_BeStringUtilities_instead;
    int strlwr                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strlwr
    int _strlwr                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strlwr
    int strupr                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strupr
    int _strupr                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strupr
    int stricmp                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Stricmp
    int _stricmp                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Stricmp
    int strnicmp                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strnicmp
    int _strnicmp                   (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strnicmp
    int strdup                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strdup
    int _strdup                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strdup
    int strtok                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strtok
    int _strtok_s                   (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Strtok
    int _snprintf_s                 (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snprintf
    int wcslwr                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcslwr
    int _wcslwr                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcslwr
    int wcsupr                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsupr
    int _wcsupr                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsupr
    int wcsicmp                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsicmp
    int _wcsicmp                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsicmp
    int _wtoi                       (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wtoi
    int wcsdup                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsdup
    int _wcsdup                     (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsdup
    int wcsncpy_s                   (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcsncpy
    int wcstok                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcstok
    int wcstok_s                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcstok
    int _snwprintf_s                (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snwprintf
    int snwprintf                   (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snwprintf
    int _snwprintf                  (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snwprintf
    int swprintf                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snwprintf
    int _swprintf                   (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Snwprintf
    int GetACP                      (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::GetCurrentCodePage
//    int swscanf                 (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BE_STRING_UTILITIES_SWSCANF
    int wcstoul                 (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wcstoul
    int wtoi                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wtoi
    int wtod                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BE_STRING_UTILITIES_SWSCANF
    int wtof                    (__this_function_is_not_portable__use_BeStringUtilities_instead&, ...); //!< Use BeStringUtilities::Wtof

    struct __this_function_is_not_portable__use_BeFile_instead;
    int _wfsopen                    (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Fsopen
    int _wfopen                     (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Fsopen
    int GetLastError                (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::GetLastError
    int SetFilePointer              (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::SetPointer, SetSize
    int SetFilePointerEx            (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::SetPointer, SetSize
    int GetFileSize                 (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::GetPointer
    int GetFileSizeEx               (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::GetPointer
    int ReadFile                    (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Read
    int WriteFile                   (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Write
    int CloseHandle                 (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Close
    int CreateFileW                 (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::Create
    int FormatMessageW              (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::FormatHResult
    int _chsize_s                   (__this_function_is_not_portable__use_BeFile_instead&, ...); //!< Use BeFile::SetSize

    struct __this_function_is_not_portable__use_BeFileName_instead;
    int _wmakepath                  (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BuildName,BeFileName
    int _wmakepath_s                (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BuildName,BeFileName
    int _wsplitpath                 (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::ParseName
    int _wsplitpath_s               (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::ParseName
    int _stati64                    (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::DoesPathExist,GetFileSize,GetFileTime
    int _wstati64                   (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::DoesPathExist,GetFileSize,GetFileTime
    int _wutime64                   (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::SetFileTime
    int GetFileTime                 (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::GetFileTime
    int SetFileTime                 (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::SetFileTime
    int _waccess                    (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::DoesPathExist,CheckAccess
    int PathFileExistsW             (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::DoesPathExist
    int PathIsDirectoryW            (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::IsDirectory
    int PathSkipRootW               (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::ParseName
    int PathCanonicalizeW           (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::FixPathName
    int PathCompactPathExW          (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeCompactPath
    int GetFullPathNameW            (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeGetFullPathName
    int FixPathNameW                (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::FixPathName
    int GetTempFileNameW            (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeGetTempFileName
    int GetTempPathW                (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeGetTempPath
    int SetFileAttributesW          (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::SetFileReadOnly
    int GetFileAttributesW          (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::CheckAccess
    int DeleteFileW                 (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeDeleteFile
    int CreateNewDirectoryW         (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::CreateNewDirectory
    int CloneDirectoryW             (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::CloneDirectory
    int CopyFileW                   (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeCopyFile
    int MoveFileW                   (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeMoveFile
    int GetFileInformationByHandle  (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::???
    int GetDiskFreeSpaceExW         (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::BeGetDiskFreeSpace
    int GetDriveTypeW               (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::IsRemovableMedia
    int PathAddBackslashW           (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::AppendSeparator
    int PathStripToRootW            (__this_function_is_not_portable__use_BeFileName_instead&, ...); //!< Use BeFileName::ParseName

    struct __this_function_is_not_portable__see_BeFileName_h;
    int GetModuleFileNameW          (__this_function_is_not_portable__see_BeFileName_h&, ...); //!< Cannot be used in portable code.
    int _wgetcwd                    (__this_function_is_not_portable__see_BeFileName_h&, ...); //!< Cannot be used in portable code.

    struct __this_function_is_not_portable__use_BeFileListIterator_instead;
    int FindFirstFileW              (__this_function_is_not_portable__use_BeFileListIterator_instead&, ...); //!< Use BeFileListIterator
    int FindFirstFileA              (__this_function_is_not_portable__use_BeFileListIterator_instead&, ...); //!< Use BeFileListIterator

    struct __this_function_is_not_portable__use_BeThreadLocalStorage_instead;
    int TlsAlloc                    (__this_function_is_not_portable__use_BeThreadLocalStorage_instead&, ...); //!< Use BeThreadLocalStorage
    int TlsSetValue                 (__this_function_is_not_portable__use_BeThreadLocalStorage_instead&, ...); //!< Use BeThreadLocalStorage
    int TlsGetValue                 (__this_function_is_not_portable__use_BeThreadLocalStorage_instead&, ...); //!< Use BeThreadLocalStorage

    struct __this_function_is_not_portable__use_BeCriticalSection_instead;
    int InitializeCriticalSection   (__this_function_is_not_portable__use_BeCriticalSection_instead&, ...); //!< Use BeCriticalSection
    int DeleteCriticalSection       (__this_function_is_not_portable__use_BeCriticalSection_instead&, ...); //!< Use BeCriticalSection
    int EnterCriticalSection        (__this_function_is_not_portable__use_BeCriticalSection_instead&, ...); //!< Use BeCriticalSection
    int LeaveCriticalSection        (__this_function_is_not_portable__use_BeCriticalSection_instead&, ...); //!< Use BeCriticalSection
    int GetCurrentThreadId          (__this_function_is_not_portable__use_BeCriticalSection_instead&, ...); //!< Use BeCriticalSection

    struct __this_function_is_not_portable__see_BeThread_h;
    int InterlockedIncrement        (__this_function_is_not_portable__see_BeThread_h&, ...); //!< Use BeThreadUtilities::BeInterlockedIncrement
    int InterlockedDecrement        (__this_function_is_not_portable__see_BeThread_h&, ...); //!< Use BeThreadUtilities::BeInterlockedDecrement
    int Sleep                       (__this_function_is_not_portable__see_BeThread_h&, ...); //!< Use BeThreadUtilities::BeSleep

    struct __this_function_is_not_portable__;
    int VirtualAlloc                (__this_function_is_not_portable__&, ...); //!< Cannot be used in portable code. Use malloc/free instead.
    int VirtualQuery                (__this_function_is_not_portable__&, ...); //!< Cannot be used in portable code. Use malloc/free instead.
    int VirtualProtect              (__this_function_is_not_portable__&, ...); //!< Cannot be used in portable code. Use malloc/free instead.
    int GetSystemInfo               (__this_function_is_not_portable__&, ...); //!< Cannot be used in portable code. Use malloc/free instead.
    int _get_heap_handle            (__this_function_is_not_portable__&, ...); //!< Cannot be used in portable code. Use malloc/free instead.

    struct __this_function_is_not_portable__use_BeTimeUtilities_instead;
    int GetTickCount                (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::QueryPerformanceCounterIn60ths
    int QueryPerformanceCounter     (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::QueryPerformanceCounter
    int QueryPerformanceFrequency   (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::QueryPerformanceCounterFrequency
    int GetSystemTime               (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::GetCurrentTimeAsUnixMillis
    int SystemTimeToFileTime        (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Cannot be used in portable code. Use Unix Millis instead
    int FileTimeToLocalFileTime     (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Cannot be used in portable code. Call BeTimeUtilities::GetCurrentTimeAsUnixMillis and then call BeTimeUtilities::AdjustUnixMillisForLocalTime
    int GetTimeFormatW              (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::UnixMillisToString
    int GetDateFormatW              (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::UnixMillisToString
    int _mktime64                   (__this_function_is_not_portable__use_BeTimeUtilities_instead&, ...); //!< Use BeTimeUtilities::ConvertTmToUnixMillis

    struct __this_function_cannot_be_used_in_portable_code;
    int CoInitialize                (__this_function_cannot_be_used_in_portable_code&, ...); //!< Cannot be used in portable code
    int CoCreateInstance            (__this_function_cannot_be_used_in_portable_code&, ...); //!< Cannot be used in portable code
    int QueryInterface              (__this_function_cannot_be_used_in_portable_code&, ...); //!< Cannot be used in portable code

    END_BENTLEY_NAMESPACE

    #define __super                 __super_is_not_portable_use_T_Super //!< Use DEFINE_T_SUPER, T_Super

#elif defined (__GNUC__)

    // Catch attempts to use functions that are specific to GCC....

#endif

#endif //!defined (BENTLEY_NON_PORTABLE)

#endif//ndef NDEBUG  // These checks are only for a developer build

/// @endcond BENTLEY_SDK_Internal
