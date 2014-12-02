/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/msfilutl.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    <stddef.h>

/*------------------------------------------------------------------------*//**
* Parses a file name into its components.
* @Param        fileName        IN  filename to be parsed
* @Param        device          OUT the device. The buffer must be at least MAXDEVICELENGTH bytes.
* @Param        directory       OUT the directory. The buffer must be at least MAXDIRLENGTH bytes.
* @Param        name            OUT the root name of the file. The buffer must be least MAXNAMELENGTH bytes.
* @Param        extension       OUT the file extension. The buffer must be at least MAXEXTENSIONLENGTH bytes.
* @Remarks      If particular component in the input name is not found, the
*               corresponding buffer, if present, is set to the NULL string ("");
*** NOTE: Old versions of this function used to leave the output buffer unmodified, which caused many problems for callers.  ***
* @ALinkJoin    usmthmdlFile_buildNameAC usmthmdlFile_createC usmthmdlFile_findC
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_parseName (WCharCP fileName, WCharP device, WCharP directory, WCharP name, WCharP extension);

/*------------------------------------------------------------------------*//**
* Parses a file name into its components.
* @Param        fileName        IN  filename to be parsed. The name can be a list of filenames separated by semicolons, in which case the first fileName is parsed.
* @Param        device          OUT the device. The buffer must be at least MAXDEVICELENGTH bytes.
* @Param        directory       OUT the directory. The buffer must be at least MAXDIRLENGTH bytes.
* @Param        name            OUT the root name of the file. The buffer must be least MAXNAMELENGTH bytes.
* @Param        extension       OUT the file extension. The buffer must be at least MAXEXTENSIONLENGTH bytes.
* @Remarks      If particular component in the input name is not found, the
*               corresponding buffer, if present, is set to the NULL string ("");
*** NOTE: Old versions of this function used to leave the output buffer unmodified, which caused many problems for callers.  ***
* @ALinkJoin    usmthmdlFile_buildNameAC usmthmdlFile_createC usmthmdlFile_findC
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_parseNameList (WCharCP fileName, WCharP device, WCharP directory, WCharP name, WCharP extension);

/*------------------------------------------------------------------------*//**
* Creates a file name from the components specified.
* @Param        fileName        OUT the file name built from the components. The buffer must be at least MAXFILELENGTH bytes.
* @Param        device          IN  the device name.
* @Param        directory       IN  the directory. If the final character is not a directory separator, the function appends one.
* @Param        name            IN  the file root name.
* @Param        extension       IN  the file extension
* @Remarks      Any of the input arguments can be NULL, or they can point to an empty string. If a
*               component is NULL or points to an empty string, mdlFile_buildNameA does
*               not place the corresponding separator into the output string. For example,
*               if device is NULL, mdlFile_buildNameA does not place a colon in the
*               output string before the directory specification.
* @ALinkJoin    usmthmdlFile_parseNameAC
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_buildName (WCharP fileName, WCharCP device, WCharCP directory, WCharCP name, WCharCP extension);

/*------------------------------------------------------------------------*//**
* Concatenates a list of strings using specified separator.
* @Param        outstg  OUT Output string.
* @Param        usersep IN String used to separate strings.
* @Param        ...     IN Variable length list of strings (CharP ) to be concatenated. The last item must be NULL.
* @Return       pointer to outstg if successful, NULL on failure.
* @Remarks      If userSep is NULL, the system path separator character is used. Pass an empty string ("") for no separator.
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT WCharP mdlFile_buildSeparatedList (WCharP outstg, WCharCP usersep, ...);

/*------------------------------------------------------------------------*//**
* Abbreviates a filename to a specified length. It abbreviates
*               by successively removing leading directory names, replacing them with ellipses (...).
* @Param        fileName    IN OUT  the string holding the file name. It is modified with the
                                    resultant file name.
* @Param        maxLength   IN      the maximum number of characters desired in the resultant name.
* @Return       mdlFile_abbreviateNameA is of type void; it does not return a status.
* @ALinkJoin    usmthmdlFile_abbreviateNameAC usmthmdlFile_compactPathAC
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_abbreviateName (WCharP fileName, size_t maxLength);

/*------------------------------------------------------------------------*//**
* Abbreviates a filename to approximately specified length. It abbreviates
*               by successively removing leading directory names, replacing them with ellipses (...).
* @Remarks      The abbreviated file name could be longer than maxLength.  If an exact length is necessary
*               see ~mmdlFile_compactPathA .
* @Param        fileName    IN OUT  the string holding the file name. It is modified with the
*                                   resultant file name.
* @Param        maxLength   IN      the maximum number of characters desired in the resultant name.
* @ALinkJoin    usmthmdlFile_abbreviateNameAC usmthmdlFile_compactPathAC
* @Group        "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_abbreviateNameEx (WCharP fileName, size_t maxLength);

/*------------------------------------------------------------------------*//**
* Shortens a file path string of wide characters to be no longer than the specified number of characters.
* @param        pwOutStr    OUT the string shortened to the specified length
* @param        pwInStr     IN  the string to be shortened
* @param        length      IN  the number of characters to display in the shortened string
* @Return       true if successful, or false otherwise.
* @Group         "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT bool mdlFile_compactPath (WCharP pwOutStr, WCharCP pwInStr, size_t length);

/*------------------------------------------------------------------------*//**
* Adds leading and trailing quotes to a string.  This is often necessary when
*               working with file names containing spaces.
* @param        fileName        IN OUT  the string to be quoted.
* @param        bufferLength    IN      the size of the fileName array.  See MAXQUOTEDFILELENGTH in basedefs.h.
* @return       SUCCESS if the operation completes; ERROR if the new string won't fit.
* @Group        "File Functions"
* @Alinkjoin    usmthmdlFile_unquoteStringAC
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT StatusInt mdlFile_quoteString (WCharP fileName, size_t bufferLength);

/*------------------------------------------------------------------------*//**
* Removes the leading and trailing quotes from a string.  This is often
*               necessary when working with file names that contain spaces.  If the
*               first character is not a double quote ("), then the function does nothing.
* @param        fileName        IN OUT  the string to be have quotes stripped.
* @Group        "File Functions"
* @Alinkjoin    usmthmdlFile_quoteStringAC
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_unquoteString (WCharP fileName);

/*------------------------------------------------------------------------*//**
* Finds the relative path from a root file to the target file.
*               If the files are on different devices, targetFileName is copied to relativePath.
* @param        relativePath    OUT the relative path to the target file
* @param        targetFileName  IN  the full file specification of the file the relative path is needed for.
* @param        rootFileName    IN  the full file specification of the root for the relative path.
* @Group         "File Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT void mdlFile_findRelativePath (WCharP relativePath, WCharCP targetFileName, WCharCP rootFileName);

/*------------------------------------------------------------------------*//**
* Generates a fullpath by resolving relativeFileName relative to basePath.
* @param        outFullPath      OUT the full path resolved from the inputs.
* @param        numChars         IN  the maximum number of characters to put in the output buffer.
* @param        relativeFileName IN  the relative file name to resolve.
* @param        basePath         IN  used as a root for the expansion.
* @Return       SUCCESS or ERROR.
* @Group         "File Functions"
* @Remarks      This function only performs a string manipulation.  It does not
*               require or make any use of an actual file.
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DESKTOP_TOOLS_EXPORT StatusInt mdlFile_resolveRelativePath (WCharP outFullPath, int numChars, WCharCP relativeFileName, WCharCP basePath);

/*---------------------------------------------------------------------------------*//**
* Opaque File Watch List object. Used with the mdlFile_xxxWatchList functions.
* @Group         "File Functions"
* @bsistruct                    BSI                                   11/02
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct FileWatchList    *FileWatchListP;

/*------------------------------------------------------------------------*//**
* Creates a list of files to watch for changes.
* @Group         "File Functions"
* @return       The created watch list .
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
FileWatchListP mdlFile_createWatchList (void);

/*------------------------------------------------------------------------*//**
* Adds or removes a file from a watch list.
* @Group         "File Functions"
* @param    watchList       IN      The list of files to watch.
* @param    fileName        IN      Name of the file to add or remove.
* @param    watch           IN      If true, adds to watch list, otherwise removes fileName from watch list.
* @return  SUCCESS if able to add or remove the file, an appropriate error code if not able to do so.
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
StatusInt mdlFile_updateWatchList (FileWatchListP watchList, const WCharCP fileName, bool watch);

/*------------------------------------------------------------------------*//**
* Discards a file watch list.
* @Group         "File Functions"
* @param    watchList   IN the watch list to discard and free.
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
void mdlFile_deleteWatchList (FileWatchListP watchList);

#define FILE_STATS_CHANGE_MTIME     1
#define FILE_STATS_CHANGE_SIZE      2

/*---------------------------------------------------------------------------------*//**
* Information about changes to the current state of a file.
* @Group         "File Functions"
* @bsistruct                    BSI                                   11/02
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct
{
UInt32      modTime;
UInt32      fileSize;
} FileStats;

typedef struct
{
/** one or more flags indicating which fields have changed:
<ul>
<li> FILE_STATS_CHANGE_SIZE: The file size
<li> FILE_STATS_CHANGE_MTIME: The file modification time
</ul>
*/
UInt32          mask;
/** The current status of the file */
FileStats stats;
/** The previous status of the file */
FileStats stats0;
} MdlFileStatsChange;


/*---------------------------------------------------------------------------------**//**
* Signature of callback invoked by mdlFile_getWatchedFileChanges
*               on a stats change.
* @param  fileName  IN  Name of the file that changed.
* @param  change    IN  Structure describing the changes.
* @param  arg       IN  User-supplied argument to the callback
* @Group         "File Functions"
* @bsimethod                                                    BSI                   11/02
+---------------+---------------+---------------+---------------+---------------+------*/
typedef void                    MdlFileStatsCallback
(
WCharCP                         fileName,
const MdlFileStatsChange*       change,
void*                           arg
);

/*---------------------------------------------------------------------------------**//**
* Calls the user supplied function for each change to the list of files in the watchList.
* @param  watchList IN  The watch list
* @param  proc      IN  Callback function to call for each change to files in the watchList
* @param  arg       IN  User-defined argument passed to proc.
* @return Number of changes.
* @Group         "File Functions"
* @bsimethod                                                    BSI                   11/02
+---------------+---------------+---------------+---------------+---------------+------*/
int mdlFile_getWatchedFileChanges (FileWatchListP watchList, MdlFileStatsCallback* proc, void* arg);
