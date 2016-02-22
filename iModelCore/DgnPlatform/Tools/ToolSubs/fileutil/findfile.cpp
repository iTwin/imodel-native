/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/fileutil/findfile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <DgnPlatformInternal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <DgnPlatform/Tools/stringop.h>
#include <Bentley/BeFileListIterator.h>

USING_NAMESPACE_BENTLEY_DGN

#define UF_INTERNALUSE_RELATIVEPATH 0x40
#define DOUBLEQUOTE                 0x22

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void util_parseFileName (WCharCP string, WStringP dev, WStringP dir, WStringP name, WStringP ext, bool treatAsFolder)
    {
    BeFileName::ParseName (dev, dir, name, ext, string);

    if (NULL == string || ! treatAsFolder || NULL == name)
        return;

    /*---------------------------------------------------------------
    Check if the file name is special directory name: '.' or '..'.
    Note that other file names ending with '.' are not valid file names
    on Windows/DOS and are ignored.

    BeFileName::ParseName doesn't handle these cases well so we do
    it here.
    ---------------------------------------------------------------*/
    WCharCP lastDirSep;

    if ((lastDirSep = ::wcsrchr (string, WCSDIR_SEPARATOR_CHAR)) == NULL)
        lastDirSep = ::wcsrchr (string, WCSALT_DIR_SEPARATOR_CHAR);

    if (NULL == lastDirSep)
        return;

    WCharCP lastDot = ::wcsrchr (string, L'.');

    if (NULL == lastDot || lastDot < lastDirSep)
        return;

    if ( (L'\0' == *(lastDot+1)) ||
         (DOUBLEQUOTE == *(lastDot+1) && L'\0' == *(lastDot+2)) )
        {
        size_t namelen = lastDot - lastDirSep;
        switch (namelen)
            {
            case 1: /* The name is '.'  */
                *name = L".";
                break;
            case 2: /* The name can be '..' */
                if (L'.' == *(lastDirSep+1))
                    *name = L"..";
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/08
+---------------+---------------+---------------+---------------+---------------+------*/
static int findFile_tryOpeningFile (BeFile* file, WCharCP filename, int option)
    {
    BeFile      tmpFile;

    StatusInt   status;
    bool        existed;

    if ((option & UF_OPEN_WRITE) || (option & UF_TRY_WRITE_THEN_READ))
        {
        existed = BeFileName::DoesPathExist (filename);
        status  = tmpFile.Open (filename, BeFileAccess::ReadWrite) == BeFileStatus::Success? SUCCESS: ERROR;

        if ((status != SUCCESS) && (option & UF_TRY_WRITE_THEN_READ))
            {
            BeFileStatus tmpStatus;

            tmpStatus = tmpFile.Open (filename, BeFileAccess::Read);
            status = (tmpStatus == BeFileStatus::Success) ? UF_WTR_SUCCESS : static_cast<StatusInt>(tmpStatus);
            }

        if (status == 0 || status == UF_WTR_SUCCESS)
            {
            if (NULL != file)
                file->Swap (tmpFile);
            else
                {
                tmpFile.Close();
                if (!existed)                             /* if file did not exist before and  */
                    BeFileName::BeDeleteFile (filename);  /* no handle requested - delete file */
                }
            }
        }
    else if ((option & UF_OPEN_CREATE))
        {
        status = 0;
        if (!(option & UF_JUST_BUILD))
            {
            status = tmpFile.Create (filename) == BeFileStatus::Success? SUCCESS: ERROR;
            if (status == 0)
                {
                if (NULL != file)
                    file->Swap (tmpFile);
                else
                    {
                    tmpFile.Close ();
                    BeFileName::BeDeleteFile (filename);
                    }
                }
            }
        }
    else if (NULL != file)
        {
        status = tmpFile.Open (filename, BeFileAccess::Read) == BeFileStatus::Success? SUCCESS: ERROR;

        if (SUCCESS == status)
            file->Swap (tmpFile);
        }
    else
        {
        status = BeFileName::DoesPathExist(filename)? SUCCESS: ERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* fourth step - build file name, check existance, attempt to open.
* @bsimethod                                                    JVB             02/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      findFile_buildFileNameAndOpen
(
BeFile*         file,           /* <= File object       (or NULL)      */
WStringR        outName,        /* <= Target file name                 */
WCharCP         dev,            /* => Device            (or NULL)      */
WCharCP         dir,            /* => Directory         (or NULL)      */
WCharCP         name,           /* => Root file name    (or NULL)      */
WCharCP         ext,            /* => File extension    (or NULL)      */
WCharCP         curdev,         /* => Current/default device           */
WCharCP         curdir,         /* => Current/default directory        */
int             option          /* => Find file options UF_...         */
)
    {
    int         status   = ERROR;
    bool        noCurDir = (0 != (option & UF_NO_CUR_DIR));

    outName.clear();

    // if no name and no extension, we won't find a file.
    if ( (NULL == name || 0 == *name) && (NULL == ext || 0 == *ext) )
        return (ERROR);

    /*-------------------------------------------------------------------
    Start with a device name on platforms that use one. If "dev" is NULL,
    use the current device with one exception...

    WIN32 doesn't always use a device.  For example "\\mount\dir\"
    -------------------------------------------------------------------*/
#if defined (_WIN32)
    if ( (0 == *dir) || (0 != wcsncmp (dir, L"\\\\", 2)) )
        {
        // if dev is blank, and we don't want to use the current device, error out.
        if ( (NULL == dev) || (0 == *dev) )
            {
            if (noCurDir)
                return ERROR;
            else
                dev = curdev;
            }

        if ( (NULL != dev) && (0 != *dev) )
            {
            outName.append (dev);
            outName.append (L":");
            }
        }
#endif

    /*-------------------------------------------------------------------
    Add the directory and file name. If "dir" is NULL, use the current directory.
    -------------------------------------------------------------------*/
    if ( (NULL == dir) || (0 == *dir) )
        {
        if (noCurDir)
            return ERROR;

        /* directory not specified, use current directory */
        outName.append (curdir);
        }
    else if ((WCSDIR_SEPARATOR_CHAR == *dir) || (WCSALT_DIR_SEPARATOR_CHAR == *dir))
        {
        /* full path to directory specified, use it */
        outName.append (dir);
        }
    else
        {
        /* directory specified as a relative path based on current directory */
        if (!noCurDir)
            outName.append (curdir);
        outName.append (dir);
        }

    if (0 != *name)
        outName.append (name);

    if (0 != *ext)
        {
        outName.append (L".");
        outName.append (ext);
        }

    static bool s_ignoreFileLengthTooLong = true;           // With BeFileName now WString based, we can handle arbitrarily long names (RayB  06/2013).

    if (s_ignoreFileLengthTooLong || outName.length() < MAXFILELENGTH)
        status = findFile_tryOpeningFile (file, outName.c_str(), option);
    else
        status = FINDFILESTATUS_NameTooLong;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* third step - search through list of directories/paths
* @bsimethod                                                    Shaun.Sewall    06/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findFile_searchDirectoryList
(
BeFile          *outFileH,          /* <= */
WStringR        outFileName,        /* <= */
wchar_t const   *inFileNameP,       /* => */
wchar_t const   *dirListP,          /* => */
wchar_t const   *defaultFileNameP,  /* => */
wchar_t const   *currDevP,          /* => */
wchar_t const   *currDirP,          /* => */
int             option              /* => */
)
    {
    int         returnValue;
    WString     inDev, inDir, inName, inExt;
    bool        treatAsFolder = (option & UF_FIND_FOLDER) ? true : false;

    /* get pieces that were passed in */
    util_parseFileName (inFileNameP, &inDev, &inDir, &inName, &inExt, treatAsFolder);

    /*-----------------------------------------------------------------------------------
    Handle relative path on input name - see comment in util_findFile
    Combine directory and name, zero out directory.
    -----------------------------------------------------------------------------------*/
    if ((option & UF_INTERNALUSE_RELATIVEPATH) && ( ! inDir.empty()) && ( ! inName.empty()))
        {
        BeFileName::BuildName (inName, NULL, inDir.c_str(), inName.c_str(), NULL);
        inDir.clear();
        }

    /* if all pieces are supplied, no need to go thru list */
    if ( ! inDir.empty() && ! inDev.empty() && ! inName.empty() && ! inExt.empty())
        {
        WString builtFileName;
        returnValue = findFile_buildFileNameAndOpen (outFileH, builtFileName, inDev.c_str(), inDir.c_str(), inName.c_str(), inExt.c_str(), currDevP, currDirP, option);
        outFileName = builtFileName;
        }
    else
        {
        wchar_t *tempDirP, *pHead;
        WString dir, name, ext, dev;
        WString defaultDir, defaultName, defaultExt, defaultDev;
        WString builtFileName;
        size_t  dirListLength;
        bool    triedDirListAsPath      = false;
        bool    truncateAtPathSeparator = true;
        bool    foundPathSeparator      = false;

        /* get the default pieces */
        util_parseFileName (defaultFileNameP, &defaultDev, &defaultDir, &defaultName, &defaultExt, treatAsFolder);

        /* allocate space on the stack for a working copy of the list of directories */
        if (NULL == (pHead = static_cast<wchar_t*>(_alloca (dirListLength = ((wcslen(dirListP)+1) * sizeof (wchar_t))))))
            return ERROR;

        /* copy list, initialize pointer to beginning of string */
        wcscpy (tempDirP=pHead, dirListP);

        // if the dirListLength is too long, don't bother to try it as a path.
        if (dirListLength > MAXDIRLENGTH)
            triedDirListAsPath = true;

        do
            {
            wchar_t *pNext = NULL;
            if (truncateAtPathSeparator)
                {
                wchar_t *p;
                // isolate the first remaining path at the separator.
                if (NULL != (p = ::wcschr (tempDirP, WCSPATH_SEPARATOR_CHAR)))
                    {
                    foundPathSeparator = true;
                    *p = 0;
                    if (0 != *(p+1))
                        pNext = p+1;
                    }
                }

            /* initialize current working pieces from the input */
            dev  = inDev;
            dir  = inDir;
            name = inName;
            ext  = inExt;

            /* use the current directory from the list, tempDirP, to fill in unsupplied pieces */
            util_parseFileName (tempDirP, (dev.empty())  ?  &dev  : NULL,
                                          (dir.empty())  ?  &dir  : NULL,
                                          (name.empty()) ?  &name : NULL,
                                          (ext.empty())  ?  &ext  : NULL,      treatAsFolder);

            /* use defaults to fill in pieces that are still not supplied */
            if (dev.empty())
                dev = defaultDev;
            if (dir.empty())
                dir = defaultDir;
            if (name.empty())
                name = defaultName;
            if (ext.empty())
                ext = defaultExt;

            /* have all the pieces, check for the file */
            returnValue = findFile_buildFileNameAndOpen (outFileH, builtFileName, dev.c_str(), dir.c_str(), name.c_str(), ext.c_str(), currDevP, currDirP, option);

            // copy to outFileName, unless we couldn't open it and it's the case where we're trying the full dirList as the path.
            if ( (SUCCESS == returnValue) || (UF_WTR_SUCCESS == returnValue) || ( (FINDFILESTATUS_NameTooLong != returnValue) && truncateAtPathSeparator) )
                outFileName = builtFileName;

            if (SUCCESS == returnValue)
                break;

            // if we didn't find it, and we haven't yet tried the full directory list as a path, try that before giving up.
            if (NULL == (tempDirP = pNext))
                {
                if (foundPathSeparator && !triedDirListAsPath)
                    {
                    // get full path back.
                    wcscpy (tempDirP=pHead, dirListP);
                    // make sure we stop after trying it.
                    triedDirListAsPath      = true;
                    truncateAtPathSeparator = false;
                    }
                }

            } while (NULL != tempDirP);
        }

    return  returnValue;
    }

/*---------------------------------------------------------------------------------**//**
* second step in util_findFile. search through list of environment variables
* @bsimethod                                                    JVB             02/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findFile_breakUpEnvVar
(
BeFile          *pHandle,
WStringR        outName,
wchar_t const   *inName,
wchar_t const   *envvar,
wchar_t const   *defaultFileName,
wchar_t const   *curdev,
wchar_t const   *curdir,
int             option
)
    {
    WChar       tmpVar[4096]; 
    WCharP      nextVar = NULL;
    WString     tmpVarExpansion;

    tmpVar[0] = 0;

    // if there's an environment variable passed in, break it up at the PATH_SEPARATOR_CHARs, expand if it's a macro, and consider each in turn.
    if ((NULL != envvar) && (envvar[0]))
        {
        BeStringUtilities::Wcsncpy (tmpVar, _countof (tmpVar), envvar);

        WCharP  separator;
        if (NULL != (separator = ::wcschr (tmpVar, WCSPATH_SEPARATOR_CHAR)))
            {
            *separator = 0;
            nextVar = (0 != *(separator+1)) ? separator+1 : NULL;
            }

        // if it's not a config variable, use it as is.
        if (SUCCESS != ConfigurationManager::GetVariable (tmpVarExpansion, tmpVar))
            tmpVarExpansion.assign (tmpVar);
        }

    // search the directories in the expansion of the macro.
    StatusInt       rtnval;
    if (SUCCESS == (rtnval = findFile_searchDirectoryList (pHandle, outName, inName, tmpVarExpansion.c_str(), defaultFileName, curdev, curdir, option)))
        return SUCCESS;

    // if we didn't find it yet, keep looking by calling this function recursively with the next part of the environment variable.
    if (NULL != nextVar)
        return      findFile_breakUpEnvVar (pHandle, outName, inName, nextVar, defaultFileName, curdev, curdir, option);

    return  rtnval;
    }


/*---------------------------------------------------------------------------------**//**
* ** util_findFile
* short *handle <= file handle or -1 if no file was opened. NULL may be passed to only check if operation would succeed
* char *outName, <= complete file spec. drive+path+file+extension. must be MAXFILELENGTH chars long or NULL
* char *inName, => Any portion of full file spec. Always used first. Should be root file name (or NULL)
* char *envvar, => Any portion of full file spec. Always used second. May contain environment variables and/or multiple parts of file specs
* separated by sepchar. (or NULL).
* char *defaultFile, => Any portion of full file spec. Used last. Usually, the defaultfile extension. (or NULL)
* int option => Option flags. (may be OR'ed togeather) UF_OPEN_WRITE - opens new file, will not delete an existing file UF_OPEN_CREATE - will
* delete existing file UF_TRY_WRITE_THEN_READ - trys to open the file for write. If that fails it trys to open for read. Returns SUCCESS if
* opened for write or UF_WTR_SUCCESS if open for read. UF_CUR_DIR_SWAP - by default the current directory will be searched first for read
* options and last for write options. This option reverses the order UF_CUR_DIR_SWAP - by default the current directory will be searched first
* for read options and last for write options. This option reverses the order Use 0 for default. open for read
* @bsimethod                                                    JVB             02/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      util_findFileDeprecated
(
BeFile*         pHandle,            /* <= File handle                          */
WCharP          outName,            /* <= Output file name                     */
WCharCP         inName,             /* => Input file name                      */
WCharCP         envvar,             /* => List of related names/logicals       */
WCharCP         defaultFileName,    /* => Default name and/or extension        */
int             option              /* => Open mode / search options           */
)
    {
    int         status;
    BeFileName  outFileName;

    if (SUCCESS == (status = util_findFile (pHandle, ((NULL == outName) ? NULL : &outFileName), inName, envvar, defaultFileName, option)) && NULL != outName)
        BeStringUtilities::Wcsncpy (outName, MAXFILELENGTH, outFileName.GetName());

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* ** util_findFile
* short *handle <= file handle or -1 if no file was opened. NULL may be passed to only check if operation would succeed
* char *outName, <= complete file spec. drive+path+file+extension. must be MAXFILELENGTH chars long or NULL
* char *inName, => Any portion of full file spec. Always used first. Should be root file name (or NULL)
* char *envvar, => Any portion of full file spec. Always used second. May contain environment variables and/or multiple parts of file specs
* separated by sepchar. (or NULL).
* char *defaultFile, => Any portion of full file spec. Used last. Usually, the defaultfile extension. (or NULL)
* int option => Option flags. (may be OR'ed togeather) UF_OPEN_WRITE - opens new file, will not delete an existing file UF_OPEN_CREATE - will
* delete existing file UF_TRY_WRITE_THEN_READ - trys to open the file for write. If that fails it trys to open for read. Returns SUCCESS if
* opened for write or UF_WTR_SUCCESS if open for read. UF_CUR_DIR_SWAP - by default the current directory will be searched first for read
* options and last for write options. This option reverses the order UF_CUR_DIR_SWAP - by default the current directory will be searched first
* for read options and last for write options. This option reverses the order Use 0 for default. open for read
* @bsimethod                                                    JVB             02/89
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt      util_findFile
(
BeFile*         pHandle,            /* <= File handle                          */
BeFileNameP     outName,            /* <= Output file name                     */
WCharCP         inName,             /* => Input file name                      */
WCharCP         envvar,             /* => List of related names/logicals       */
WCharCP         defaultFileName,    /* => Default name and/or extension        */
int             option              /* => Open mode / search options           */
)
    {
    int         rtnval;
    WString     localenv, localIn, localOutputFileName;;
    bool        read;
    WString     cwdDirectory, cwdDevice, cwdPath;

    // get the current path and make sure it ends with the directory separator.
    BeFileName::GetCwd (cwdPath);

    if ( ( ! cwdPath.empty()) && (WCSDIR_SEPARATOR_CHAR != *(cwdPath.end() - 1)) )
        cwdPath.append (WCSDIR_SEPARATOR);

    // parse the current working path into its device and dir
    util_parseFileName (cwdPath.c_str(), &cwdDevice, &cwdDirectory, NULL, NULL, true);

    if ((NULL != envvar) && (0 != *envvar) )
        localenv = envvar;

    read = !((option & UF_OPEN_WRITE) || (option & UF_OPEN_CREATE) || (option & UF_TRY_WRITE_THEN_READ));

    if ( (NULL != inName) && (0 != *inName) )
        {
        WString     tempString = inName, deviceString;

        size_t      colonIndex = tempString.find_first_of (L':');

        if (WString::npos != colonIndex && 
            colonIndex > 0 &&
            ! (deviceString = tempString.substr (0, colonIndex)).empty() &&
            ConfigurationManager::IsVariableDefined  (deviceString.c_str()))
            {
            // prepend the logical that's in inName to
            localenv = deviceString;

            if (NULL != envvar && 0 != envvar[0])
                {
                localenv.append (WCSPATH_SEPARATOR);
                localenv.append (envvar);
                }

            // if opening for read, do current directory last if we have an environment variable.
            if (read)
                option |= UF_CUR_DIR_SWAP;

            // if any part of the specification follows the environment variable, copy it to localIn and parse from there.
            size_t      remainingIndex = colonIndex + 1;
            if (remainingIndex < tempString.size())
                {
                localIn = tempString.substr (remainingIndex, tempString.size() - remainingIndex);

                /*-----------------------------------------------------------------------
                If there was a logical on the input name and the input name contains a
                directory but doesn't start with the root (or a drive) then treat the
                input name as a relative path i.e. don't parse off the directory in later
                stages of searching.
                -----------------------------------------------------------------------*/
                WString testDev, testDir;

                util_parseFileName (localIn.c_str(), &testDev, &testDir, NULL, NULL, TO_BOOL (option & UF_FIND_FOLDER));
                if (testDev.empty() && ! testDir.empty() && (WCSDIR_SEPARATOR_CHAR != testDir[0]))
                    option |= UF_INTERNALUSE_RELATIVEPATH;
                }
            }
        else
            {
            // no logical name, but if the name starts with a "..", use it as a relative name.
            if (0 == wcsncmp (inName, L"..", 2))
                option |= UF_INTERNALUSE_RELATIVEPATH;
            else
                {
                WString testDev, testDir;

                // if no device, and there's a directory that doesn't start with DIR_SEPARATOR_CHAR, treat as a relative path.
                util_parseFileName (inName, &testDev, &testDir, NULL, NULL, TO_BOOL (option & UF_FIND_FOLDER));
                if ((testDev.empty()) && ( ! testDir.empty()) && (WCSDIR_SEPARATOR_CHAR != testDir[0]))
                    option |= UF_INTERNALUSE_RELATIVEPATH;
                }

            localIn = inName;
            }
        }


    // check the current directory first, if appropriate.
    if ( (0 == (option & UF_NO_CUR_DIR)) && ( (read && (! (option & UF_CUR_DIR_SWAP))) || (!read && (option & UF_CUR_DIR_SWAP))))
        {
        rtnval = findFile_searchDirectoryList (pHandle, localOutputFileName, localIn.c_str(), cwdPath.c_str(), defaultFileName, cwdDevice.c_str(), cwdDirectory.c_str(), option);
        }
    else
        {
        rtnval = 1;
        }

    // didn't find it in the current directory, search the environment variables
    if (SUCCESS != rtnval)
        {
        rtnval = findFile_breakUpEnvVar (pHandle, localOutputFileName, localIn.c_str(), localenv.c_str(), defaultFileName, cwdDevice.c_str(), cwdDirectory.c_str(), option);
        }

    // didn't find it in the environment variable list. If we wanted to look in the current directory last, try that now.
    if ( (SUCCESS != rtnval) && (UF_WTR_SUCCESS != rtnval) )
        {
        if ( (0 == (option & UF_NO_CUR_DIR)) && ( (read && (option & UF_CUR_DIR_SWAP)) || (!read && (! (option & UF_CUR_DIR_SWAP))) ) )
            {
            rtnval = findFile_searchDirectoryList (pHandle, localOutputFileName, localIn.c_str(), cwdPath.c_str(), defaultFileName, cwdDevice.c_str(), cwdDirectory.c_str(), option);
            }
        }

    if (NULL != outName)
        {
        WString         fullName;

        BeFileName::BeGetFullPathName (fullName, localOutputFileName.c_str());
        outName->SetName (fullName.c_str());
        }

    return  rtnval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      util_findFileInPath
(
BeFileNameP     expandedNameP,
WCharCP         filenameP,
WCharCP         extensionP
)
    {
    WString     path;
    if (SUCCESS != ConfigurationManager::GetVariable (path, L"PATH"))
        return ERROR;

    // make sure all the components of path have correct termination, by copying into another string.
    WString     processedPath;
    size_t  inputPos;
    size_t  nextPos;

    for (inputPos = 0; WString::npos != inputPos; inputPos = nextPos)
        {
        nextPos = path.find (PATH_SEPARATOR_CHAR, inputPos);

        if (WString::npos != nextPos)
            {
            processedPath.append (path.substr (inputPos, nextPos-1));
            nextPos++;
            }
        else
            {
            processedPath.append (path.substr (inputPos));
            }

        if (DIR_SEPARATOR_CHAR != (processedPath.at (processedPath.length()-1)))
            processedPath.append (1, WCSDIR_SEPARATOR_CHAR);

        processedPath.append (1, WCSPATH_SEPARATOR_CHAR);
        }

    /* see if we can find the file in the modified path */
    return util_findFile (NULL, expandedNameP, filenameP, processedPath.c_str(), extensionP, 0);
    }
