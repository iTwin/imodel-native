/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/fileutil/findfile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
#include <DgnPlatform/DesktopTools/msfilutl.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <DgnPlatform/Tools/stringop.h>
#include <Bentley/BeFileListIterator.h>

USING_NAMESPACE_BENTLEY_DGN

#define UF_INTERNALUSE_RELATIVEPATH    0x40

static const char    *nullstr = "";  /* linked external - cant use FMTc_nullstring */
static const WChar *wcsNullStr = L"";  /* linked external - cant use FMTc_nullstring */
#define     MAXUI4  (4294967295)

#if 0 // WIP_BEFILENAME_PORTABILITY
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    DanEast                         04/2001
+---------------+---------------+---------------+---------------+---------------+------*/
bool mdlFile_compactPath (WCharP pwOutStr, WCharCP pwInStr, size_t length)
    {
    WString spath;
    if (BeFileName::BeCompactPath (spath, pwInStr, (uint32_t)length) != SUCCESS)
        return false;
    BeStringUtilities::Wcsncpy (pwOutStr, length, spath.c_str());
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* This is the same as mdlFile_parseName, except it allows the input string to have a
*  directory list, where the directories are separated by semicolons. This behavior is
*  the same as mdlFile_parseName in versions previous to XM.
* @bsimethod                                                    barry.bentley   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void mdlFile_parseNameList (WCharCP string, WCharP dev, WCharP dir, WCharP name, WCharP ext)
    {
    WChar     *p1, tmpstg[5*MAXFILELENGTH]; /* allow for MAXDRIVELENGTH\MAXDIRLENGTH\MAXFILELENGTH\MAXEXTENSIONLENGTH */

    if ( (NULL == string) || (0 == *string) )
        return;

    wcscpy (tmpstg, string);

    /* Truncate string at path separator */
    if ((p1 = ::wcschr (tmpstg, WCSPATH_SEPARATOR_CHAR)) != 0)
        *p1 = 0;

    mdlFile_parseName (tmpstg, dev, dir, name, ext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Simon.Normand   03/2003
+---------------+---------------+---------------+---------------+---------------+------*/
static void parseName (WCharCP string, WChar *dev, WChar *dir, WChar *name, WChar *ext, bool treatAsFolder)
    {
    WChar     *p1, *p2, *p3, tmpstg[5*MAXFILELENGTH]; /* allow for MAXDRIVELENGTH\MAXDIRLENGTH\MAXFILELENGTH\MAXEXTENSIONLENGTH */
    size_t       index;

    //NOTE: Previous versions of this function did not clear the output if a part of the filename wasn't found. That caused no 
    //      end of problems, and I simply changed this policy. If this breaks something, fix the caller - DO NOT CHANGE THIS BACK - KAB
    if (dev)  *dev = 0;
    if (dir)  *dir = 0;
    if (name) *name = 0;
    if (ext)  *ext = 0;

    if ((NULL == string) || (0 == *string))
        return;

    wcscpy (tmpstg, string);

    p1 = tmpstg;

    if ((p2 = ::wcschr (p1, L':')) != 0)  /* get device name if any           */
        {
        *p2 = 0;
        if (dev)
            wcscpy (dev, p1);
        p1 = p2+1;
        }

    index = wcslen (p1)-1;
    p2 = ::wcsrchr (p1, WCSDIR_SEPARATOR_CHAR);
    p3 = ::wcsrchr (p1, WCSALT_DIR_SEPARATOR_CHAR);
    if ((p2 == NULL || p2 != (p1+index)) && (p3 == NULL || p3 != (p1+index)))
        {
        /*---------------------------------------------------------------
        if string does not end with a path separator assume there is a
        file name at the end
        ---------------------------------------------------------------*/
        if ((p3 = ::wcsrchr (p1, WCSDIR_SEPARATOR_CHAR)) == NULL)
            p3 = ::wcsrchr (p1, WCSALT_DIR_SEPARATOR_CHAR);

        if (!p3)
            p3 = p1-1;                   /* get ext if dot after last dirsep  */
                                         /* For consistency get a pointer to character before p1. As we would get
                                            a pointer to first character before the filename */
        if (ext)
            *ext = 0;

        if ( (p2 = ::wcsrchr (p1, '.')) && p2 >= p3)
            {
            /*---------------------------------------------------------------
            Check if the file name is special directory name: '.' or '..'.
            Note that other file names ending with '.' are not valid file names
            on Windows/DOS and are ignored.
            ---------------------------------------------------------------*/
            if ( (true == treatAsFolder) &&
                 (L'\0' == *(p2+1)) ||
                 (DOUBLEQUOTE == *(p2+1) && L'\0' == *(p2+2)))
                {
                size_t namelen = p2 - p3;
                switch (namelen)
                    {
                    case 1: /* The name is '.'  */
                        break;
                    case 2: /* The name can be '..' */
                        if ('.' != *(p3+1)) /* if name is not '..' it is a normal file name */
                            *p2 = 0;
                        break;
                    default:
                        *p2 = 0;
                        break;
                    }
                }
            else
                *p2 = 0; /* Normal extension */

            if (ext) /* Handle extension only if we have found one */
                {
                if (0 != *p2)  /* Extension is not found */
                    {
                    *ext = L'\0';
                    }
                else
                    {
                    size_t extlen;

                    wcscpy (ext, p2+1);

                    extlen = wcslen (ext);
                    /* if the file name was in a pair of quotes, remove the ending quote now */
                    if  (extlen > 0 && ext[extlen-1] == DOUBLEQUOTE)
                        ext[extlen-1] = 0;
                    }
                }
            }
                                       /* get file name if any              */

        if ((p2 = ::wcsrchr (p1, WCSDIR_SEPARATOR_CHAR)) ||
            (p2 = ::wcsrchr (p1, WCSALT_DIR_SEPARATOR_CHAR)))
            {                          /* if directory name exists          */
            if (name)
                wcscpy (name, p2+1);
            * (p2+1) = 0;
            }
        else                           /* if no dir name before file name   */
            {                          /*    use whole string               */
            if (name)
                wcscpy (name, p1);
            *p1 = 0;
            }
        }

    if (*p1 && dir)                   /* whats left of string is dir name  */
        wcscpy (dir, p1);
    }

/*---------------------------------------------------------------------------------**//**
* Unicode version of mdlFile_parseNameA
* @bsimethod                                                    YogeshSajanikar 05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void mdlFile_parseName (WCharCP string, WCharP dev, WCharP dir, WCharP name, WCharP ext)
    {
    parseName (string, dev, dir, name, ext, false /*Always treat as file*/);
    }

/*---------------------------------------------------------------------------------**//**
* Unicode version of mdlFile_buildSeparatedListA returns - pointer to outstg on success or NULL on failure
* @bsimethod                                                  JVB             03/2003
+---------------+---------------+---------------+---------------+---------------+------*/
WChar *mdlFile_buildSeparatedList
(
WCharP  outstg,          /* <= Output string                                      */
WCharCP usersep,         /* => String used to separate strings. If a NULL pointer */
                   /*    is passed, the system path separator character     */
                   /*    (patsep) will be used. Pass the null string ("")   */
                   /*    for no separator                                   */
...                /* => Variable length list of strings (char *) to be     */
                   /*    concatenated. The last item must be NULL           */
)
    {
    va_list     arg;
    WCharP    argstg;
    WCharCP   sepstg;

    if (outstg)                        /* if outstg is a NULL pointer, fail */
        *outstg = 0;                   /*   if not, make it a null string   */
    else
        return (NULL);
                                       /* find string separator to use      */
    sepstg = usersep ? usersep : WCSPATH_SEPARATOR; //PATH_SEPARATOR
    va_start (arg, usersep);           /* start variable argument list      */
    argstg = va_arg (arg, WChar*);      /* get pointer to first argument          */

    if (argstg)
        do
            {
            wcscat (outstg, argstg);
            argstg = va_arg (arg, WChar*);   /* get next argument in list    */
            if (argstg && *argstg)
                wcscat (outstg, sepstg);
            } while (argstg);

    va_end (arg);                      /* reset argument list */
    return (outstg);

    }

/*---------------------------------------------------------------------------------**//**
* Unicode version of mdlFile_buildNameA
* @bsimethod                                                  Simon.Normand   03/2003
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlFile_buildName
(
WCharP    outstg,    /* <= output string                        */
WCharCP   dev,       /* => input device spec                    */
WCharCP   dir,       /* => input directory                      */
WCharCP   name,      /* => input name                           */
WCharCP   ext        /* => input extension                      */
)
    {
    WCharCP       devP, nameP, extP;
    WCharCP       dirP;
    WCharCP       devsepP, dirsepP, namesepP;
    static WChar  colon[] = L":";
    static WChar  period[] = L".";
    WChar         tmpstring[3*MAXFILELENGTH];     /* allow for MAXDRIVELENGTH\MAXDIRLENGTH\MAXFILELENGTH\MAXEXTENSIONLENGTH */
    WChar         tmpdir[MAXDIRLENGTH];
    size_t          len;
    bool            bDirsep = false;

    /*
    *   If directory ends with a directory separator, get rid of it so
    *   we can add it later.
    *   Works great unless just a dirsep is passed in (ie len = 1).
    *   Must compensate for this below.
    */
    if (dir)
        {
        wcscpy (tmpdir, dir);

        if ( (len = wcslen (dir)-1) != -1)
            {
            if ( (dir[len] == WCSDIR_SEPARATOR_CHAR) || (dir[len] == WCSALT_DIR_SEPARATOR_CHAR) )
                {
                if (len == 0)
                    bDirsep = true;
                else
                    tmpdir[len] = '\0';
                }
            }
        }

    devP  = dev  ? dev  :  wcsNullStr;
    devsepP = (dev && *dev) ? colon :  wcsNullStr;
    dirP  = dir  ? tmpdir  :  wcsNullStr;
    dirsepP = (dir && name && *dir && !bDirsep && *name) ? WCSDIR_SEPARATOR :  wcsNullStr;
    nameP = name ? name :  wcsNullStr;
    namesepP = (name && ext && *name && *ext) ? period :  wcsNullStr;
    extP  = ext  ? ext  :  wcsNullStr;

    mdlFile_buildSeparatedList (tmpstring,  wcsNullStr, devP, devsepP, dirP, dirsepP, nameP, namesepP, extP, NULL);
    wcscpy (outstg, tmpstring);
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
        status  = tmpFile.Open (filename, BeFileAccess::ReadWrite, BeFileSharing::None) == SUCCESS? SUCCESS: ERROR;

        if ((status != SUCCESS) && (option & UF_TRY_WRITE_THEN_READ))
            {
            BeFileStatus tmpStatus;

            tmpStatus = tmpFile.Open (filename, BeFileAccess::Read, BeFileSharing::Read);
            status = (tmpStatus == SUCCESS) ? UF_WTR_SUCCESS : (StatusInt)tmpStatus;
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
            status = tmpFile.Create (filename) == SUCCESS? SUCCESS: ERROR;
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
        status = tmpFile.Open (filename, BeFileAccess::Read, BeFileSharing::Read) == SUCCESS? SUCCESS: ERROR;
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
static int     findFile_buildFileNameAndOpen
(
BeFile          *file,          /* <= File object       (or NULL)      */
wchar_t         *outName,       /* <= Target file name                 */
wchar_t const   *dev,           /* => Device            (or NULL)      */
wchar_t const   *dir,           /* => Directory         (or NULL)      */
wchar_t const   *name,          /* => Root file name    (or NULL)      */
wchar_t const   *ext,           /* => File extension    (or NULL)      */
wchar_t const   *curdev,        /* => Current/default device           */
wchar_t const   *curdir,        /* => Current/default directory        */
int             option          /* => Find file options UF_...         */
)
    {
    int         status   = ERROR;
    bool        noCurDir = (0 != (option & UF_NO_CUR_DIR));

    *outName = 0;

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

        wcscat (outName, dev);
        // if there is a device, follow it with a :
        if (0 != *outName)
            wcscat (outName, L":");
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
        wcscat (outName, curdir);
        }
    else if ((WCSDIR_SEPARATOR_CHAR == *dir) || (WCSALT_DIR_SEPARATOR_CHAR == *dir))
        {
        /* full path to directory specified, use it */
        wcscat (outName, dir);
        }
    else
        {
        /* directory specified as a relative path based on current directory */
        if (!noCurDir)
            wcscat (outName, curdir);
        wcscat (outName, dir);
        }

    if (0 != *name)
        wcscat (outName, name);

    if (0 != *ext)
        {
        wcscat (outName, L".");
        wcscat (outName, ext);
        }

    if (wcslen (outName) < MAXFILELENGTH)
        status = findFile_tryOpeningFile (file, outName, option);
    else
        status = DGNPLATFORMTOOLS_STATUS_NameTooLong;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* third step - search through list of directories/paths
* @bsimethod                                                    Shaun.Sewall    06/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findFile_searchDirectoryList
(
BeFile          *outFileH,          /* <= */
wchar_t         *outFileNameP,      /* <= */
wchar_t const   *inFileNameP,       /* => */
wchar_t const   *dirListP,          /* => */
wchar_t const   *defaultFileNameP,  /* => */
wchar_t const   *currDevP,          /* => */
wchar_t const   *currDirP,          /* => */
int             option              /* => */
)
    {
    int         returnValue;
    wchar_t     inDir[MAXDIRLENGTH];
    wchar_t     inName[MAXNAMELENGTH];
    wchar_t     inExt[MAXEXTENSIONLENGTH];
    wchar_t     inDev[MAXDEVICELENGTH];
    bool        treatAsFolder = (option & UF_FIND_FOLDER) ? true : false;

    /* get pieces that were passed in */
    parseName (inFileNameP, inDev, inDir, inName, inExt, treatAsFolder);

    /*-----------------------------------------------------------------------------------
    Handle relative path on input name - see comment in util_findFile
    Combine directory and name, zero out directory.
    -----------------------------------------------------------------------------------*/
    if ((option & UF_INTERNALUSE_RELATIVEPATH) && (0 != *inDir) && (0 != *inName))
        {
        wchar_t tmpName[MAXDIRLENGTH];
        mdlFile_buildName (tmpName, NULL, inDir, inName, NULL);
        wcscpy (inName, tmpName);
        inDir[0] = 0;
        }

    /* if all pieces are supplied, no need to go thru list */
    if (inDev[0] && inDir[0] && inName[0] && inExt[0])
        {
        returnValue = findFile_buildFileNameAndOpen (outFileH, outFileNameP, inDev, inDir, inName, inExt, currDevP, currDirP, option);
        }
    else
        {
        wchar_t *tempDirP, *pHead;
        wchar_t dir[MAXDIRLENGTH];
        wchar_t name[MAXNAMELENGTH];
        wchar_t ext[MAXEXTENSIONLENGTH];
        wchar_t dev[MAXDEVICELENGTH];
        wchar_t defaultDir[MAXDIRLENGTH];
        wchar_t defaultName[MAXNAMELENGTH];
        wchar_t defaultExt[MAXEXTENSIONLENGTH];
        wchar_t defaultDev[MAXDEVICELENGTH];
        wchar_t builtFileName[MAXFILELENGTH*2];
        size_t  dirListLength;
        bool    triedDirListAsPath      = false;
        bool    truncateAtPathSeparator = true;
        bool    foundPathSeparator      = false;

        /* get the default pieces */
        parseName (defaultFileNameP, defaultDev, defaultDir, defaultName, defaultExt, treatAsFolder);

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
            dev[0] = dir[0] = name[0] = ext[0] = 0;
            if (0 != inDev[0])
                wcscpy (dev,  inDev);
            if (0 != inDir[0])
                wcscpy (dir,  inDir);
            if (0 != inName[0])
                wcscpy (name, inName);
            if (0 != inExt)
                wcscpy (ext,  inExt);

            /* use the current directory from the list, tempDirP, to fill in unsupplied pieces */
            parseName (tempDirP, (0 != inDev[0]) ? NULL:dev, (0 != inDir[0]) ? NULL:dir, (0 != inName[0]) ? NULL:name, (0 != inExt[0]) ? NULL:ext, treatAsFolder);

            /* use defaults to fill in pieces that are still not supplied */
            if (0 == dev[0])
                wcscpy (dev,  defaultDev);
            if (0 == dir[0])
                wcscpy (dir,  defaultDir);
            if (0 == name[0])
                wcscpy (name, defaultName);
            if (0 == ext[0])
                wcscpy (ext,  defaultExt);

            /* have all the pieces, check for the file */
            returnValue = findFile_buildFileNameAndOpen (outFileH, builtFileName, dev, dir, name, ext, currDevP, currDirP, option);

            // copy to outFileName, unless we couldn't open it and it's the case where we're trying the full dirList as the path.
            if ( (SUCCESS == returnValue) || (UF_WTR_SUCCESS == returnValue) || ( (DGNPLATFORMTOOLS_STATUS_NameTooLong != returnValue) && truncateAtPathSeparator) )
                wcscpy (outFileNameP, builtFileName);

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
wchar_t         *outName,
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
Public int      util_findFile
(
BeFile          *pHandle,           /* <= File handle                          */
wchar_t         *outName,           /* <= Output file name                     */
const wchar_t   *inName,            /* => Input file name                      */
const wchar_t   *envvar,            /* => List of related names/logicals       */
const wchar_t   *defaultFileName,   /* => Default name and/or extension        */
int             option              /* => Open mode / search options           */
)
    {
    int         rtnval = 1;
    wchar_t     localOutputFileName[MAXFILELENGTH];
    wchar_t     localenv[4096];
    wchar_t     localIn[MAXFILELENGTH];
    wchar_t     tempString[MAXFILELENGTH];

    wchar_t     *pchr;
    Byte read;
#if 0 // WIP_BEFILENAME_PORTABILITY - can't depend on current working directory
    wchar_t     cwdDirectory[MAXDIRLENGTH];
    wchar_t     cwdDevice[MAXDEVICELENGTH];
    wchar_t     cwdPath[MAXDIRLENGTH];

    // initialize all directories, etc.
    cwdDirectory[0] = cwdPath[0] = cwdDevice[0] = 0;

    // get the current path and make sure it ends with the directory separator.
    BeStringUtilities::Wcsncpy (cwdPath, _countof(cwdPath)-1, BeGetCwd ().c_str());

    if ( (0 != cwdPath[0]) && (WCSDIR_SEPARATOR_CHAR != cwdPath[wcslen (cwdPath)-1]) )
        wcscat (cwdPath, WCSDIR_SEPARATOR);

    // parse the current working path into its device and dir
    parseName (cwdPath, cwdDevice, cwdDirectory, NULL, NULL, true);
#endif

    if ((NULL != envvar) && (0 != *envvar) )
        BeStringUtilities::Wcsncpy (localenv, _countof (localenv), envvar);
    else
        localenv[0] = 0;

    read = !((option & UF_OPEN_WRITE) || (option & UF_OPEN_CREATE) || (option & UF_TRY_WRITE_THEN_READ));

    localIn[0] = tempString[0] = 0;
    if ( (NULL != inName) && (0 != *inName) )
        {
        wchar_t testDev[MAXDEVICELENGTH];
        wchar_t testDir[MAXDIRLENGTH];

        // if there's a logical name in inName, expand it
        wcscpy (tempString, inName);
        if ((pchr = ::wcschr (tempString, L':')) && !(*pchr=0) && BeStringUtilities::Wcsupr (tempString) && ConfigurationManager::IsVariableDefined  (tempString))
            {
            // prepend the logical that's in inName to
            mdlFile_buildSeparatedList (localenv, NULL, tempString, envvar, NULL);

            // if opening for read, do current directory last if we have an environment variable.
            if (read)
                option |= UF_CUR_DIR_SWAP;

            // if any part of the specification follows the environment variable, copy it to localIn and parse from there.
            if (0 != *(pchr+1))
                {
                wcscpy (localIn, pchr+1);

                /*-----------------------------------------------------------------------
                If there was a logical on the input name and the input name contains a
                directory but doesn't start with the root (or a drive) then treat the
                input name as a relative path i.e. don't parse off the directory in later
                stages of searching.
                -----------------------------------------------------------------------*/
                testDev[0] = testDir[0] = 0;
                parseName (localIn, testDev, testDir, NULL, NULL, TO_BOOL (option & UF_FIND_FOLDER));
                if ((0 == testDev[0]) && (0 != testDir[0]) && (WCSDIR_SEPARATOR_CHAR != testDir[0]))
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
                // if no device, and there's a directory that doesn't start with DIR_SEPARATOR_CHAR, treat as a relative path.
                testDev[0] = testDir[0] = 0;
                parseName (inName, testDev, testDir, NULL, NULL, TO_BOOL (option & UF_FIND_FOLDER));
                if ((0 == testDev[0]) && (0 != testDir[0]) && (WCSDIR_SEPARATOR_CHAR != testDir[0]))
                    option |= UF_INTERNALUSE_RELATIVEPATH;
                }

            wcscpy (localIn, inName);
            }
        }

#if 0 // WIP_BEFILENAME_PORTABILITY - can't depend on current working directory
    // check the current directory first, if appropriate.
    if ( (0 == (option & UF_NO_CUR_DIR)) && ( (read && (! (option & UF_CUR_DIR_SWAP))) || (!read && (option & UF_CUR_DIR_SWAP))))
        {
        rtnval = findFile_searchDirectoryList (pHandle, localOutputFileName, localIn, cwdPath, defaultFileName, cwdDevice, cwdDirectory, option);
        }
    else
        {
        rtnval = 1;
        }
#endif

    // didn't find it in the current directory, search the environment variables
    if (SUCCESS != rtnval)
        {
#if 0 // WIP_BEFILENAME_PORTABILITY - can't depend on current working directory
        rtnval = findFile_breakUpEnvVar (pHandle, localOutputFileName, localIn, localenv, defaultFileName, cwdDevice, cwdDirectory, option);
#else
        rtnval = findFile_breakUpEnvVar (pHandle, localOutputFileName, localIn, localenv, defaultFileName, NULL, NULL, option | UF_NO_CUR_DIR);
#endif
        }

#if 0 // WIP_BEFILENAME_PORTABILITY - can't depend on current working directory
    // didn't find it in the environment variable list. If we wanted to look in the current directory last, try that now.
    if ( (SUCCESS != rtnval) && (UF_WTR_SUCCESS != rtnval) )
        {
        if ( (0 == (option & UF_NO_CUR_DIR)) && ( (read && (option & UF_CUR_DIR_SWAP)) || (!read && (! (option & UF_CUR_DIR_SWAP))) ) )
            {
            rtnval = findFile_searchDirectoryList (pHandle, localOutputFileName, localIn, cwdPath, defaultFileName, cwdDevice, cwdDirectory, option);
            }
        }
#endif

    if (NULL != outName)
        {
        WString fullName;
        BeFileName::BeGetFullPathName (fullName, localOutputFileName);
        BeStringUtilities::Wcsncpy (outName, MAXFILELENGTH, fullName.c_str());
        }

    return  rtnval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      util_findFileInPath
(
WCharP  expandedNameP,
WCharCP filenameP,
WCharCP extensionP
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
