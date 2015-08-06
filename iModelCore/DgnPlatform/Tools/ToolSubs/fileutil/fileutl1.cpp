/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/fileutil/fileutl1.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include    <fcntl.h>
#include    <stdlib.h>
#include    <string.h>
#include    <errno.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <time.h>

#include    <Bentley/BeFileName.h>
#include    <Bentley/ScopedArray.h>
#include    <DgnPlatform/DesktopTools/fileutil.h>
#include    <DgnPlatform/DesktopTools/msfilutl.h>

USING_NAMESPACE_BENTLEY_DGN

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Dan.East                        12/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlFile_abbreviateNameEx
(
WChar     *fileName,      /* MODIFIED: to be at most maxLength characters */
size_t      maxLength
)
    {
    char    dirSeparator = DIR_SEPARATOR_CHAR;
    WChar tmpName[2 * MAXFILELENGTH];
    WChar sepName[2 * MAXFILELENGTH];
    WChar sep1Name[2 * MAXFILELENGTH];
    WChar *p, *pSep, *pSepFirst, *pSepSecond=NULL;

    wcscpy (tmpName, fileName);
    wcscpy (sepName, fileName);
    sep1Name[0] = 0;

    if (NULL == ::wcschr (sepName, DIR_SEPARATOR_CHAR))
        dirSeparator = '/';      //  for StartPoint's DocumentManager

    if (wcslen (fileName) >= (size_t)maxLength-3)
        {
        if (pSepFirst = ::wcschr (sepName, dirSeparator))
            {
            if (NULL == (pSepSecond = ::wcschr(pSepFirst+1, dirSeparator)))
                {
                pSepSecond = pSepFirst;
                }
            }

        if (::wcsrchr (sepName, dirSeparator) == &sepName[wcslen(sepName)-1])
            {
            for (p = sepName; (wcslen (p) > (size_t)maxLength-3); p = pSep)
                {
                if ((pSep = ::wcschr (p+1, dirSeparator)) == NULL)
                    break;
                }
            if (p > pSepSecond)
                {
                if (NULL != pSepSecond)
                    *(pSepSecond+1) = 0;

                wcscpy (fileName, sepName);
                wcscat (fileName, L"...");
                wcscat (fileName, p);
                }
            }
        else
            {
            for (p = sepName; (wcslen (p) > (size_t)maxLength-3); p = pSep)
                {
                if ((pSep = ::wcschr (p+1, dirSeparator)) == NULL)
                    break;
                }

            if (p > pSepSecond)
                {
                if (NULL != pSepSecond)
                    *(pSepSecond+1) = '\0';

                wcscpy (fileName, sepName);
                wcscat (fileName, L"...");
                wcscat (fileName, p);
                }
            }

        }
    else
        {
        mdlFile_abbreviateName (fileName, maxLength);
        }
     }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    Dan.East                        11/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlFile_abbreviateName
(
WCharP      fileName,      /* MODIFIED: to be at most maxLength characters */
size_t      maxLength
)
    {
    WChar     tmpName[2 * MAXFILELENGTH];
    WChar     *p, *pSep;

    wcscpy (tmpName, fileName);
    for (p = tmpName; (wcslen (p) > maxLength-3); p = pSep)
        {
        if ((pSep = ::wcschr (p+1, WCSDIR_SEPARATOR_CHAR)) == NULL)
            break;
        }

    if (p != tmpName)
        {
        wcscpy (fileName, L"...");
        wcscat (fileName, p);
        }
    }

/*----------------------------------------------------------------------------------*//**
* Put quotes around a string.  Often used for file names with spaces around them. (Unicode)
*
* @bsimethod                    Jean.Lalande                  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt      mdlFile_quoteString
(
WCharP     fileName,
size_t       maxBufferLength
)
    {
    if (NULL == fileName)
        return (ERROR);

    size_t      len;
    if ((len = wcslen (fileName)) > maxBufferLength-3)
        return (ERROR);

    ScopedArray<wchar_t> bufferSA (len * sizeof(WChar) + 3 * sizeof(WChar));
    WCharP buffer = bufferSA.GetData();

    wcscpy (buffer, L"\"");
    wcscat (buffer, fileName);
    wcscat (buffer, L"\"");

    wcsncpy (fileName, buffer, maxBufferLength);
    fileName [maxBufferLength-1] = L'\0';
    return (SUCCESS);
    }
    
/*----------------------------------------------------------------------------------*//**
* Remove quotes around a string.  Often used for file names with spaces around them. (Unicode)
*
* @bsimethod                    Jean.Lalande                  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public void      mdlFile_unquoteString
(
WCharP    fileName
)
    {
    /* check for quoted file names */
    if (fileName[0] == L'\"')
        {
        size_t    i, len = wcslen (fileName);
        WCharP pEndQuote;

        for (i=0; i<len; i++) /* Go far enough to pick up \0 */
            fileName[i] = fileName[i+1];

        if (NULL != (pEndQuote = ::wcsrchr (fileName, L'\"')))
            *pEndQuote = L'\0';
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   util_findDirComponents
(
WCharP    pathComponents[],
int         arraySize,
WCharP    directory
)
    {
    WCharP    pAltSep;
    WCharP    pPrevSep;
    WCharP    pSep;
    int         iSep;

    // replace all ALT_SEPARATORS with SEPARATORS.
    for (pAltSep = ::wcschr (directory, WCSALT_DIR_SEPARATOR_CHAR); NULL != pAltSep;  )
        {
        *pAltSep = DIR_SEPARATOR_CHAR;
        pAltSep++;
        }

    // skip leading directory separators.
    for (pPrevSep = directory; *pPrevSep == WCSDIR_SEPARATOR_CHAR; pPrevSep++)
        ;

    // find all the DIR_SEPARATORS, replace them with zeros, and put the components into the output array.
    for (iSep=0; iSep < (arraySize-1); iSep++)
        {
        pathComponents[iSep] = pPrevSep;
        if (NULL != (pSep = ::wcschr (pPrevSep, WCSDIR_SEPARATOR_CHAR)))
            {
            *pSep = 0;
            pSep++;
            pPrevSep = pSep;
            }
        else
            break;
        }

    // a NULL marks the end of the array.
    if (iSep < arraySize-1)
        {
        pathComponents[iSep] = NULL;
        return SUCCESS;
        }
    else
        return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static int     util_findIdenticalStrings
(
WCharP    setOne[],
WCharP    setTwo[]
)
    {
    int iSame;
    for (iSame=0; ; iSame++)
        {
        if ( (NULL == setOne[iSame]) || (NULL == setTwo[iSame]) )
            break;
        if (0 != BeStringUtilities::Wcsicmp (setOne[iSame], setTwo[iSame]))
            break;
        }
    return iSame;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    util_buildRelativePath
(
WCharP      relativePath,
WCharP      pathsBeyondSameInTarget[],
int         numBeyondSameInRoot
)
    {
    int     iRelativePath;

    *relativePath = 0;

    for (;numBeyondSameInRoot >0; numBeyondSameInRoot--)
        wcscat (relativePath, L"..\\");

    for (iRelativePath = 0; ; iRelativePath++)
        {
        if (NULL == pathsBeyondSameInTarget[iRelativePath])
            break;
        wcscat (relativePath, pathsBeyondSameInTarget[iRelativePath]);
        wcscat (relativePath, WCSDIR_SEPARATOR);
        }
    }

#define MAX_PATHDEPTH 20

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public void    mdlFile_findRelativePath
(
WCharP     outRelativePath,
WCharCP    targetFileName,
WCharCP    rootFileName
)
    {
    WCharP   targetDirComponents[MAX_PATHDEPTH];
    WChar    targetDir[MAXFILELENGTH];
    WChar    targetDevice[MAXDEVICELENGTH];
    WChar    targetName[MAXFILELENGTH];
    WChar    targetExtension[MAXEXTENSIONLENGTH];
    WChar    targetRelativeDir[MAXFILELENGTH];

    WCharP   rootDirComponents[MAX_PATHDEPTH];
    WChar    rootDevice[MAXDEVICELENGTH];
    WChar    rootDir[MAXFILELENGTH];

    rootDevice[0] = rootDir[0] = 0;
    mdlFile_parseName (rootFileName, rootDevice, rootDir, NULL, NULL);

    targetDevice[0] = targetDir[0] = targetName[0] = targetExtension[0] = 0;
    mdlFile_parseName (targetFileName, targetDevice, targetDir, targetName, targetExtension);

    if ( (0 == BeStringUtilities::Wcsicmp (targetDevice, rootDevice)) &&
         (SUCCESS == util_findDirComponents (rootDirComponents, sizeof(rootDirComponents)/sizeof(char *), rootDir)) &&
         (SUCCESS == util_findDirComponents (targetDirComponents, sizeof(targetDirComponents)/sizeof(char *), targetDir)) )
        {
        int     componentsSame = util_findIdenticalStrings (rootDirComponents, targetDirComponents);
        int     componentsBeyondSameInRoot;

        // find the number of paths beyond those that are the same in the root. That's how many we have to back up.
        for (componentsBeyondSameInRoot=0; NULL != rootDirComponents[componentsSame+componentsBeyondSameInRoot]; componentsBeyondSameInRoot++)
            ;
        util_buildRelativePath (targetRelativeDir, &targetDirComponents[componentsSame], componentsBeyondSameInRoot);
        mdlFile_buildName (outRelativePath, NULL, targetRelativeDir, targetName, targetExtension);
        return;
        }

    // if we got here, couldn't find relative path.
    wcscpy (outRelativePath, targetFileName);
    }

namespace { /* unnamed */
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
class DirectoryStack
    {
    int         m_numEntries;
    WCharP    m_stack[MAX_PATHDEPTH];
    bool        m_leadSep;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    DirectoryStack ()
        {
        m_numEntries = 0;
        m_stack[0]   = NULL;
        m_leadSep    = false;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~DirectoryStack ()
        {
        for (;;)
            {
            if (SUCCESS != Pop ())
                break;
            }
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    void        PushLeadSeparator () { m_leadSep = true; }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   ToDirString (WCharP outStr, size_t numChars)
        {
        if (0 >= numChars)
            return SUCCESS;

        outStr[0] = '\0';

        if (m_leadSep)
            {
            wcsncat (outStr, WCSDIR_SEPARATOR, numChars); outStr[numChars - 1] = '\0';
            numChars -= 1;
            }

        for (int iDir = 0; iDir < m_numEntries; iDir++)
            {
            size_t charsToCopy = 1 + wcslen (m_stack[iDir]);

            if (numChars < charsToCopy)
                return ERROR;

            wcsncat (outStr, m_stack[iDir], numChars); outStr[numChars - 1] = '\0';
            numChars -= (charsToCopy - 1);

            wcsncat (outStr, WCSDIR_SEPARATOR, numChars); outStr[numChars - 1] = '\0';
            numChars -= 1;
            }

        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   Push (WCharP newDir)
        {
        if (0 == newDir || '\0' == newDir[0])
            return ERROR;

        if (MAX_PATHDEPTH - 1  < m_numEntries)
            return ERROR;

        m_stack[m_numEntries] = BeStringUtilities::Wcsdup (newDir);
        m_numEntries++;

        m_stack[m_numEntries] = NULL;

        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   Pop  ()
        {
        if (0 >= m_numEntries)
            return ERROR;

        free (m_stack[m_numEntries-1]);
        m_numEntries--;

        m_stack[m_numEntries] = NULL;

        return SUCCESS;
        }

    };  // DirectoryStack
} // unnamed

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    mdlFile_resolveRelativePath
(
WCharP        outFullPath,
int             nChars,
WCharCP       relativeFileName,
WCharCP       basePath
)
    {
    WCharP    relativeDirComponents[MAX_PATHDEPTH];
    WChar     relativeDir[MAXFILELENGTH];
    WChar     relativeDevice[MAXDEVICELENGTH];
    WChar     relativeName[MAXFILELENGTH];
    WChar     relativeExtension[MAXEXTENSIONLENGTH];

    WCharP    baseDirComponents[MAX_PATHDEPTH];
    WChar     baseDevice[MAXDEVICELENGTH];
    WChar     baseDir[MAXFILELENGTH];

    bool            hasLeadSeparator = false;

    baseDevice[0] = baseDir[0] = 0;
    mdlFile_parseName (basePath, baseDevice, baseDir, NULL, NULL);

    relativeDevice[0] = relativeDir[0] = relativeName[0] = relativeExtension[0] = 0;
    mdlFile_parseName (relativeFileName, relativeDevice, relativeDir, relativeName, relativeExtension);

    if ('\\' == baseDir[0])
        hasLeadSeparator = true;

#if defined (DEBUG_RESOLVE_RELATIVE)
    printf ("Resolve Relative Path\n");
    printf ("  base path: %S\n", basePath);
    printf ("  relative : %S\n", relativeFileName);
#endif

    if (outFullPath)
        outFullPath[0] = 0;

    if (SUCCESS != util_findDirComponents (relativeDirComponents, _countof (relativeDirComponents), relativeDir) ||
        SUCCESS != util_findDirComponents (baseDirComponents, _countof (baseDirComponents), baseDir))
        return ERROR;

    WChar         fullPath[MAXFILELENGTH];
    DirectoryStack  dirStack;

    if (hasLeadSeparator)
        dirStack.PushLeadSeparator ();

    // Push the base path onto a stack.
    for (int iComponent = 0; NULL != baseDirComponents[iComponent]; iComponent++)
        dirStack.Push (baseDirComponents[iComponent]);

    // Push or Pop based on each dir in the relative path.
    for (int iComponent = 0; NULL != relativeDirComponents[iComponent]; iComponent++)
        {
        WCharP    pDir = relativeDirComponents[iComponent];

        if ('\0' == pDir[0])
            continue;

        if (0 == BeStringUtilities::Wcsicmp (L".", pDir))
            continue;

        if (0 == BeStringUtilities::Wcsicmp (L"..", pDir))
            dirStack.Pop ();
        else
            dirStack.Push (pDir);
        }

    dirStack.ToDirString (fullPath, nChars);

    mdlFile_buildName (outFullPath, baseDevice, fullPath, relativeName, relativeExtension);

#if defined (DEBUG_RESOLVE_RELATIVE)
    printf ("  full path: %S\n", outFullPath);
#endif

    return SUCCESS;
    }

