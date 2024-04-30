/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.h"
#include "BeAssert.h"
#include "BeFileName.h"
#include "ByteStream.h"
#include "bvector.h"
#include <stdio.h>

#if defined (BENTLEY_WIN32)
    #define CSS_UTF8    L", ccs=UTF-8"
#else
    #define CSS_UTF8
#endif

BEGIN_BENTLEY_NAMESPACE

//! The possible status returns for the BeFile methods.
//! @ingroup GROUP_File
enum class BeFileStatus
    {
    Success                      = SUCCESS,  //!< The file operation was successful.
    FileNotFoundError,                       //!< Unable to open the file because there is no file of the specified name.
    AccessViolationError,                    //!< The file exists, but is either already open or protected from an open of the type specified.
    SharingViolationError,                   //!< The file exists, but is already open in a sharing mode that does not allow this open.
    TooManyOpenFilesError,                   //!< There are too many files already open.
    FileNotOpenError,                        //!< An operation was attempted on a file that has already been closed.
    NotLockedError,                          //!< The file is not locked.
    ReadError,                               //!< An error during read.
    DiskFull,                                //!< Not enough storage available to complete operation
    FileAlreadyOpened,                       //!< This object already represents an open file; close it first before calling Open or Create again.
    UnknownError                  = ERROR,   //!< An unknown file I/O error occurred.
    };

//! The possible file access values for the Open method.
//! @ingroup GROUP_File
enum class BeFileAccess
    {
    Read  = 1,       //!< Open for read access.
    Write = 2,       //!< Open for write access.
    ReadWrite = 3,   //!< Open for both read and write.
    };

ENUM_IS_FLAGS(BeFileAccess)

//! Possible origin modes for the SetPosition method.
//! @ingroup GROUP_File
enum class BeFileSeekOrigin
    {
    Begin      = 0,    //!< The specified position is relative to the beginning of the file.
    Current    = 1,    //!< The specified position is relative to the current read/write position of the file.
    End        = 2,    //!< The specified position is relative to the end of the file.
    };

//=======================================================================================
//! BeFile represents a file in the native file system. BeFile has functions to open a file
//! and to access its contents and attributes. BeFile reads and writes binary data. It
//! does not transform stored data to/from text. I/O operations are un-buffered.
//! @ingroup GROUP_File
//=======================================================================================
struct BeFile
{
protected:
    void*               m_handle;
    BeFileAccess        m_mode;
    mutable BeFileStatus m_lastError;

    static BENTLEYDLL_EXPORT uint32_t GetWinntAccessMode(BeFileAccess m);

    //! Used only by subclasses.
    BENTLEYDLL_EXPORT BeFileStatus SetLastError() const;

private:
    BeFile(BeFile const&) {BeAssert(false);}  // No!
    BeFile& operator=(BeFile const&) {BeAssert(false); return *this;} // No!

public:
    // use this in place of fopen_s, which is only available under Windows.
    static int Fopen(FILE**file, char const* name, char const* mode) {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    return fopen_s(file, name, mode);
#else
    return (0 == (*file = fopen(name, mode)));
#endif
    }
    //! Construct a BeFile object. @see Create and Open.
    BENTLEYDLL_EXPORT BeFile();

    //! Destroys a BeFile instance.
    ~BeFile() {Close();}

    //! Swap file handles between two BeFile instances.
    //! @param[in] other    The other file.
    BENTLEYDLL_EXPORT void Swap(BeFile& other);

    //! Creates a new disk file
    //! @param[in] filename     The full path of the file to create.
    //! @param[in] createAlways If false, returns a status other than BeFileStatus::Success, and does not create a new file if \a filename already exists. If true, deletes any existing file before creating a new one by the same name.
    //! @return BeFileStatus::Success if the file was created or non-zero if create failed.
    BENTLEYDLL_EXPORT BeFileStatus Create(WCharCP filename, bool createAlways = true);
    BeFileStatus Create(Utf8String filename, bool createAlways = true) {return Create(WString(filename.c_str(), true).c_str(), createAlways);}

    //! Opens an existing file.
    //! @param[in] filename     The full path the file to open.
    //! @param[in] mode         The file open mode.
    //! @return BeFileStatus::Success if the file was opened or non-zero if open failed.
    BENTLEYDLL_EXPORT BeFileStatus Open(WCharCP filename, BeFileAccess mode);
    BeFileStatus Open(Utf8String filename, BeFileAccess mode) {return Open(WString(filename.c_str(), true).c_str(), mode);}

    //! Close the disk file
    BENTLEYDLL_EXPORT BeFileStatus Close();

    //! Query whether the disk file is open.
    BENTLEYDLL_EXPORT bool IsOpen() const;

    //! Gets the access mode used to open the disk file.
    BeFileAccess GetAccess() const {return m_mode;}

    //! Query the native file handle to the open disk file. This is only useful if you want to pass the handle to another
    //! portable function, such as StgOpenStorage.
    void* GetHandle() const {return (void*)(intptr_t)m_handle;}

    //! Gets the status returned by the last operation that failed.
    BeFileStatus GetLastError() const {return m_lastError;}

    //! Moves the file read/write position.
    //! @param[in] position The new read/write position.
    //! @param[in] origin   The origin mode for the move.
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus SetPointer(int64_t position, BeFileSeekOrigin origin);

    //! Gets the file read/write position
    //! @param[out] position      the current read/write position
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus GetPointer(uint64_t& position);

    BENTLEYDLL_EXPORT BeFileStatus ReadEntireFile(bvector<Byte>& buffer);
    BENTLEYDLL_EXPORT BeFileStatus ReadEntireFile(ByteStream& bytestream);

    //! Reads bytes from the file at the current position. The file position is advanced by the number of bytes read.
    //! @param[out] buffer      Data is copied from the file into this buffer.
    //! @param[out] bytesRead   If not NULL, the number of bytes actually read is returned here
    //! @param[in]  numBytes    The number of bytes to read from the file into \a buffer
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus Read(void* buffer, uint32_t* bytesRead, uint32_t numBytes);

    //! Writes bytes to the file at the current position. The file position is advanced by the number of bytes written.
    //! @param[out] bytesWritten    If not NULL, the number of bytes actually written is returned here.
    //! @param[in]  buffer          The data to be written.
    //! @param[in]  numBytes        The number of bytes to be copied from \a buffer into the file.
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus Write(uint32_t* bytesWritten, void const* buffer, uint32_t numBytes);

    //! Writes all of the requested bytes to the file at the current position. The file position is advanced by the number of bytes written.
    //! @param[in]  buffer          The data to be written.
    //! @param[in]  numBytes        The number of bytes to be copied from \a buffer into the file.
    //! @return BeFileStatus::Success if all bytes were successfully written or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus WriteAll(void const* buffer, size_t numBytes);

    //! Flushes all pending writes to the file
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus Flush();

    //! Gets the size of the file.
    //! @param[out] length  The size of the file in bytes
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus GetSize(uint64_t& length) const;

    //! Sets the size of the file.
    //! @param[in] length       The new size of the file in bytes.
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus SetSize(uint64_t length);
};

END_BENTLEY_NAMESPACE
