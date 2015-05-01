/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
    #include <objbase.h>
    #include <io.h>
    #if !defined (BENTLEY_WINRT)
        #include <Shlwapi.h>
    #endif
#elif defined (__unix__)
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #define INVALID_HANDLE_VALUE     ((void*)~(intptr_t)0)
#else
    #error unknown compiler
#endif
#include <errno.h>              // for EACCES, etc.
#include "../BentleyInternal.h"
#include <Bentley/Bentley.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeDebugLog.h>

#undef CreateFileW

#define AS_FDES(h)    ((int)(intptr_t)(h))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEYCONFIG_OS_WINDOWS)
static BeFileStatus translateHResultToBeFileStatus (int32_t result)
    {
    switch (result)
        {
        case S_OK:
            return BeFileStatus::Success;

        case ERROR_NOT_LOCKED:
            return BeFileStatus::NotLockedError;

        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
            return BeFileStatus::SharingViolationError;

        case ERROR_ACCESS_DENIED:
            return BeFileStatus::AccessViolationError;

        case ERROR_TOO_MANY_OPEN_FILES:
            return BeFileStatus::TooManyOpenFilesError;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return BeFileStatus::FileNotFoundError;

        case ERROR_DISK_FULL:
            return BeFileStatus::DiskFull;
        }

    return BeFileStatus::UnknownError;
    }
#endif

#if defined (__unix__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toUtf8 (wchar_t const* w)
    {
    Utf8String u;
    BeStringUtilities::WCharToUtf8 (u, w);
    return u;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFile::BeFile ()
    :
    m_mode(BeFileAccess::Read),
    m_handle(INVALID_HANDLE_VALUE),
    m_lastError (BeFileStatus::UnknownError)
    {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeFile::Swap (BeFile& f2)
    {
    std::swap (m_handle,    f2.m_handle);
    std::swap (m_mode,      f2.m_mode);
    std::swap (m_lastError, f2.m_lastError);
    }

#if defined (__unix__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileStatus translateErrnoToBeFileStatus (int e)
    {
    switch (e)
        {
        case 0:
            return BeFileStatus::Success;

        case EACCES:
            return BeFileStatus::AccessViolationError;

        case ENFILE:
            return BeFileStatus::TooManyOpenFilesError;

        case ENOENT:
            return BeFileStatus::FileNotFoundError;

#if defined(ENOSPC)
        case ENOSPC:
            return BeFileStatus::DiskFull;
#endif
        }

    return BeFileStatus::UnknownError;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::SetLastError () const
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    HRESULT r = ::GetLastError();
    BeAssert ((S_OK != r) && "SetLastError called when there is no failure?!");
    return m_lastError = translateHResultToBeFileStatus (r);

#elif defined (BENTLEYCONFIG_OS_UNIX)

    return m_lastError = translateErrnoToBeFileStatus (errno);

#else
#error unknonwn runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::SetPointer (uint64_t pos, BeFileSeekOrigin origin)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    if (::SetFilePointerEx (m_handle, *((LARGE_INTEGER*) &pos), NULL, static_cast<int>(origin)))
        return BeFileStatus::Success;

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX)

    if (pos != (uint32_t)pos)
        return m_lastError = BeFileStatus::UnknownError;

    if (lseek (AS_FDES(m_handle), (uint32_t)pos, (int)origin) != -1)
        return BeFileStatus::Success;

    lseek (AS_FDES(m_handle), 0, SEEK_END); // make sure the next read fails

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::GetPointer (uint64_t& pos)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    LARGE_INTEGER cur;
    cur.QuadPart = 0;
    if (::SetFilePointerEx (m_handle, cur, (LARGE_INTEGER*)&pos, FILE_CURRENT))
        return BeFileStatus::Success;

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX)

    long v = lseek (AS_FDES(m_handle), 0, SEEK_CUR);
    if (v != -1)
        {
        pos = (uint64_t)v;
        return BeFileStatus::Success;
        }

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::GetSize (uint64_t& sz) const
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    FILE_STANDARD_INFO info;
    if (!::GetFileInformationByHandleEx (m_handle, FileStandardInfo, &info, sizeof(info)))
        return SetLastError ();

    sz = info.EndOfFile.QuadPart;

    return BeFileStatus::Success;

#elif defined (BENTLEYCONFIG_OS_UNIX)

    #if defined (__APPLE__)

    struct stat status;

    if (fstat (AS_FDES(m_handle), &status) == 0)
        {
        sz = status.st_size;
        return BeFileStatus::Success;
        }
    #else

    struct stat64 status;

    if (fstat64 (AS_FDES(m_handle), &status) == 0)
        {
        sz = status.st_size;
        return BeFileStatus::Success;
        }
    #endif

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::SetSize (uint64_t pos)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    LARGE_INTEGER lpos;
    lpos.QuadPart = pos;
    if (::SetFilePointerEx (m_handle, lpos, NULL, 0) && ::SetEndOfFile (m_handle))
        return BeFileStatus::Success;

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX)

    #if defined (ANDROID) || defined (__APPLE__)
        if (ftruncate (AS_FDES(m_handle), (uint32_t)pos) == 0)
            return BeFileStatus::Success;
    #else
        if (ftruncate64 (AS_FDES(m_handle), pos) == 0)
            return BeFileStatus::Success;
    #endif

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::Read (void* buffer, uint32_t* bytesRead, uint32_t numBytes)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    uint32_t bytesRead_;
    if (NULL == bytesRead)
        bytesRead = &bytesRead_;

    if (::ReadFile (m_handle, buffer, (DWORD)numBytes, (DWORD*)bytesRead, NULL))
        return BeFileStatus::Success;

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX) 

    uint32_t bytesRead_;
    if (NULL == bytesRead)
        bytesRead = &bytesRead_;

    if (-1 != (*bytesRead = read (AS_FDES(m_handle), buffer, numBytes)))
        return BeFileStatus::Success;

    *bytesRead = 0;
    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::Write (uint32_t* bytesWritten, void const* buf, uint32_t numBytes)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)

    ULONG bytesWritten_;
    if (::WriteFile (m_handle, buf, (ULONG)numBytes, bytesWritten? (ULONG*)bytesWritten: &bytesWritten_, NULL))
        return BeFileStatus::Success;

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX)

    uint32_t bytesWritten_;
    if (NULL == bytesWritten)
        bytesWritten = &bytesWritten_;

    if (-1 != (*bytesWritten = write (AS_FDES(m_handle), buf, numBytes)))
        return BeFileStatus::Success;

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::ReadEntireFile (bvector<Byte>& buffer)
    {
    buffer.clear();

    uint64_t fileSize;
    BeFileStatus stat = GetSize (fileSize);

    if (BeFileStatus::Success != stat)
        return  stat;

    buffer.resize(static_cast <uint32_t> (fileSize));
    return Read(buffer.data(), NULL, static_cast <uint32_t> (fileSize));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::Flush ()
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    if (FlushFileBuffers(m_handle))
        return BeFileStatus::Success;

#elif defined (BENTLEYCONFIG_OS_UNIX)

    if (fsync (AS_FDES(m_handle)) == 0)
        return BeFileStatus::Success;

#else
#error unknown runtime
#endif
    return SetLastError ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::Close()
    {
    if (IsBogus() || !IsOpen())
        return (m_lastError != BeFileStatus::Success)? m_lastError: (m_lastError=BeFileStatus::FileNotOpenError);

#if defined (BENTLEYCONFIG_OS_WINDOWS)

    if (::CloseHandle (m_handle))
        {
        m_handle = INVALID_HANDLE_VALUE;
        return BeFileStatus::Success;
        }

    return SetLastError ();

#elif defined (BENTLEYCONFIG_OS_UNIX)

    BeAssert (AS_FDES(m_handle) > 2);

    if (close (AS_FDES(m_handle)) == 0)
        {
        m_handle = INVALID_HANDLE_VALUE;
        return BeFileStatus::Success;
        }

    return SetLastError ();

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFile::IsOpen() const
    {
    return m_handle != INVALID_HANDLE_VALUE;
    }

#if defined (BENTLEYCONFIG_OS_WINDOWS)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BeFile::GetWinntAccessMode (BeFileAccess m)
    {
    DWORD a = 0;
    if ((m & BeFileAccess::Read) == BeFileAccess::Read)
        a |= GENERIC_READ;
    if ((m & BeFileAccess::Write) == BeFileAccess::Write)
        a |= GENERIC_WRITE;
    return a;
    }

#elif defined (BENTLEYCONFIG_OS_UNIX)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static int getLinuxAccessMode (BeFileAccess m)
    {
    int a;
    if ((m & BeFileAccess::ReadWrite) == BeFileAccess::ReadWrite)
        a = O_RDWR;
    else if ((m & BeFileAccess::Write) == BeFileAccess::Write)
        a = O_WRONLY;
    else
        a = O_RDONLY;

    return a;
    }

#define CREAT_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)


#else
#error unknown runtime
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/00
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus   BeFile::Create (WCharCP filename, bool createAlways)
    {
    m_mode  = BeFileAccess::ReadWrite;

#if defined (BENTLEY_WIN32)

    DWORD   createOptions = createAlways? CREATE_ALWAYS: CREATE_NEW;

    WString fixedFileName;
    BeFileName::FixPathName (fixedFileName, filename, true);

    // grant read and write permissions to others. This is how Unix open works. We want them to be consistent.
    m_handle = ::CreateFileW (fixedFileName.c_str(), GetWinntAccessMode(m_mode), FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, createOptions, FILE_ATTRIBUTE_NORMAL, NULL);

#elif defined (BENTLEY_WINRT)

    DWORD   createOptions = createAlways? CREATE_ALWAYS: CREATE_NEW;

    CREATEFILE2_EXTENDED_PARAMETERS xparms;
    memset (&xparms, 0, sizeof(xparms));
    xparms.dwSize = sizeof(xparms);
    xparms.dwFileFlags = FILE_ATTRIBUTE_NORMAL;

    WString fixedFileName;
    BeFileName::FixPathName (fixedFileName, filename, true);

    m_handle = CreateFile2 (fixedFileName.c_str(), GetWinntAccessMode(m_mode), FILE_SHARE_READ|FILE_SHARE_WRITE, createOptions, &xparms);

#elif defined (BENTLEYCONFIG_OS_UNIX)

    int flags = O_RDWR | O_CREAT;
    if (createAlways)
        flags |= O_TRUNC;
    else
        flags |= O_EXCL;

    m_handle = (void*) open (toUtf8(filename).c_str(), flags, CREAT_MODE);

#else
#error unknown runtime
#endif

    if (IsOpen())
        {
        BeAssert (GetHandle() == m_handle);
        return  BeFileStatus::Success;
        }
    return SetLastError ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus BeFile::Open (WCharCP filename, BeFileAccess mode)
    {
    m_mode  = mode;

#if defined (BENTLEYCONFIG_OS_WINRT)

    CREATEFILE2_EXTENDED_PARAMETERS xparms;
    memset (&xparms, 0, sizeof(xparms));
    xparms.dwSize = sizeof (xparms);
    xparms.dwFileFlags = FILE_ATTRIBUTE_NORMAL;

    m_handle = ::CreateFile2 (filename, GetWinntAccessMode(mode), FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING, &xparms);

#elif defined (BENTLEYCONFIG_OS_WINDOWS)

    // grant read and write permissions to others. This is how Unix open works. We want them to be consistent.
    m_handle = ::CreateFileW (filename, GetWinntAccessMode (mode), FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

#elif defined (BENTLEYCONFIG_OS_UNIX)

    m_handle = (void*) open (toUtf8(filename).c_str(), getLinuxAccessMode (mode));

#else
#error unknown runtime
#endif

    if (IsOpen ())
        {
        BeAssert (GetAccess() == mode);
        BeAssert (GetHandle() == m_handle);
        return  BeFileStatus::Success;
        }
    return SetLastError ();
    }
