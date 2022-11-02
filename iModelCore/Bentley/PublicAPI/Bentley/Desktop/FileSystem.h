/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_NAMESPACE
namespace Desktop {

//=======================================================================================
//! Information about the file system that is implemented only on desktop platforms.
//! @note The functions in this class are not implemented for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
//! @ingroup GROUP_File
//=======================================================================================
struct FileSystem
{
/** @name Temporary files. */
/** @{ */

    //! Get the directory that can be used to store temporary files.
    //! @note This function is not implemented for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetTempPath(BeFileNameR tempPath);

    //! Get a name that can be used to create a file in the specified directory.
    //! @note This function is not implemented for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetTempFileName(BeFileName& tempFileName, BeFileName const& pathName, WCharCP prefixString);
/** @} */

/** @name Query the file system */
/** @{ */
    //! Get free space on disk
    //! @param[out] freeBytes       The amount of free space on the storage device, in bytes
    //! @param[in] dirName          The name of a file or directory on the device to check
    //! @return BeFileNameStatus::Success, if the query succeeded. Non-zero if the directory is invalid or refers to a storage device that does not exist.
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetDiskFreeSpace(uint64_t& freeBytes, BeFileNameCR dirName);

    //! Get free space on disk
    //! @param[out] freeBytes       The amount of free space on the storage device, in bytes
    //! @param[in] dirName          The name of a file or directory on the device to check
    //! @return BeFileNameStatus::Success, if the query succeeded. Non-zero if the directory is invalid or refers to a storage device that does not exist.
    //! @deprecated Use version of BeFileName::BeGetDiskFreeSpace that takes a BeFileNameCR parameter instead
    //! @private
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetDiskFreeSpace(uint64_t& freeBytes, WCharCP dirName);

    //! Get the current working directory
    //! @param[out] currentDirectory The current directory;
    //! @note This function is not useful for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
    //! @private
    BENTLEYDLL_EXPORT static BeFileNameStatus GetCwd (WStringR currentDirectory);
/** @} */

/** @name Executable Paths */
/** @{ */
    //! Get the directory from which the executing program was launched. This is a full path. It does not include the name of the executable program itself.
    //! @note This function is not implemented for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
    BENTLEYDLL_EXPORT static BeFileName GetExecutableDir(BeFileNameCP moduleName = nullptr);

    //! Get the directory from which the executing library was loaded. This is a full path. It does not include the name of the library itself.
    //! @note This function is not implemented for mobile apps. Use DgnPlatformLib::Host::GetKnownLocations for a portable way to identify known locations.
    BENTLEYDLL_EXPORT static BeFileName GetLibraryDir();

    /** @} */
};

}// Desktop
END_BENTLEY_NAMESPACE
