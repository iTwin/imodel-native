/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeFileName.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <string.h>
#include <stdlib.h>
#include "Bentley.h"
#include "WString.h"
#include "BeStringUtilities.h"

BEGIN_BENTLEY_NAMESPACE

#if defined (BENTLEYCONFIG_OS_WINDOWS) // Windows && WinRT
    #ifndef MAX_PATH
        #define MAX_PATH 260
    #endif
    static CharCP   const  DIR_SEPARATOR             = "\\";
    static char     const  DIR_SEPARATOR_CHAR        = '\\';
    static WCharCP  const  WCSDIR_SEPARATOR          = L"\\";
    static WChar    const  WCSDIR_SEPARATOR_CHAR     = L'\\';
    static char     const  ALT_DIR_SEPARATOR_CHAR    = '/';
    static WChar    const  WCSALT_DIR_SEPARATOR_CHAR = L'/';
    static WCharCP  const  WCSALT_DIR_SEPARATOR      = L"/";
    static CharCP   const  ALL_FILES_FILTER          = "*.*";
    static WCharCP  const  WCSALL_FILES_FILTER       = L"*.*";
    static WCharCP  const  WCSDLL_EXT                = L"dll";
    static WCharCP  const  WCSSTLIB_EXT              = L"lib";
#elif defined (BENTLEYCONFIG_OS_UNIX)
    #define MAX_PATH 1024
    static CharCP   const  DIR_SEPARATOR             = "/";
    static char     const  DIR_SEPARATOR_CHAR        = '/';
    static WCharCP  const  WCSDIR_SEPARATOR          = L"/";
    static WChar    const  WCSDIR_SEPARATOR_CHAR     = L'/';
    static char     const  ALT_DIR_SEPARATOR_CHAR    = '\\';
    static WChar    const  WCSALT_DIR_SEPARATOR_CHAR = L'\\';
    static WCharCP  const  WCSALT_DIR_SEPARATOR      = L"\\";
    static CharCP   const  ALL_FILES_FILTER          = "*";
    static WCharCP  const  WCSALL_FILES_FILTER       = L"*";
    #if defined (__APPLE__)
        static WCharCP  const  WCSDLL_EXT            = L"dylib";
    #else
        static WCharCP  const  WCSDLL_EXT            = L"so";
    #endif
    static WCharCP  const  WCSSTLIB_EXT              = L"a";
#else
    #error unknown platform
#endif
static CharCP   const  PATH_SEPARATOR            = ";";
static char     const  PATH_SEPARATOR_CHAR       = ';';

static WCharCP  const  WCSPATH_SEPARATOR         = L";";
static WChar    const  WCSPATH_SEPARATOR_CHAR    = L';';

/**
* @addtogroup BeFileGroup
* The BentleyApi::BeFileName and BentleyApi::BeFile classes contain cross-platform utilities for working with files.
*/

//! Status codes for BeFileName operations
//! @ingroup BeFileGroup
enum class BeFileNameStatus
    {
    Success             = SUCCESS,  //!< The BeFileName operation was successful
    IllegalName         = 1,        //!< The BeFileName operation failed because an illegal name was used
    AlreadyExists       = 2,        //!< The BeFileName operation failed because the specified file already exists
    CantCreate          = 3,        //!< Unable to create the specified file
    FileNotFound        = 4,        //!< The BeFileName operation failed because the file was not found
    CantDeleteFile      = 5,        //!< Unable to delete the specified file
    AccessViolation     = 6,        //!< The BeFileName operation failed due to access permissions
    CantDeleteDir       = 7,        //!< Unable to delete the specified directory
    UnknownError        = ERROR,    //!< The BeFileName operation failed with an unspecified error
    };

//! Types used when checking for user access rights.
//! @see BeFileName::CheckAccess
//! @ingroup BeFileGroup
enum class BeFileNameAccess
    {
    Read  = 4,              //!< read the file.
    Write = 2,              //!< write to the file.
    ReadWrite = Read | Write, //!< read and write the file.
    };

//=======================================================================================
//! Represents a filename and provides functions to manipulate the filename and to work with files on disk by name.
//! @ingroup BeFileGroup
//=======================================================================================
struct BeFileName : WString
{

public:
/** @name Construct a filename */
/** @{ */
    //! Construct a blank BeFileName
    BeFileName() {}

    //! Construct a BeFileName from a null-terminated wchar string.
    explicit BeFileName(WCharCP name) {SetName(name);}

    //! Construct a BeFileName from a WString.
    explicit BeFileName(WString name) {SetName(name);}

    //! Construct a BeFileName from a UTF8 string.
    explicit BeFileName(Utf8StringCR name) {SetNameUtf8(name);}

    //! Construct a BeFileName from a char string.
    //! @param[in] name The file name to use
    //! @param[in] isUtf8 if false, conversion is done according to the current locale (ACP).
    BENTLEYDLL_EXPORT BeFileName(CharCP name, bool isUtf8);

    //! Construct a BeFileName from a char string.
    //! @param[in] name The file name to use
    //! @param[in] encoding specifies the encoding for conversion purposes.
    BENTLEYDLL_EXPORT BeFileName(CharCP name, BentleyCharEncoding encoding);

    //! Supply default values for parts of a file name.
    //! If the existing name includes a value for a part of the name, that value is preserved. Otherwise the default is used.
    BENTLEYDLL_EXPORT void SupplyDefaultNameParts(WCharCP defaultName);

    //! Override values for parts of a file name.
    //! If the override name includes a value for a part of the name, that value is used. Otherwise the current value is preserved.
    BENTLEYDLL_EXPORT void OverrideNameParts(WCharCP overrideName);

    enum FileNameParts {Device=1, Directory=2, Basename=4, Extension=8, DevAndDir=(Device|Directory), NameAndExt=(Basename|Extension), All=(DevAndDir|NameAndExt) };
    //! Construct a BeFileName from parts of another full filename. Only the parts of the name where the corresponding argument is true are copied.
    //! @param[in] mask     Mask of the parts of fullName that will used to construct this BeFileName
    //! @param[in] fullName The full pathname from which this BeFileName is to be constructed.
    BENTLEYDLL_EXPORT BeFileName(FileNameParts mask, WCharCP fullName);

    //! Construct a BeFileName from parts of a pathname. Any values that are NULL are omitted.
    BeFileName(WCharCP dev, WCharCP dir, WCharCP name, WCharCP ext) {BuildName(dev, dir, name, ext);}

    //! Copy a filename
    BeFileName(BeFileName const& from) {*this = from;}

    // Assign a filename
    // BeFileName& operator=(BeFileName const& from) { *this = from; return *this;}

    //! Clear the value of this BeFileName.
    void Clear() {clear();}

    //! Change the value of this BeFileName.
    //! @param[in] name The new value for this BeFileName. If NULL, the value is cleared.
    BENTLEYDLL_EXPORT void SetName(WCharCP name);
    void SetName(WStringCR name) {assign(name);}

    //! Build a BeFileName from parts of a pathname. Any values that are NULL are omitted.
    BENTLEYDLL_EXPORT void BuildName(WCharCP dev, WCharCP dir, WCharCP name, WCharCP ext);

    //! Build a WString from parts of a pathname. Any values that are NULL are omitted.
    BENTLEYDLL_EXPORT static void BuildName(WStringR combinedPath, WCharCP dev, WCharCP dir, WCharCP name, WCharCP ext);

    //! Append \a additionComponent to this filepath. A directory separator character is inserted, if necessary, before \a additionComponent.
    BeFileNameR AppendToPath(BeFileNameCR additionComponent) {return AppendToPath(additionComponent.GetName());}

    //! Append \a additionComponent to this filepath. A directory separator character is inserted, if necessary, before \a additionComponent.
    BENTLEYDLL_EXPORT BeFileNameR AppendToPath(WCharCP additionComponent);

    //! Append \a extension to this filepath. A period is inserted, if necessary, before \a extension.
    BENTLEYDLL_EXPORT BeFileNameR AppendExtension(WCharCP extension);

    //! Append a path component to a dir
    BENTLEYDLL_EXPORT static void AppendToPath(WStringR dir, WCharCP additionComponent);

    //! Make sure \a dir ends with a directory separator
    BENTLEYDLL_EXPORT static void AppendSeparator(WStringR dir);

    //! Make sure this filename ends with a directory separator
    BENTLEYDLL_EXPORT void AppendSeparator();

    //! Append string.
    void AppendString(WCharCP string) {append(string);}

    //! Remove enclosing quotes
    BENTLEYDLL_EXPORT void RemoveQuotes();

    //! Creates a shortened version of this path by removing leading path components until maxLength is reached. The resulting string is NOT a valid path, but is suitable for UI presentations. If the path is shortened, "..." will be prepended.
    //! @note Result is undefined when maxLength is less than or equal to 3.
    BENTLEYDLL_EXPORT WString Abbreviate(size_t maxLength) const;
/** @} */

/** @name Convert to/from other encodings */
/** @{ */
    //! Copy name into WChar buffer.
    void GetName(WCharP name, size_t size) const {wcsncpy(name, c_str(), size-1); name[size-1] = 0;}

    //! Convert the filename into a null-terminated multibyte string according to the current locale (ACP).
    //! @param[out] name The buffer into which the conversion is made. Must be of size MAX_PATH.
    //! @private
    BENTLEYDLL_EXPORT void GetNameA (char name[MAX_PATH]) const;

    //! Convert the filename into a null-terminated multibyte string according to the current locale (ACP).
    //! @param[out] name The buffer into which the conversion is made. Must be numChars bytes.
    //! @param[in] numBytes Number of bytes in name.
    //! @private
    BENTLEYDLL_EXPORT void GetNameA (CharP name, size_t numBytes) const;

    //! Get the UTF8 encoded name from this BeFileName.
    BENTLEYDLL_EXPORT Utf8String GetNameUtf8() const;

    //! Get the URI for this BeFileName.
    BENTLEYDLL_EXPORT Utf8String GetUri() const;

    //! Change the value of this BeFileName using a multibyte null terminated string in the current locale.
    //! @private
    BENTLEYDLL_EXPORT void SetNameA (CharCP name);

    //! Change the value of this BeFileName using a UTF8 encoded string.
    void SetNameUtf8(Utf8String name) {SetNameUtf8(name.c_str());}

    //! Change the value of this BeFileName using a UTF8 encoded string.
    BENTLEYDLL_EXPORT void SetNameUtf8(Utf8CP name);
/** @} */

/** @name Parse a filename */
/** @{ */
    //! Parse a full filename into WStrings for the device, directory, filename, and extension.
    //! @param[out] dev The device part of this filename. May be NULL.
    //! @param[out] dir The directory part of this filename. May be NULL.
    //! @param[out] name The filename part of this filename. May be NULL.
    //! @param[out] ext The extension part of this filename. May be NULL.
    //! @param[in] fullFileName The filename to parse.
    BENTLEYDLL_EXPORT static void ParseName(WStringP dev, WStringP dir, WStringP name, WStringP ext, WCharCP fullFileName);

    //! Parse a full filename into WStrings for the device, directory, filename, and extension.  If the
    //! input does not contain a file part, the output is not changed.
    //! @param[out] dev The device part of this filename. May be NULL.
    //! @param[out] dir The directory part of this filename. May be NULL.
    //! @param[out] name The filename part of this filename. May be NULL.
    //! @param[out] ext The extension part of this filename. May be NULL.
    //! @param[in] fullFileName The filename to parse.
    BENTLEYDLL_EXPORT static void ParseNameNoClear(WStringP dev, WStringP dir, WStringP name, WStringP ext, WCharCP fullFileName);

    //! Parse the value of this BeFileName into WStrings for the device, directory, filename, and extension.
    //! @param[out] dev The device part of this filename. May be NULL.
    //! @param[out] dir The directory part of this filename. May be NULL.
    //! @param[out] name The filename part of this filename. May be NULL.
    //! @param[out] ext The extension part of this filename. May be NULL.
    void ParseName(WStringP dev, WStringP dir, WStringP name, WStringP ext) const {ParseName(dev, dir, name, ext, c_str());}

    //! Parse the value of this BeFileName into WStrings for the device, directory, filename, and extension.
    //! If the input does not contain a file part, the output is not changed.
    //! @param[out] dev The device part of this filename. May be NULL.
    //! @param[out] dir The directory part of this filename. May be NULL.
    //! @param[out] name The filename part of this filename. May be NULL.
    //! @param[out] ext The extension part of this filename. May be NULL.
    void ParseNameNoClear(WStringP dev, WStringP dir, WStringP name, WStringP ext) const {ParseNameNoClear(dev, dir, name, ext, c_str());}

    //! Remove the right-most component from the path. If this BeFileName contains a single directory (or otherwise contains no directory separators), it becomes empty as a result of this function.
    BENTLEYDLL_EXPORT void PopDir();

    //! Returns the complete directory (including drive, if any) from path. Has terminating separator.
    //! @deprecated Use instance version of BeFileName::GetDirectoryName instead
    BENTLEYDLL_EXPORT static WString GetDirectoryName(WCharCP path);

    //! Returns the complete directory (including drive, if any) from this BeFileName. Has terminating separator.
    BENTLEYDLL_EXPORT BeFileName GetDirectoryName() const;

    //! Returns the directory (excluding drive) from path. Has terminating separator.
    //! @deprecated Use instance version of BeFileName::GetDirectoryWithoutDevice instead
    BENTLEYDLL_EXPORT static WString GetDirectoryWithoutDevice(WCharCP path);

    //! Returns the directory (excluding drive) from this BeFileName. Has terminating separator.
    BENTLEYDLL_EXPORT BeFileName GetDirectoryWithoutDevice() const;

    //! Returns the filename extension from path (everything following the last dot non-inclusive), or empty if there is none.
    //! @deprecated Use instance version of BeFileName::GetExtension instead
    BENTLEYDLL_EXPORT static WString GetExtension(WCharCP path);

    //! Returns the filename extension from this BeFileName (everything following the last dot non-inclusive), or empty if there is none.
    BENTLEYDLL_EXPORT WString GetExtension() const;

    //! Returns the filename from path, including extension, but not directory.
    //! @deprecated Use instance version of BeFileName::GetFileNameAndExtension instead
    static WString GetFileNameAndExtension(WCharCP path) {return WString(BeFileName(NameAndExt, path));}

    //! Returns the filename from this BeFileName, including extension, but not directory.
    WString GetFileNameAndExtension() const {return WString(BeFileName(NameAndExt, GetName()).GetName());}

    //! Returns the base filename from path, with no directory or extension.
    //! @deprecated Use instance version of BeFileName::GetFileNameWithoutExtension instead
    BENTLEYDLL_EXPORT static WString GetFileNameWithoutExtension(WCharCP path);

    //! Returns the base filename from this BeFileName, with no directory or extension.
    BENTLEYDLL_EXPORT WString GetFileNameWithoutExtension() const;

    //! Returns the device letter from path - Empty if the path doesn't start with a device.
    //! @note Always empty on unix.
    //! @deprecated Use instance version of BeFileName::GetDevice instead
    BENTLEYDLL_EXPORT static WString GetDevice(WCharCP path);

    //! Returns the device from this BeFileName - Empty if the path doesn't start with a device. 
    //! @note Always empty on unix.
    BENTLEYDLL_EXPORT WString GetDevice() const;
/** @} */

/** @name Resolve paths and work with relative paths */
/** @{ */
    //! Finds the relative path from a root file to the target file.
    //! If the files are on different devices, targetFileName is copied to relativePath.
    //! @param        relativePath    OUT the relative path to the target file
    //! @param        targetFileName  IN  the full file specification of the file the relative path is needed for.
    //! @param        rootFileName    IN  the full file specification of the root for the relative path.
    BENTLEYDLL_EXPORT static void FindRelativePath(WStringR relativePath, WCharCP targetFileName, WCharCP rootFileName);

    //! Generates a fullpath by resolving relativeFileName relative to basePath.
    //! @param        fullPath      OUT the full path resolved from the inputs.
    //! @param        relativeFileName IN  the relative file name to resolve.
    //! @param        basePath         IN  used as a root for the expansion.
    //! @return       non-zero error status if not successful
    //! @remarks      This function only performs a string manipulation.  It does not
    //!               require or make any use of an actual file.
    BENTLEYDLL_EXPORT static BentleyStatus ResolveRelativePath(WStringR fullPath, WCharCP relativeFileName, WCharCP basePath);

    //! Perform any platform-specific fixes of an input pathname into a "canonical" pathname. On Windows, this includes
    //! converting forward slashes to backslashes and removing double backslashes. It also removes ".\" and "..\" notation.
    //! On other platforms those rules may be different.
    //! @param[out] path The "fixed" version of the pathname.
    //! @param[in] original The "raw" pathname.
    //! @param[in] keepTrailingSeparator if true, keep trailing separator if present
    //! @return BeFileNameStatus::Success if the pathname was successfully "fixed" (whether any changes were made or not), or PATH_IllegalName if
    //! the pathname is invalid for any reason.
    //! @private
    BENTLEYDLL_EXPORT static BeFileNameStatus FixPathName(WStringR path, WCharCP original, bool keepTrailingSeparator=true);

    //! Get the full name of a file
    //! @remarks this function does not check that the file or path exists
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetFullPathName(WStringR path, WCharCP src);

    //! Convert this to a full name
    //! @remarks this function does not check that the file or path exists
    BENTLEYDLL_EXPORT BeFileNameStatus BeGetFullPathName();
/** @} */

/** @name Resolve paths and work with relative paths */
/** @{ */
    //! Determines if this path is a symbolic link. If the path does not exist, also returns false.
    BENTLEYDLL_EXPORT bool IsSymbolicLink() const;

    //! Determines if the specified path is a symbolic link. If the path does not exist, also returns false.
    //! @deprecated Use instance version of BeFileName::IsSymbolicLink instead
    BENTLEYDLL_EXPORT static bool IsSymbolicLink(WCharCP);

    //! Determines a symbolic link's target. This recurses, meaning that it keeps evaluating each incremental target until it reaches a concrete file, or the target does not exist.
    BENTLEYDLL_EXPORT static BentleyStatus GetTargetOfSymbolicLink(BeFileNameR, WCharCP);

    //! Determines a symbolic link's target. If told to recurse, keeps evaluating each incremental target until it reaches a concrete file, or the target does not exist.
    BENTLEYDLL_EXPORT static BentleyStatus GetTargetOfSymbolicLink(BeFileNameR, WCharCP, bool shouldRecurse);
/** @} */

/** @name Test filenames */
/** @{ */
    //! Tests whether this %BeFileName refers to an existing file or directory.
    //! @return true if this %BeFileName refers to an existing file or directory.
    bool DoesPathExist() const {return DoesPathExist(GetName());}

    //! Test whether a pathname refers to an existing file or directory.
    //! @param path The pathname to test.
    //! @return true if the pathname refers to an existing file or directory.
    //! @deprecated Use instance version of BeFileName::DoesPathExist instead
    BENTLEYDLL_EXPORT static bool DoesPathExist(WCharCP path);

    //! Test if a filename is an absolute path.
    BENTLEYDLL_EXPORT bool IsAbsolutePath() const;

    //! Test if a filename is an URL.
    BENTLEYDLL_EXPORT bool IsUrl() const;

    //! Test if a filename is an URL.
    BENTLEYDLL_EXPORT static bool IsUrl(WCharCP filename);

    //! Get the name from this BeFileName.
    WCharCP GetName() const {return c_str();}

    operator WCharCP() const {return c_str();}

    //! Determines if the value of this BeFileName is empty (blank).
    bool IsEmpty() const { return empty(); }

    //! Get name length
    size_t GetNameSize() const { return size(); }
/** @} */

/** @name Work with directories */
/** @{ */
    //! Test whether this pathname refers to an existing directory.
    //! @return true if the pathname refers to an existing directory.
    BENTLEYDLL_EXPORT bool IsDirectory() const;

    //! Test whether the specified pathname refers to an existing directory.
    //! @param path The pathname to test.
    //! @return true if the pathname refers to an existing directory.
    //! @deprecated Use instance version of BeFileName::IsDirectory instead
    BENTLEYDLL_EXPORT static bool IsDirectory(WCharCP path);

    //! Create a new directory.
    //! @param[in] path The pathname for the new directory.
    //! @return BeFileNameStatus::Success if the directory was successfully created.
    //! @note this method will return an error if the directory already exists.
    BENTLEYDLL_EXPORT static BeFileNameStatus CreateNewDirectory(WCharCP path);

    //! Recursively empty all of the contents of an existing directory and then remove it.
    //! @param[in] path The pathname for the directory to remove.
    //! @return BeFileNameStatus::Success if the directory was successfully removed.
    BENTLEYDLL_EXPORT static BeFileNameStatus EmptyAndRemoveDirectory(WCharCP path);

    //! Clone the contents of an existing directory, and optionally its subdirectories, into a new directory.
    //! @param[in] destDir The name for the new directory.
    //! @param[in] sourceDir The source directory to clone.
    //! @param[in] includeSubDirs If true, also clone all subdirectories, recursively.
    //! @return BeFileNameStatus::Success if the directory was successfully cloned.
    BENTLEYDLL_EXPORT static BeFileNameStatus CloneDirectory(WCharCP sourceDir, WCharCP destDir, bool includeSubDirs=true);

    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetTempPath (BeFileNameR tempPath);

    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetTempFileName (BeFileName& tempFileName, BeFileName const& pathName, WCharCP prefixString);

/** @} */

/** @name Copy, delete, move a file by name */
/** @{ */
    //! Copy an existing file. If a file with the new name already exists, it is overwritten.
    //! @param[in] newFileName The name for the newly copied file.
    //! @param[in] existingFileName The name for the source file.
    //! @param[in] failIfFileExists Do not copy and return error if the destination file already exists.
    //! @return BeFileNameStatus::Success if the file was successfully copied.
    BENTLEYDLL_EXPORT static BeFileNameStatus BeCopyFile(BeFileNameCR existingFileName, BeFileNameCR newFileName, bool failIfFileExists = false);

    //! Copy an existing file. If a file with the new name already exists, it is overwritten.
    //! @param[in] newFileName The name for the newly copied file.
    //! @param[in] existingFileName The name for the source file.
    //! @param[in] failIfFileExists Do not copy and return error if the destination file already exists.
    //! @return BeFileNameStatus::Success if the file was successfully copied.
    //! @deprecated Use version of BeFileName::BeCopyFile that takes BeFileNameCR parameters instead
    BENTLEYDLL_EXPORT static BeFileNameStatus BeCopyFile(WCharCP existingFileName, WCharCP newFileName, bool failIfFileExists = false);

    //! Delete this file.
    //! @return BeFileNameStatus::Success if the file was successfully deleted. It returns BeFileNameStatus::CantDeleteFile if not.
    //! @remarks This function deletes the target file even if it is "read-only".
    //! @remarks This function behaves differently on different operating systems if the calling program currently has the target file open.
    //! On Windows, this function will not delete an open file. On other operating systems, this function will delete an open file.
    BENTLEYDLL_EXPORT BeFileNameStatus BeDeleteFile() const;
    
    //! Delete an existing file.
    //! @param[in] fileNameP The name of an existing file.
    //! @return BeFileNameStatus::Success if the file was successfully deleted. It returns BeFileNameStatus::CantDeleteFile if not.
    //! @remarks This function deletes the target file even if it is "read-only".
    //! @remarks This function behaves differently on different operating systems if the calling program currently has the target file open.
    //! On Windows, this function will not delete an open file. On other operating systems, this function will delete an open file.
    //! @deprecated Use instance version of BeFileName::BeDeleteFile instead
    BENTLEYDLL_EXPORT static BeFileNameStatus BeDeleteFile(WCharCP fileNameP);
    
    //! Move or rename an existing file.
    //! @param[in] oldFileName The name of an existing file.
    //! @param[in] newFileName The new name for the file.
    //! @param numRetries the number of times to retry the open in the event of a sharing violation
    //! @return BeFileNameStatus::Success if the file was successfully moved or BeFileNameStatus::UnknownError if not.
    BENTLEYDLL_EXPORT static BeFileNameStatus BeMoveFile(BeFileNameCR oldFileName, BeFileNameCR newFileName, int numRetries = 0);

    //! Move or rename an existing file.
    //! @param[in] oldFileName The name of an existing file.
    //! @param[in] newFileName The new name for the file.
    //! @param numRetries the number of times to retry the open in the event of a sharing violation
    //! @return BeFileNameStatus::Success if the file was successfully moved or BeFileNameStatus::UnknownError if not.
    //! @deprecated Use version of BeFileName::BeMoveFile that takes BeFileNameCR parameters instead
    BENTLEYDLL_EXPORT static BeFileNameStatus BeMoveFile(WCharCP oldFileName, WCharCP newFileName, int numRetries = 0);

    //! Set the read-only status of this file
    //! @param[in] readOnly the desired read-only status
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if not
    BENTLEYDLL_EXPORT BeFileNameStatus SetFileReadOnly(bool readOnly) const;

    //! Set the read-only status of the specified file
    //! @param[in] fileName The name of an existing file.
    //! @param[in] readOnly the desired read-only status
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if not
    //! @deprecated Use instance version of BeFileName::SetFileReadOnly instead
    BENTLEYDLL_EXPORT static BeFileNameStatus SetFileReadOnly(WCharCP fileName, bool readOnly);

    //! Query if this file is read-only
    //! @remarks On unix, this function tests if the owner does not have write permission
    //! @return true if this file exists and is read-only
    BENTLEYDLL_EXPORT  bool IsFileReadOnly() const;

    //! Query if a file is read-only
    //! @param[in] fileName The name of the file 
    //! @remarks On unix, this function tests if the owner does not have write permission
    //! @return true if the file exists and is read-only
    BENTLEYDLL_EXPORT static bool IsFileReadOnly(WCharCP fileName);

/** @} */

/** @name Query file properties */
/** @{ */
    //! Get file size of this file.
    //! @param[out] sz The size of the file in bytes
    BENTLEYDLL_EXPORT BeFileNameStatus GetFileSize(uint64_t& sz) const;

    //! Get file size of the specified file.
    //! @param[out] sz The size of the file in bytes
    //! @param[in] fileName The name of an existing file.
    //! @deprecated Use instance version of BeFileName::GetFileSize instead
    BENTLEYDLL_EXPORT static BeFileNameStatus GetFileSize(uint64_t& sz, WCharCP fileName);

    //! Get the create, access, and/or modification times of this file.
    //! @param[out] ctime   If not NULL, the create time of the file
    //! @param[out] atime   If not NULL, the last accessed time of the file
    //! @param[out] mtime   If not NULL, the last modified time of the file
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileNameStatus GetFileTime(time_t* ctime, time_t* atime, time_t* mtime) const;

    //! Get the create, access, and/or modification times of the specified file.
    //! @param[out] ctime   If not NULL, the create time of the file
    //! @param[out] atime   If not NULL, the last accessed time of the file
    //! @param[out] mtime   If not NULL, the last modified time of the file
    //! @param[in] fileName The name of an existing file.
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if it failed.
    //! @deprecated Use instance version of BeFileName::GetFileTime instead
    BENTLEYDLL_EXPORT static BeFileNameStatus GetFileTime(time_t* ctime, time_t* atime, time_t* mtime, WCharCP fileName);

    //! Set the access and/or modification times of this file.
    //! @param[in] atime If not NULL, the last accessed time of the file
    //! @param[in] mtime If not NULL, the last modified time of the file
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileNameStatus SetFileTime(time_t const* atime, time_t const* mtime) const;

    //! Set the access and/or modification times of the specified file.
    //! @param[in] fileName The name of an existing file.
    //! @param[in] atime If not NULL, the last accessed time of the file
    //! @param[in] mtime If not NULL, the last modified time of the file
    //! @return BeFileNameStatus::Success if the operation was successful or non-zero if it failed.
    //! @deprecated Use instance version of BeFileName::SetFileTime instead
    BENTLEYDLL_EXPORT static BeFileNameStatus SetFileTime(WCharCP fileName, time_t const* atime, time_t const* mtime);

    //! Check user's access to this file
    //! @param[in] accs The kinds of access to check for
    //! @return BeFileNameStatus::Success if all of the specified access rights are granted; non-zero if the file does not exist or some of the specified rights are not granted.
    BENTLEYDLL_EXPORT BeFileNameStatus CheckAccess(BeFileNameAccess accs) const;

    //! Check user's access to the specified file
    //! @param[in] fileName The name of an existing file.
    //! @param[in] accs The kinds of access to check for
    //! @return BeFileNameStatus::Success if all of the specified access rights are granted; non-zero if the file does not exist or some of the specified rights are not granted.
    //! @deprecated Use instance version of BeFileName::CheckAccess instead
    BENTLEYDLL_EXPORT static BeFileNameStatus CheckAccess(WCharCP fileName, BeFileNameAccess accs);
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
    BENTLEYDLL_EXPORT static BeFileNameStatus BeGetDiskFreeSpace(uint64_t& freeBytes, WCharCP dirName);

    //! Get the current working directory
    //! @param[out] currentDirectory The current directory;
    BENTLEYDLL_EXPORT static BeFileNameStatus GetCwd (WStringR currentDirectory);

/** @} */
};

END_BENTLEY_NAMESPACE
