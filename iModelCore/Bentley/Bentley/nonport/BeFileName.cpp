/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeFileName.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    #include <windows.h>
    #include <objbase.h>
    #include <sys/utime.h>
    #include <sys/stat.h>
    #if !defined (BENTLEY_WINRT)
        #include <Shlwapi.h>
        #include <WinIoCtl.h>
        #include <io.h>
        #include <fcntl.h>
    #endif
#elif defined (__unix__)
    #include <limits.h>
    #include <stdarg.h>
    #include <libgen.h>
    #include <errno.h>
    #include <sys/stat.h>
    #if !defined (__APPLE__)
        #include <sys/vfs.h>
    #else
        #include <sys/mount.h>
    #endif 
    #include <utime.h>
    #include <dlfcn.h>
    #include <unistd.h>
    #if defined (ANDROID)
        #define S_IREAD S_IRUSR
        #define S_IWRITE S_IWUSR
    #endif
#else
    #error unknown compiler
#endif

#include <stdlib.h>
#include "../BentleyInternal.h"
#include <Bentley/WString.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeDebugLog.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeThread.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/bmap.h>
#include <Logging/bentleylogging.h>

#define TO_BOOL(x)              (0 != (x))

USING_NAMESPACE_BENTLEY

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #undef CreateFileW
    #undef DeleteFileW
    #undef CopyFileW
    #undef MoveFileW
    #undef SetFileAttributesW
    #define WINDOWS_EXTENDED_PATH_PREFIX L"\\\\?\\"
#elif defined (__unix__)
    typedef struct stat     T_Stat64;
#else
#error unknown runtime
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String toUtf8 (wchar_t const* w)
    {
    Utf8String u;
    BeStringUtilities::WCharToUtf8 (u, w);
    return u;
    }

#if defined (__unix__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static WString fromUtf8 (char const* u)
    {
    WString w;
    BeStringUtilities::Utf8ToWChar (w, u);
    return w;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus beStat (T_Stat64& status, wchar_t const* fn)
    {
    int e = stat (toUtf8(fn).c_str(), &status);
    return (e==-1)? ERROR: SUCCESS;
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                  04/11
+---------------+---------------+---------------+---------------+--------------+------*/
BeFileName::BeFileName (CharCP name, bool isUtf8)
    {
    if (isUtf8)
        SetNameUtf8 (name);
    else
        SetNameA (name);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                   08/12
+---------------+---------------+---------------+---------------+--------------+------*/
BeFileName::BeFileName (CharCP name, BentleyCharEncoding encoding)
    {
    if (BentleyCharEncoding::Utf8 == encoding)
        SetNameUtf8 (name);
    else
        SetNameA (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::GetNameA (char name[MAX_PATH]) const 
    {
    BeStringUtilities::WCharToCurrentLocaleChar (name, this->c_str(), MAX_PATH);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::GetNameA (CharP name, size_t numBytes) const 
    {
    BeStringUtilities::WCharToCurrentLocaleChar (name, this->c_str(), numBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/11
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeFileName::GetNameUtf8 () const
    {
    return toUtf8 (this->c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeFileName::GetUri () const
    {
    Utf8String nameUtf8 (this->c_str());
    Utf8String uri (nameUtf8[0] == '/' ? "file://" : "file:///");
    uri.append (nameUtf8.c_str());

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    uri.ReplaceAll ("\\", "/");
#endif

    return uri;
    }

#if defined (__unix__)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void fixDirSeparators (WCharP name)
    {
    for (WCharP p = name; *p; p++)
        {
        if (*p == WCSALT_DIR_SEPARATOR_CHAR)
            *p = WCSDIR_SEPARATOR_CHAR;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::SetNameA (CharCP name)
    {
    WString nameW (name, BentleyCharEncoding::Locale);
    SetName (nameW.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::SetName (WCharCP name)
    {
    if (!name)
        {
        Clear ();
        return;
        }

    // also accept URI style names
    if (0 == wcsncmp (name, L"file:///", 8))
        name += 7;

    FixPathName (*this, name, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::SetNameUtf8 (Utf8CP name)
    {
    WString nameW (name, BentleyCharEncoding::Utf8);
    SetName (nameW.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName::BeFileName (FileNameParts mask, WCharCP fullName)
    {
    WString    fileDev, fileDir, fileBase, fileExt;
    ParseName (&fileDev, &fileDir, &fileBase, &fileExt, fullName);
    BuildName ((mask & Device)    ? fileDev.c_str() : NULL, 
               (mask & Directory) ? fileDir.c_str() : NULL, 
               (mask & Basename)  ? fileBase.c_str() : NULL, 
               (mask & Extension) ? fileExt.c_str() : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::SupplyDefaultNameParts (WCharCP defaultName)
    {
    WString dev, dir, name, ext;
    ParseName (&dev, &dir, &name, &ext);

    WString defdev, defdir, defname, defext;
    ParseName (&defdev, &defdir, &defname, &defext, defaultName);

    if (dev.empty()) dev = defdev;
    if (dir.empty()) dir = defdir;
    if (name.empty()) name = defname;
    if (ext.empty()) ext = defext;
    
    BuildName (dev.c_str(), dir.c_str(), name.c_str(), ext.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::OverrideNameParts (WCharCP overrideName)
    {
    WString dev, dir, name, ext;
    ParseName (&dev, &dir, &name, &ext, overrideName);

    WString defdev, defdir, defname, defext;
    ParseName (&defdev, &defdir, &defname, &defext);

    if (dev.empty()) dev = defdev;
    if (dir.empty()) dir = defdir;
    if (name.empty()) name = defname;
    if (ext.empty()) ext = defext;
    
    BuildName (dev.c_str(), dir.c_str(), name.c_str(), ext.c_str());
    }

/*---------------------------------------------------------------------------------**//**
    path - Full path buffer.

    drive - Contains a letter (A, B, and so on) corresponding to the desired drive and an optional trailing colon. _makepath inserts 
    the colon automatically in the composite path if it is missing. If drive is NULL or points to an empty string, no drive letter 
    appears in the composite path string.

    dir - Contains the path of directories, not including the drive designator or the actual file name. The trailing slash is optional, 
    and either a forward slash (/) or a backslash (\) or both might be used in a single dir argument. If no trailing slash (/ or \) is 
    specified, it is inserted automatically. If dir is NULL or points to an empty string, no directory path is inserted in the composite path string.

    fname - Contains the base file name without any file name extensions. If fname is NULL or points to an empty string, no filename is inserted in 
    the composite path string.

    ext - Contains the actual file name extension, with or without a leading period (.). _makepath inserts the period automatically if it does not 
    appear in ext. If ext is NULL or points to an empty string, no extension is inserted in the composite path string.

* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::BuildName (WCharCP dev, WCharCP dir, WCharCP name, WCharCP ext) 
    {
    WString combinedPath;
    BeFileName::BuildName (combinedPath, dev, dir, name, ext);

    SetName (combinedPath.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2012
//---------------------------------------------------------------------------------------
void BeFileName::BuildName (WStringR combinedPath, WCharCP dev, WCharCP dir, WCharCP name, WCharCP ext)
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    {
    WChar outPath[4*MAX_PATH];
    if (0 != _wmakepath_s (outPath, 4*MAX_PATH, dev, dir, name, ext))
        *outPath = 0;
    
    combinedPath = outPath;
    }
#elif defined (__unix__)
    {
    combinedPath.clear ();
    combinedPath.reserve (MAX_PATH);

    // dev is ignored for unix.
    if (dir && *dir)
        {
        combinedPath.append (dir);

        size_t backOfPath = combinedPath.length () - 1;
        if ((WCSDIR_SEPARATOR_CHAR != combinedPath[backOfPath]) && (WCSALT_DIR_SEPARATOR_CHAR != combinedPath[backOfPath]))
            combinedPath.append (WCSDIR_SEPARATOR);
        }

    if (name && *name)
        {
        combinedPath.append (name);
        }

    if (ext && *ext)
        {
        if (L'.' != *ext)
            combinedPath.append (L".");

        combinedPath.append (ext);
        }

    BeAssert (combinedPath.length () < MAX_PATH-1);
    }
#else
#error unknown runtime
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar 05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::ParseNameNoClear (WStringP dev, WStringP dir, WStringP name, WStringP ext, WCharCP fullName)
    {
    WString devStr, dirStr, nameStr, extStr;

    BeFileName::ParseName (&devStr, &dirStr, &nameStr, &extStr, fullName);

    if (NULL != dev && ! devStr.empty())
        *dev = devStr;
    if (NULL != dir && ! dirStr.empty())
        *dir = dirStr;
    if (NULL != name && ! nameStr.empty())
        *name = nameStr;
    if (NULL != ext && ! extStr.empty())
        *ext = extStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::ParseName (WStringP dev, WStringP dir, WStringP name, WStringP ext, WCharCP fullName)
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    {   // Windows implementation
    if (!fullName || !*fullName)
        return;

    wchar_t fileDev[MAX_PATH], fileDir[4*MAX_PATH], fileBase[MAX_PATH], fileExt[MAX_PATH];

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fullNameFixed;
    FixPathName (fullNameFixed, fullName, true);

    // NOTE: _wsplitpath_s can deal with either windows or unix directory separators
    if ( (NULL == fullName) || (0 == *fullName) || (0 != _wsplitpath_s (fullNameFixed.c_str(), fileDev, MAX_PATH, fileDir, 4*MAX_PATH, fileBase, MAX_PATH, fileExt, MAX_PATH)) )
        {
        // set all outputs to empty strings.
        fileDev[0] = fileDir[0] = fileBase[0] = fileExt[0] = 0;
        }

    if (NULL != dev)
        {
        size_t len = wcslen (fileDev);
        if ( (len > 0) && (':' == fileDev[len-1]) )
            fileDev[len-1] = 0;
        dev->assign (fileDev);
        }

    if (NULL != dir)
        {
        dir->assign (fileDir);
        }

    if (NULL != name)
        {
        name->assign (fileBase);
        }

    if (NULL != ext)
        {
        if ('.' == fileExt[0])
            ext->assign (&fileExt[1]);
        else
            ext->assign (fileExt);
        }
    }
#elif defined (__unix__)
    {
    //
    //  See http://linux.about.com/library/cmd/blcmdl3_dirname.htm for dirname / basename behavior
    //  See comment regarding dirname / basename in Android version of <libgen.h>
    //
    wchar_t baseNameW[MAX_PATH];
    WCharP  lastDot;

    // device always empty in unix.
    if (NULL != dev)
        dev->clear ();

    if (NULL == fullName)
        {
        *baseNameW = 0;
        lastDot = NULL;
        }
    else
        {
        wchar_t fullNameFixedW[MAX_PATH];
        WCharCP lastDirSeparator;

        // handle the case where fullName has Windows backslashes
        wcsncpy (fullNameFixedW, fullName, MAX_PATH);
        fullNameFixedW[MAX_PATH-1] = 0;
        fixDirSeparators (fullNameFixedW);

        // look for the last directory separator
        if (NULL == (lastDirSeparator = wcsrchr (fullNameFixedW, WCSDIR_SEPARATOR_CHAR)))
            {
            if (NULL != dir)
                dir->clear ();

            wcsncpy (baseNameW, fullNameFixedW, MAX_PATH);
            }
        else
            {
            if (NULL != dir)
                dir->assign (fullNameFixedW, 1+(lastDirSeparator-fullNameFixedW));

            wcsncpy (baseNameW, lastDirSeparator+1, MAX_PATH);
            }

        // split the basename into name and extension.
        if (NULL != (lastDot = wcsrchr (baseNameW, '.')))
            *lastDot = 0;
        }

    if (NULL != name)
        name->assign (baseNameW);

    if (NULL != ext)
        {
        if (NULL == lastDot)
            ext->clear ();
        else
            ext->assign (lastDot+1);
        }
    }
#else
#error unknown runtime
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brandon.Bohrer  12/2011
//---------------------------------------------------------------------------------------
WString BeFileName::GetDirectoryName (WCharCP path)
    {
    WString dev, dir;

    ParseName (&dev, &dir, NULL, NULL, path);
    return WString (BeFileName (dev.c_str (), dir.c_str (), NULL, NULL).GetName ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName BeFileName::GetDirectoryName() const
    {
    WString dev, dir;
    ParseName (&dev, &dir, NULL, NULL, GetName());
    return BeFileName (dev.c_str(), dir.c_str(), NULL, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brandon.Bohrer  12/2011
//---------------------------------------------------------------------------------------
WString BeFileName::GetDirectoryWithoutDevice (WCharCP path)
    {
    WString dir;
    ParseName (NULL, &dir, NULL, NULL, path);
    return WString (BeFileName (NULL, dir.c_str (), NULL, NULL).GetName ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName BeFileName::GetDirectoryWithoutDevice() const
    {
    WString dir;
    ParseName (NULL, &dir, NULL, NULL, GetName());
    return BeFileName (NULL, dir.c_str(), NULL, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brandon.Bohrer  12/2011
//---------------------------------------------------------------------------------------
WString BeFileName::GetExtension (WCharCP path)
    {
    WString ext;
    ParseName (NULL, NULL, NULL, &ext, path);
    return ext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString BeFileName::GetExtension() const
    {
    WString ext;
    ParseName (NULL, NULL, NULL, &ext, GetName());
    return ext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brandon.Bohrer  12/2011
//---------------------------------------------------------------------------------------
WString BeFileName::GetFileNameWithoutExtension (WCharCP path)
    {
    WString base;
    ParseName (NULL, NULL, &base, NULL, path);
    return base;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString BeFileName::GetFileNameWithoutExtension() const
    {
    WString base;
    ParseName (NULL, NULL, &base, NULL, GetName());
    return base;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brandon.Bohrer  12/2011
//---------------------------------------------------------------------------------------
WString BeFileName::GetDevice (WCharCP path)
    {
    WString dev;
    ParseName (&dev, NULL, NULL, NULL, path);
    return dev;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
WString BeFileName::GetDevice() const
    {
    WString dev;
    ParseName (&dev, NULL, NULL, NULL, GetName());
    return dev;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsAbsolutePath () const
    {
    if (empty())
        return false;
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    return (at(0) == '\\' || (length() > 2 && at(1)== ':'));
#elif defined (__unix__)
    return at(0) == '/';
#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsUrl() const
    {
    return IsUrl(c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsUrl(WCharCP filename)
    {
    return NULL != filename && (0 == wcsncmp(L"http:", filename, 5) || 0 == wcsncmp(L"https:", filename, 6))  ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   12/07
//---------------------------------------------------------------------------------------
void BeFileName::AppendToPath (WStringR path, WCharCP val)
    {
    if (!val || !*val)
        return;

    WString newPath (path.c_str());
    if (!newPath.empty())
        AppendSeparator (newPath);

    newPath.append (val);
    FixPathName (path, newPath.c_str(), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      08/11
//---------------------------------------------------------------------------------------
BeFileNameR BeFileName::AppendToPath (WCharCP val)
    {
    if (!val || !*val)
        return *this;

    WString newPath (GetName());
    if (!newPath.empty())
        AppendSeparator (newPath);

    newPath.append (val);
    SetName (newPath.c_str());
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Barry.Bentley  01/12
//---------------------------------------------------------------------------------------
BeFileNameR BeFileName::AppendExtension (WCharCP extension)
    {
    if ( (NULL == extension) || (0 == *extension) )
        return *this;

    WString newPath (GetName());
    if ('.' != at (newPath.length()-1))
        newPath.append (L".");

    newPath.append (extension);
    SetName (newPath.c_str());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::AppendSeparator (WStringR path)
    {
    if (path.empty() || (*(path.end()-1) != WCSDIR_SEPARATOR_CHAR))
        path.append (WCSDIR_SEPARATOR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::AppendSeparator ()
    {
    AppendSeparator (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::PopDir ()
    {
    if (this->length() == 0)
        return;

    WString::iterator lastChar = this->end()-1;
        
    // A trailing separator doesn't count.
    if (*lastChar == WCSDIR_SEPARATOR_CHAR)
        --lastChar;
        
    while (lastChar > this->begin() && *lastChar != WCSDIR_SEPARATOR_CHAR)
        --lastChar;

    this->erase (lastChar, this->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void util_findDirComponents (bvector<WString>& pathComponents, WCharCP directory)
    {
    WString path(directory);

    path.ReplaceAll (WCSALT_DIR_SEPARATOR, WCSDIR_SEPARATOR);
    BeStringUtilities::Split (path.c_str(), WCSDIR_SEPARATOR, NULL, pathComponents);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t util_findIdenticalStrings (bvector<WString> const& setOne, bvector<WString> const& setTwo)
    {
    size_t iSame;
    for (iSame=0; ; iSame++)
        {
        if (setOne.size() <= iSame || setTwo.size() <= iSame)
            break;
        if (0 != BeStringUtilities::Wcsicmp (setOne[iSame].c_str(), setTwo[iSame].c_str()))
            break;
        }
    return (uint32_t) iSame;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void util_buildRelativePath (WStringR relativePath, bvector<WString> const& targetPathComponents, uint32_t numSame, uint32_t numBeyondSameInRoot)
    {
    relativePath.clear();

    for (;numBeyondSameInRoot >0; numBeyondSameInRoot--)
        {
        relativePath.append (L"..");
        relativePath.append (WCSDIR_SEPARATOR);
        }

    for (size_t iRelativePath = numSame; iRelativePath < targetPathComponents.size(); iRelativePath++)
        {
        relativePath.append (targetPathComponents[iRelativePath]);
        relativePath.append (WCSDIR_SEPARATOR);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeFileName::FindRelativePath (WStringR outRelativePath, WCharCP targetFileName, WCharCP rootFileName)
    {
    WString             targetDir;
    WString             targetDevice;
    WString             targetName;
    WString             targetExtension;

    WString             rootDevice;
    WString             rootDir;

    BeFileName::ParseName (&rootDevice, &rootDir, NULL, NULL, rootFileName);
    BeFileName::ParseName (&targetDevice, &targetDir, &targetName, &targetExtension, targetFileName);

    if ( (0 == BeStringUtilities::Wcsicmp (targetDevice.c_str(), rootDevice.c_str())) )
        {
        bvector<WString>    targetDirComponents;
        bvector<WString>    rootDirComponents;

        util_findDirComponents (rootDirComponents, rootDir.c_str());
        util_findDirComponents (targetDirComponents, targetDir.c_str());

        // find the number of paths beyond those that are the same in the root. That's how many we have to back up.
        uint32_t    componentsSame = util_findIdenticalStrings (rootDirComponents, targetDirComponents);
        uint32_t    componentsBeyondSameInRoot = (uint32_t) rootDirComponents.size() - componentsSame;

        WString     targetRelativeDir;

        util_buildRelativePath (targetRelativeDir, targetDirComponents, componentsSame, componentsBeyondSameInRoot);
        BeFileName::BuildName (outRelativePath, NULL, targetRelativeDir.c_str(), targetName.c_str(), targetExtension.c_str());
        return;
        }

    // if we got here, couldn't find relative path.
    outRelativePath.assign (targetFileName);
    }

namespace { /* unnamed */
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
class DirectoryStack
    {
    bvector<WString>    m_stack;
    bool                m_leadSep;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    /* ctor */  DirectoryStack () : m_leadSep(false) {}
    void        PushLeadSeparator () { m_leadSep = true; }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   ToDirString (WStringR outStr)
        {
        outStr.clear();

        if (m_leadSep)
            outStr.append (WCSDIR_SEPARATOR);

        FOR_EACH (WString const& dirName, m_stack)
            {
            outStr.append (dirName);
            outStr.append (WCSDIR_SEPARATOR);
            }

        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   Push (WCharCP newDir)
        {
        if (0 == newDir || '\0' == newDir[0])
            return ERROR;

        m_stack.push_back (newDir);
        return SUCCESS;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    02/06
    +---------------+---------------+---------------+---------------+-----------+------*/
    StatusInt   Pop  ()
        {
        if (0 >= m_stack.size())
            return ERROR;

        m_stack.pop_back();
        return SUCCESS;
        }

    };  // DirectoryStack
} // unnamed

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeFileName::ResolveRelativePath (WStringR outFullPath, WCharCP relativeFileName, WCharCP basePath)
    {
    WString             relativeDir;
    WString             relativeDevice;
    WString             relativeName;
    WString             relativeExtension;

    WString             baseDevice;
    WString             baseDir;

    bool        hasLeadSeparator = false;
    BeFileName::ParseName (&baseDevice, &baseDir, NULL, NULL, basePath);
    BeFileName::ParseName (&relativeDevice, &relativeDir, &relativeName, &relativeExtension, relativeFileName);

    if ( ! relativeDevice.empty())
        {
        // If the input path is absolute, then we cannot make a path relative to basePath
        outFullPath.assign (relativeFileName);
        return SUCCESS;
        }

    if (DIR_SEPARATOR_CHAR == baseDir[0])
        hasLeadSeparator = true;

    outFullPath.clear();

    bvector<WString>    relativeDirComponents;
    bvector<WString>    baseDirComponents;

    util_findDirComponents (relativeDirComponents, relativeDir.c_str());
    util_findDirComponents (baseDirComponents, baseDir.c_str());

    DirectoryStack  dirStack;

    if (hasLeadSeparator)
        dirStack.PushLeadSeparator ();

    // Push the base path onto a stack.
    FOR_EACH (WStringCR dirComponent, baseDirComponents)
        dirStack.Push (dirComponent.c_str());

    // Push or Pop based on each dir in the relative path.
    FOR_EACH (WStringCR dirComponent, relativeDirComponents)
        {
        if (dirComponent.empty())
            continue;

        if (dirComponent.EqualsI (L"."))
            continue;

        if (dirComponent.EqualsI (L".."))
            dirStack.Pop ();
        else
            dirStack.Push (dirComponent.c_str());
        }

    WString fullPath;
    dirStack.ToDirString (fullPath);

    BeFileName::BuildName (outFullPath, baseDevice.c_str(), fullPath.c_str(), relativeName.c_str(), relativeExtension.c_str());
    return SUCCESS;
    }

#if defined (_WIN32)
// ---------------------------------------------------------------------------------
// This mechanism is to prevent checking for existence of UNC drives multiple times,
//   each time causing a substantial delay while the network timeout expires.
//   We keep track of the UNC drives that have failed and look in that failed table
//   for subsequent checks.
// ---------------------------------------------------------------------------------
#define     RETRY_AFTER_MILLISECONDS        1200000  // 20 minutes.
#define     APPARENTLY_DEAD_MILLISECONDS    5000     // 5 seconds.

typedef bmap<WString,double>  T_ServerMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
static bool UNCPathExists (WCharCP filename)
    {
    static T_ServerMap *s_timedOutUNCServers;

    // called only if first two characters in filename are \\.
    WCharCP    serverName = &filename[2];
    WCharCP    endServerName;

    if (NULL == (endServerName = wcschr (serverName, '\\')))
        {
        assert (false);
        endServerName = serverName + wcslen (serverName);
        }

    // make a local copy of just the UNC server part of the name.
    ptrdiff_t   numChars        = endServerName - serverName;
    WCharP      localServerName = (WCharP) _alloca ( (numChars+1) * sizeof (WChar));
    wcsncpy (localServerName, serverName, numChars);
    localServerName[numChars] = 0;

    double timeNow = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    T_ServerMap::iterator   entry;
    if ( (NULL != s_timedOutUNCServers) && (s_timedOutUNCServers->end() != (entry = s_timedOutUNCServers->find (localServerName))) )
        {
        if ( (timeNow - entry->second) < RETRY_AFTER_MILLISECONDS)
            return false;
        else
            s_timedOutUNCServers->erase (entry);
        }

    // if we got here, we didn't find it, or it's been too long since we last tried it.
#if defined(BENTLEY_WINRT)
    bool exists = -1 != _waccess(filename,0);
#else
    bool exists = TO_BOOL(::PathFileExistsW(filename));
#endif
    if ( !exists && ((BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble() - timeNow) > APPARENTLY_DEAD_MILLISECONDS) )
        {
        if (NULL == s_timedOutUNCServers)
            s_timedOutUNCServers = new T_ServerMap();

        WString  serverNameString(localServerName);
        (*s_timedOutUNCServers)[serverNameString] = timeNow;
        }

    return exists;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::DoesPathExist(WCharCP path)
    {
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    if ((NULL != path) && ('\\' == path[0]) && ('\\' == path[1]) && ('?' != path[2]))
        return UNCPathExists (path);

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString pathFixed;
    BeFileNameStatus status = FixPathName (pathFixed, path, false);
    
    if (BeFileNameStatus::Success != status)
        return false;
    
    return -1 != _waccess (pathFixed.c_str(), 0);

#elif defined (__unix__)

    Utf8String pathUtf8;
    BeStringUtilities::WCharToUtf8 (pathUtf8, path);
    return (-1 != access (pathUtf8.c_str (), F_OK));

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsDirectory() const
    {
    return BeFileName::IsDirectory (this->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsDirectory(WCharCP path)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString pathFixed;
    FixPathName (pathFixed, path, false);

    BeFileName resolvedPath (pathFixed);
    
    // resolve the target if it's a link.
    if (IsSymbolicLink(pathFixed.c_str()) && (SUCCESS != GetTargetOfSymbolicLink(resolvedPath, pathFixed.c_str())))
        BeAssert(false);
    
    if (WCSDIR_SEPARATOR_CHAR == resolvedPath[resolvedPath.size() - 1])
        resolvedPath.resize(resolvedPath.size() - 1);

    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (GetFileAttributesExW (resolvedPath.GetName(), GetFileExInfoStandard, &fileAttributeData))
        return (0 != (fileAttributeData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

    return false;

#elif defined (__unix__)

    struct stat info;
    if (0 != stat (Utf8String(path).c_str (), &info))
        return false;
    return S_ISDIR(info.st_mode);

#else
#error unknown runtime
#endif
    }

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool isRelativePath (WCharCP path)
    {
    if ('.' == path[0])
        return true;

    if ((WCSDIR_SEPARATOR_CHAR == path[0]) || (WCSALT_DIR_SEPARATOR_CHAR == path[0]))
        return false;

    if (':' == path[1])
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool hasExtendedPathPrefix (WCharCP path)
    {
    return (0 == wcsncmp (path, WINDOWS_EXTENDED_PATH_PREFIX, wcslen (WINDOWS_EXTENDED_PATH_PREFIX)));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::FixPathName(WStringR path, WCharCP src, bool keepTrailingSeparator)
    {
    if (!src || !*src)
        return  BeFileNameStatus::IllegalName;

    // skip over any leading whitespace
    for (; *src && iswspace (*src); src++)
        ;

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    bool extendedPathPrefixNeeded = false;
    
    // Patch to support URLs (http or https) ... max length is irrelevant and no relative paths possible.
    // IMPORTANT NOTE: This code is necessary on the DgnDb06 stream for ConceptStation but should be unrequired
    // on newer stream since the use of BeFileName for URLs has been eliminated in ThreeMX
    // DO NOT PORT
    if (!IsUrl(src))
        {
        if (isRelativePath (src))
            {
            // Windows does not allow relative path names to be >= MAX_PATH
            if (wcslen (src) >= MAX_PATH)
                return  BeFileNameStatus::IllegalName;
            }
        else
            {
            // Need the "extended path prefix" if name would be too long and if prefix is not already there
            // MAX_PATH - 12 is the maximum length for a directory name (keeping room for an 8.3 filename)
            if ((wcslen (src) >= (MAX_PATH - 12)) && !hasExtendedPathPrefix (src))
                extendedPathPrefixNeeded = true;
            }
        }

    size_t numToAllocate = wcslen (src) + 1;
    if (extendedPathPrefixNeeded)
        numToAllocate += wcslen (WINDOWS_EXTENDED_PATH_PREFIX);
        
    ScopedArray<WChar, (MAX_PATH * sizeof (WChar))> tmp1Buff (numToAllocate);
    WCharP tmp1 = tmp1Buff.GetData();
    WCharP dst=tmp1;

    if (extendedPathPrefixNeeded)
        {
        wcscpy (dst, WINDOWS_EXTENDED_PATH_PREFIX);
        dst += wcslen (WINDOWS_EXTENDED_PATH_PREFIX);
        }

    // Patch to supportn URLs ... forward slash and double forward slashes are preserved
    // Patch to support URLs (http or https) ... max length is irrelevant and no relative paths possible.
    // IMPORTANT NOTE: This code is necessary on the DgnDb06 stream for ConceptStation but should be unrequired
    // on newer stream since the use of BeFileName for URLs has been eliminated in ThreeMX
    // DO NOT PORT
    if (IsUrl(src))
        {
        wcscpy(dst, src);
        }
    else
        {
        for (; *src; ++src, ++dst)
            {
            *dst = ('/' == *src) ? '\\' : *src;

            // don't copy embedded multiple backskashes (keep leading double backslash for UNC's)
            if ('\\' == dst[0]  &&  ('\\' == src[1] || '/'== src[1]) && dst != tmp1)
                dst--;
            }
        
        *dst = '\0';

        if (('\\' == dst[-1]) && !keepTrailingSeparator)
            dst[-1] = '\0';

        }

    //Remove dots, compress relative paths, etc.
    ScopedArray<WChar, (MAX_PATH * sizeof (WChar))> tmpBuff(wcslen(tmp1) + 1);
    WCharP tmp = tmpBuff.GetData();
    wcscpy (tmp, tmp1);

  #if defined (BENTLEY_WIN32)

    // NOTE: PathCanonicalizeW does not support names >= MAX_PATH
    if (wcslen (tmp1) < MAX_PATH)
        {
        WCharCP root = ::PathSkipRootW(tmp1);
        if (NULL != root)
            {
            size_t lenRoot = root-tmp1;
            if (!::PathCanonicalizeW(tmp+lenRoot, tmp1+lenRoot))
                return  BeFileNameStatus::IllegalName;

            }
        else if ( '.' == tmp1[0])
            {
            int skipD = 1;
            while (('.' == tmp1[skipD]) || (WCSDIR_SEPARATOR_CHAR == tmp1[skipD]))
                skipD++;

            if (!::PathCanonicalizeW(tmp+skipD, tmp1+skipD))
                return  BeFileNameStatus::IllegalName;
            }
        else
            {
            if (!::PathCanonicalizeW(tmp, tmp1))
                return  BeFileNameStatus::IllegalName;
            }
        }

  #endif

    path.assign (tmp);
    path.Trim();
    return  BeFileNameStatus::Success;

#elif defined (__unix__)

  #if !defined (ANDROID)
    // realpath is unreliable on Android
    char buf[MAX_PATH];
    if (realpath (toUtf8(src).c_str(), buf))
        {
        BeStringUtilities::Utf8ToWChar (path, buf);
        
        if (keepTrailingSeparator)
            {
            // realpath removes trailing separators which can cause problems downstream
            WCharCP lastSrcChar = src += wcslen (src) - 1;
            if (WCSDIR_SEPARATOR_CHAR == *lastSrcChar)
                BeFileName::AppendSeparator (path);
            }
        
        return BeFileNameStatus::Success;
        }
  #endif

    //  File does not exist. Do some simple fix-ups.
    path.reserve (wcslen (src));
    path.resize (0);
    int prevc=0;
    for (WCharCP p = src; *p; p++)
        {
        int c = *p;
        if (c == '\\')
            c = '/';
        if (c == '/' && prevc == '/')
            continue;
        if (c == '.' && p[1] == '/')
            {
            p++;
            continue;
            }
        // *** WIP_LINUX: remove ..
        path.push_back (c);
        prevc = c;
        }
    
    if (!keepTrailingSeparator && (WCSDIR_SEPARATOR_CHAR == path.end()[-1]))
        path.resize (path.size()-1);

    path.Trim();
    return  BeFileNameStatus::Success;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/01
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetFullPathName (WStringR path, WCharCP src)
    {
#if defined (BENTLEY_WIN32)

    wchar_t     fullName[MAX_PATH];
    DWORD PathSize = ::GetFullPathNameW (src, _countof(fullName), fullName, NULL);

    if (PathSize == 0) // Error
        {
        path = src;   
        return BeFileNameStatus::UnknownError;
        }

    if (PathSize < MAX_PATH) // got a path, and it fit into the buffer.
        {
        path = fullName;
        return  BeFileNameStatus::Success;
        }

    ScopedArray<wchar_t> longFileName (PathSize+1);
    GetFullPathNameW (src, PathSize+1, longFileName.GetData(), NULL);
    path = longFileName.GetData();
    return  BeFileNameStatus::Success;
         
#elif defined (BENTLEY_WINRT)

    // WIP_WINRT
    path = src;
    return BeFileNameStatus::Success;

#elif defined (__unix__)

    char        fullName[PATH_MAX];
    if (realpath (toUtf8(src).c_str(), fullName) != NULL)
        path = fromUtf8(fullName);
    else
        path = src;

    return  BeFileNameStatus::Success;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/01
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetFullPathName ()
    {
    WString fn;
    BeFileNameStatus status = BeGetFullPathName (fn, this->c_str());
    if (BeFileNameStatus::Success != status)
        return status;

    this->assign(fn.c_str());
    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::CreateNewDirectory (WCharCP inPath)
    {
    WString path;
    BeFileNameStatus fileNameStatus = FixPathName (path, inPath, false);
    if (BeFileNameStatus::Success != fileNameStatus)
        return  fileNameStatus;

    if (DoesPathExist(path.c_str()))
        return  BeFileNameStatus::AlreadyExists;

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    // skip drive letter and its trailing slash
    WCharCP root = path.c_str ();

    bool isUnc = ('\\' == path[0]) && ('\\' == path[1]);
    if ( !isUnc )
        while (':' != *root && 0 != *root)
            root++;
    root += 2;
    BeAssert (root < path.c_str () + path.size ());

    WString fullpath;
    size_t start = root ? root-path.c_str() : 0;
    int skipDirCheckCount = isUnc ? 2 : 0;  // skip the IsDirectory and CreateDirectoryW calls for the UNC host and sharename.
    int nameSectionCount = 0;
    while (fullpath.size() < path.size())
        {
        size_t end = path.find(L"\\", start);
        fullpath.assign (path.substr (0, end));
        start = end+1;

        ++nameSectionCount;
        if (skipDirCheckCount > 0)
            {
            --skipDirCheckCount;
            continue;
            }

        if (!IsDirectory(fullpath.c_str()) && (0 == ::CreateDirectoryW (fullpath.c_str(), NULL)))
            {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
                return  BeFileNameStatus::CantCreate;
            }
        }

    // If it is a minimal UNC path (consisting only of hostname and sharename) then it cannot be brought into being with a CreateDirectoryW() call;
    // All that we can do is ensure that it exists. If not then return BE_FILENAME_ERROR_CantCreate.
    if (isUnc)
        if (2 == nameSectionCount)
            return IsDirectory(fullpath.c_str()) ? BeFileNameStatus::Success : BeFileNameStatus::CantCreate;
        else if (2 > nameSectionCount)
            return BeFileNameStatus::CantCreate;

#elif defined (__unix__)

    Utf8String pathUtf8;
    BeStringUtilities::WCharToUtf8 (pathUtf8, inPath);

    // ANDROID WIP: need a better permissions strategy !!!
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;

    size_t start = 0;
    if (pathUtf8[0] == '/') // skip over root
        ++start;

    do
        {
        size_t end = pathUtf8.find ("/", start);
        Utf8String partialPath = pathUtf8.substr (0, end);
        
        if (Utf8String::npos == end)
            start = pathUtf8.size ();
        else
            start = end + 1;

        if (0 != mkdir (partialPath.c_str (), mode))
            {
            if (EEXIST != errno)
                {
                int  errval = errno;
                char errstr[128];
                strncpy (errstr, strerror(errval), sizeof(errstr));
                errstr[sizeof(errstr)-1] = 0;
                NativeLogging::LoggingManager::GetLogger(L"DgnPlatform")->warningv (L"BeFileName::CreateNewDirectory (%s) - %d %s", partialPath.c_str(), errval, errstr);
                return BeFileNameStatus::CantCreate;
                }
            }
        }
    while (start < pathUtf8.size());

#else
#error unknown runtime
#endif

    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeDeleteFile () const
    {
    return BeFileName::BeDeleteFile (this->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeDeleteFile (WCharCP fileNameP)
    {
    if (!DoesPathExist (fileNameP))
        return BeFileNameStatus::FileNotFound;

#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileNameP, true);

    WIN32_FILE_ATTRIBUTE_DATA wasAttributes;
    memset (&wasAttributes, 0, sizeof(wasAttributes));
    ::GetFileAttributesExW (fileNameFixed.c_str(), GetFileExInfoStandard, &wasAttributes);
    ::SetFileAttributesW (fileNameFixed.c_str(), FILE_ATTRIBUTE_NORMAL);   // remove read-only attribute (and anything else that might be in the way)
    
    if (!::DeleteFileW(fileNameFixed.c_str()))
        {
        ::SetFileAttributesW (fileNameFixed.c_str(), wasAttributes.dwFileAttributes);   // restore original attributes

        return BeFileNameStatus::CantDeleteFile;
        }
    return BeFileNameStatus::Success;

#elif defined (__unix__)
    
    return remove (toUtf8(fileNameP).c_str()) == 0? BeFileNameStatus::Success: BeFileNameStatus::CantDeleteFile;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::EmptyDirectory(WCharCP inPath)
    {
    WString path2;
    BeFileNameStatus stat = FixPathName (path2, inPath, false);
    if (BeFileNameStatus::Success != stat)
        return  stat;
    BeFileName path (path2.c_str());

    if (!DoesPathExist(path))
        return  BeFileNameStatus::FileNotFound;

    BeFileName filename;
    bool       isDir;
    for (BeDirectoryIterator dir (path); dir.GetCurrentEntry(filename,isDir,true) == SUCCESS; dir.ToNext())
        {
        if (isDir)
            {
            stat = EmptyAndRemoveDirectory (filename);
            if (BeFileNameStatus::Success != stat)
                return  stat;
            }
        else
            {
            SetFileReadOnly (filename, false);
            stat = BeDeleteFile (filename);
            if (stat != BeFileNameStatus::Success)
                return  stat;
            }
        }

    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::EmptyAndRemoveDirectory(WCharCP inPath)
    {
    BeFileNameStatus status = EmptyDirectory(inPath);
    if (BeFileNameStatus::Success != status)
        return status;

    WString path2;
    FixPathName (path2, inPath, false);
    BeFileName path (path2.c_str());

#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)

    return (0 == ::RemoveDirectoryW(path)) ? BeFileNameStatus::CantDeleteDir : BeFileNameStatus::Success;

#elif defined (__unix__)

    return (-1 == rmdir (toUtf8(path).c_str())) ? BeFileNameStatus::CantDeleteDir : BeFileNameStatus::Success;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::CloneDirectory (WCharCP sourceDirIn, WCharCP destDirIn, bool includeSubDirs)
    {
    WString sourceDir2;
    BeFileNameStatus stat = FixPathName (sourceDir2, sourceDirIn, false);
    if (BeFileNameStatus::Success != stat)
        return  stat;

    BeFileName sourceDir (sourceDir2.c_str());

    if (!IsDirectory(sourceDir))
        return  BeFileNameStatus::FileNotFound;

    WString destDir2;
    stat = FixPathName (destDir2, destDirIn, false);
    if (BeFileNameStatus::Success != stat)
        return  stat;
    
    BeFileName destDir (destDir2.c_str());

    stat = CreateNewDirectory (destDir);
    if (BeFileNameStatus::Success != stat)
        return  stat;
    
    BeFileName filename;
    bool       isDir;
    for (BeDirectoryIterator dir (sourceDir); dir.GetCurrentEntry(filename,isDir,true) == SUCCESS; dir.ToNext())
        {
        BeFileName basename (BeFileName::NameAndExt, filename);
        BeFileName destname (NULL, destDir, basename, NULL);
        if (isDir)
            {
            if (!includeSubDirs)
                continue;

            stat = CloneDirectory (filename, destname, true);
            }
        else
            stat = BeCopyFile (filename, destname);

        if (BeFileNameStatus::Success != stat)
            return  stat;
        }

    return  BeFileNameStatus::Success;
    }


#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/12
+---------------+---------------+---------------+---------------+---------------+------*/
    static bool CallWindowsCopyFile (WCharCP sourceFile, WCharCP destinationFile, bool failIfFileExists)
        {
    #if defined (BENTLEY_WIN32)

        return 0 != CopyFileW (sourceFile, destinationFile, failIfFileExists);

    #else

        COPYFILE2_EXTENDED_PARAMETERS xparms;
        memset (&xparms, 0, sizeof(xparms));
        xparms.dwSize = sizeof (xparms);
        if (failIfFileExists)
            xparms.dwCopyFlags |= COPY_FILE_FAIL_IF_EXISTS;

        return S_OK == CopyFile2 (sourceFile, destinationFile, &xparms);

    #endif
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeCopyFile (BeFileNameCR sourceFile, BeFileNameCR destinationFile, bool failIfFileExists)
    {
    return BeFileName::BeCopyFile (sourceFile.GetName(), destinationFile.GetName(), failIfFileExists);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeCopyFile (WCharCP sourceFile, WCharCP destinationFile, bool failIfFileExists)
    {
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fixedSourceFile;
    FixPathName (fixedSourceFile, sourceFile, true);

    WString fixedDestinationFile;
    FixPathName (fixedDestinationFile, destinationFile, true);

    if (0 == CallWindowsCopyFile (fixedSourceFile.c_str(), fixedDestinationFile.c_str(), failIfFileExists))
        {
        int errorNum = ::GetLastError();

        if (ERROR_ACCESS_DENIED == errorNum)
            return BeFileNameStatus::AccessViolation;

        if (ERROR_FILE_NOT_FOUND == errorNum)
            return BeFileNameStatus::FileNotFound;

        return BeFileNameStatus::UnknownError;
        }

    // CopyFile retains the read-only attribute if the source is read-only. We need to clear it.
    SetFileReadOnly (destinationFile, false);

    return BeFileNameStatus::Success;

#elif defined (__unix__)

    #define COPY_BUFFER_SIZE 16384
    char    *buffer;
    int     bytesRead, bytesWritten;
    FILE    *sFile, *dFile;

    if (!(sFile = fopen (toUtf8(sourceFile).c_str(), "rb"))) 
        return BeFileNameStatus::AccessViolation;
    if (!(dFile = fopen (toUtf8(destinationFile).c_str(), "w+b"))) 
        {
        fclose (sFile);
        return BeFileNameStatus::AccessViolation;
        }

    fseek (sFile, 0L, SEEK_SET);
    fseek (dFile, 0L, SEEK_SET);

    buffer = (char *) malloc (COPY_BUFFER_SIZE);
    bytesRead = bytesWritten = 0;
    do
        {
        bytesRead = fread (buffer, 1, COPY_BUFFER_SIZE, sFile);
        if (bytesRead > 0)
            bytesWritten = fwrite (buffer, bytesRead, 1, dFile);
        }
    while ((bytesRead > 0) && (bytesWritten > 0));

    free (buffer);
    fclose (sFile);
    fclose (dFile);

    return BeFileNameStatus::Success;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeMoveFile (BeFileNameCR source, BeFileNameCR target, int numRetries)
    {
    return BeFileName::BeMoveFile (source.GetName(), target.GetName(), numRetries);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeMoveFile (WCharCP source, WCharCP target, int numRetries)
    {
    for (;;)
        {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

        // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
        WString fixedSource;
        FixPathName (fixedSource, source, true);

        WString fixedTarget;
        FixPathName (fixedTarget, target, true);

        if (0 != ::MoveFileExW (fixedSource.c_str(), fixedTarget.c_str(), MOVEFILE_COPY_ALLOWED/*Needed when changing drive*/))
            return  BeFileNameStatus::Success;

#elif defined (__unix__)

        if (0 == rename (toUtf8(source).c_str(), toUtf8(target).c_str()))
            return  BeFileNameStatus::Success;

#else
#error unknown runtime
#endif

        if (numRetries-- == 0)
            return BeFileNameStatus::UnknownError;

        BeThreadUtilities::BeSleep (1000); // sleep for a second
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsFileReadOnly() const
    {
    return IsFileReadOnly (GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsFileReadOnly (WCharCP fileName)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileName, true);

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (!GetFileAttributesExW (fileNameFixed.c_str(), GetFileExInfoStandard, &attributes))
        return false;

    return (attributes.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
 
#elif defined (__unix__)

    struct stat info;
    if (0 != stat (Utf8String(fileName).c_str (), &info))
        return false;

    return (info.st_mode & S_IWRITE) == 0;

#else
#error unknown runtime
#endif
  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::SetFileReadOnly (bool readOnly) const
    {
    return BeFileName::SetFileReadOnly (this->GetName(), readOnly);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::SetFileReadOnly (WCharCP fileName, bool readOnly)
    {
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileName, true);

    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (!GetFileAttributesExW (fileNameFixed.c_str(), GetFileExInfoStandard, &attributes))
        return BeFileNameStatus::UnknownError;

    if (readOnly)
        attributes.dwFileAttributes |=  FILE_ATTRIBUTE_READONLY;
    else
        attributes.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;

    SetFileAttributesW (fileNameFixed.c_str(), attributes.dwFileAttributes);
    return BeFileNameStatus::Success;

#elif defined (__unix__)
    
    struct stat statbuf;
    Utf8String ufilename = toUtf8 (fileName);
    if (stat (ufilename.c_str(), &statbuf) == -1)
        return BeFileNameStatus::FileNotFound;

    mode_t mode = statbuf.st_mode;

    if (readOnly)
        mode &= ~S_IWRITE;
    else
        mode |=  S_IWRITE;

    chmod (ufilename.c_str(), mode);
    return BeFileNameStatus::Success;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::GetFileSize (uint64_t& sz) const
    {
    return BeFileName::GetFileSize (sz, this->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::GetFileSize (uint64_t& sz, WCharCP fileName)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    WString fileNameFixed;

    // resolve the target if it's a link.
    if (IsSymbolicLink (fileName))
        {
        BeFileName resolvedPath;
        if (SUCCESS != GetTargetOfSymbolicLink (resolvedPath, fileName))
            return BeFileNameStatus::UnknownError;

        fileNameFixed.assign (resolvedPath.GetName());
        }
    else
        {
        // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
        FixPathName (fileNameFixed, fileName, true);
        }
    
    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (GetFileAttributesExW (fileNameFixed.c_str(), GetFileExInfoStandard, &fileAttributeData))
        {
        sz = ((uint64_t) fileAttributeData.nFileSizeHigh << 32) | fileAttributeData.nFileSizeLow;
        return BeFileNameStatus::Success;
        }

    return BeFileNameStatus::FileNotFound;

#else

    T_Stat64 status;
    if (beStat (status, fileName) != SUCCESS)
        return BeFileNameStatus::FileNotFound;

    sz = status.st_size;
    return BeFileNameStatus::Success;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetDiskFreeSpace (uint64_t& freeBytes, BeFileNameCR dirName)
    {
    return BeFileName::BeGetDiskFreeSpace (freeBytes, dirName.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* see whether there's sufficient free disk space on the temp device. If not, set flag on this object that will
* be checked later.
* @bsimethod                                                    Keith.Bentley   03/01
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetDiskFreeSpace (uint64_t& freeBytes, WCharCP dirName)
    {
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString dirNameFixed;
    FixPathName (dirNameFixed, dirName, false);

    if (0 == ::GetDiskFreeSpaceExW (dirNameFixed.c_str(), (PULARGE_INTEGER) &freeBytes, NULL, NULL))
        return BeFileNameStatus::UnknownError;     // couldn't determine disk space.

#elif defined (__unix__)

    struct statfs   statusblk;
    int status = statfs (toUtf8(dirName).c_str(), &statusblk);
    if (status != 0)
        return BeFileNameStatus::UnknownError;
    freeBytes = statusblk.f_bavail * statusblk.f_bsize;

#else
#error unknown runtime
#endif

    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::GetFileTime (time_t* ctime, time_t* atime, time_t* mtime) const
    {
    return BeFileName::GetFileTime (ctime, atime, mtime, this->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::GetFileTime (time_t* ctime, time_t* atime, time_t* mtime, WCharCP fileName)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileName, true);

    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (GetFileAttributesExW (fileNameFixed.c_str(), GetFileExInfoStandard, &fileAttributeData))
        {
        if (ctime)
            *ctime = (uint64_t) (BeTimeUtilities::ConvertFiletimeToUnixMillisDouble (fileAttributeData.ftCreationTime) / 1000.0);

        if (atime)
            *atime = (uint64_t) (BeTimeUtilities::ConvertFiletimeToUnixMillisDouble (fileAttributeData.ftLastAccessTime) / 1000.0);

        if (mtime)
            *mtime = (uint64_t) (BeTimeUtilities::ConvertFiletimeToUnixMillisDouble (fileAttributeData.ftLastWriteTime) / 1000.0);

        return BeFileNameStatus::Success;
        }

    return BeFileNameStatus::FileNotFound;

#else

    T_Stat64 status;
    if (SUCCESS != beStat(status, fileName))
        return BeFileNameStatus::FileNotFound;

    if (ctime)
        *ctime = status.st_ctime;
    
    if (atime)
        *atime = status.st_atime;
    
    if (mtime)
        *mtime = status.st_mtime;

    return BeFileNameStatus::Success;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::SetFileTime (time_t const* atime, time_t const* mtime) const
    {
    return BeFileName::SetFileTime (this->GetName(), atime, mtime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::SetFileTime (WCharCP fileName, time_t const* atime, time_t const* mtime)
    {
#if defined (BENTLEY_WIN32)

    FILETIME* pAccessTime=NULL;
    FILETIME* pModifTime=NULL;

    FILETIME AccessTime;
    if (NULL != atime)
        {
        BeTimeUtilities::ConvertUnixTimeToFiletime(AccessTime, *atime);
        pAccessTime = &AccessTime;
        }

    FILETIME ModifTime;
    if (NULL != mtime)
        {
        BeTimeUtilities::ConvertUnixTimeToFiletime(ModifTime, *mtime);
        pModifTime = &ModifTime;
        }

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileName, true);

    int fh;
    if (_wsopen_s(&fh, fileNameFixed.c_str(), _O_RDWR | _O_BINARY, _SH_DENYNO, 0) != 0)
        return(BeFileNameStatus::FileNotFound);
    ::SetFileTime((HANDLE)_get_osfhandle(fh), NULL, pAccessTime, pModifTime);
    _close(fh);

#elif defined(BENTLEY_WINRT)

    time_t atimeOriginal, mtimeOriginal;

    BeFileNameStatus fileNameStatus = GetFileTime (NULL, &atimeOriginal, &mtimeOriginal, fileName);
    if (BeFileNameStatus::Success != fileNameStatus)
        return fileNameStatus;

    struct __utimbuf64 ut;

    if (NULL != atime)
        ut.actime = *atime;
    else
        ut.actime = atimeOriginal;

    if (NULL != mtime)
        ut.modtime = *mtime;
    else
        ut.modtime = mtimeOriginal;

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fileName, true);

    _wutime64 (fileNameFixed.c_str(), &ut);
    
#elif defined (__unix__)

    T_Stat64 status;
    if (SUCCESS != beStat(status, fileName))
        return BeFileNameStatus::FileNotFound;

    struct utimbuf ut;

    if (NULL != atime)
        ut.actime = *atime;
    else
        ut.actime = status.st_atime;

    if (NULL != mtime)
        ut.modtime = *mtime;
    else
        ut.modtime = status.st_mtime;

    utime(toUtf8(fileName).c_str(), &ut);

#else
    
    #error unknown runtime

#endif

    return BeFileNameStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::CheckAccess (BeFileNameAccess req) const
    {
    return BeFileName::CheckAccess (this->GetName(), req);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::CheckAccess (WCharCP fn, BeFileNameAccess req)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString fileNameFixed;
    FixPathName (fileNameFixed, fn, false);

    int i = _waccess (fileNameFixed.c_str(), (int)req);

#elif defined (__unix__)

    int i = access (toUtf8(fn).c_str(), (int)req);

#else
#error unknown runtime
#endif
    return (0 == i)? BeFileNameStatus::Success: (-1 == i)? BeFileNameStatus::FileNotFound: BeFileNameStatus::AccessViolation;
    }

/*----------------------------------------------------------------------------------*//**
* Remove quotes around a string.  Often used for file names with spaces around them. (Unicode)
+---------------+---------------+---------------+---------------+---------------+------*/
void    BeFileName::RemoveQuotes ()
    {
    /* check for quoted file names */
    if (this->at(0) == L'\"')
        {
        assign (this->substr (1, this->size()));
        if (!this->empty() && this->at (this->size()-1) == L'\"')
            this->resize (this->size() - 1);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2013
//---------------------------------------------------------------------------------------
WString BeFileName::Abbreviate(size_t maxLength) const
    {
    // Do we already fit?
    if (size() <= maxLength)
        return *this;

    static const WChar LEADING_INDICATOR[] = L"...";
    static size_t LEADING_INDICATOR_COUNT = _countof(LEADING_INDICATOR);

    // Need at least LEADING_INDICATOR_COUNT because we'll prepend LEADING_INDICATOR to whatever else we can fit.
    // This is documented as undefined behavior; arbitrarily deciding to return the full string.
    if (maxLength <= LEADING_INDICATOR_COUNT)
        return *this;
    
    WCharCP start = c_str();
    bool isOnLastComponent = false;

    // Pop leading path components until we fit (remember the leading "..." we'll prepend).
    while (wcslen(start) > (maxLength - LEADING_INDICATOR_COUNT))
        {
        WCharCP newStart = wcschr(start + 1, WCSDIR_SEPARATOR_CHAR);
        if (NULL == newStart)
            {
            isOnLastComponent = true;
            break;
            }
        
        start = newStart;
        }

    if (!isOnLastComponent)
        return WString(LEADING_INDICATOR) + start;

    return WString(LEADING_INDICATOR) + substr(size() - (maxLength - LEADING_INDICATOR_COUNT));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool BeFileName::IsSymbolicLink() const
    {
    return BeFileName::IsSymbolicLink (this->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
bool BeFileName::IsSymbolicLink(WCharCP path)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    
    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString pathFixed;
    FixPathName (pathFixed, path, false);
    
    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
    if (GetFileAttributesExW (pathFixed.c_str(), GetFileExInfoStandard, &fileAttributeData))
        return (0 != (fileAttributeData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT));

    return false;

#elif defined (__unix__)

    struct stat attrs;
    if (0 != lstat(Utf8String(path).c_str(), &attrs))
        return false;
    
    return S_ISLNK (attrs.st_mode);

#else
#error Unimplemented on this platform.
#endif
    }

#if defined (BENTLEY_WIN32)

//=======================================================================================
// From ntifs.h in the WDK: http://msdn.microsoft.com/en-us/library/ms791514.aspx
// You should check for the Microsoft bit in the reparse tag before using this; IO_REPARSE_TAG_SYMLINK inherently also checks that.
// @bsiclass                                                    Jeff.Marker     07/2013
//=======================================================================================
typedef struct _REPARSE_DATA_BUFFER
{
    ULONG ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    
    union
    {
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct
        {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct
        {
            UCHAR DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#endif
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
static BentleyStatus getImmediateTargetOfSymLink(BeFileNameR target, WCharCP path)
    {
#if defined (BENTLEY_WIN32)

    // FixPathName adds the "extended path prefix" if name length >= MAX_PATH
    WString pathFixed;
    if (BeFileNameStatus::Success != BeFileName::FixPathName (pathFixed, path, true))
        return ERROR;

    HANDLE hFile = ::CreateFileW(pathFixed.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS), 0);
    if (INVALID_HANDLE_VALUE == hFile)
        return ERROR;

    REPARSE_DATA_BUFFER*    reparsePointData    = (REPARSE_DATA_BUFFER*)_alloca(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    DWORD                   bytesReturned       = 0;
    bool                    succeeded           = (0 != ::DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, reparsePointData, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, NULL));
    
    if (0 == ::CloseHandle(hFile))
        { BeAssert(false); }

    if (!succeeded || (IO_REPARSE_TAG_SYMLINK != reparsePointData->ReparseTag))
        return ERROR;
    
    target.assign(reparsePointData->SymbolicLinkReparseBuffer.PathBuffer + (reparsePointData->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof (WChar)), (reparsePointData->SymbolicLinkReparseBuffer.PrintNameLength / sizeof (WChar)));

    return SUCCESS;

#elif defined (BENTLEY_WINRT)
    
    // Not expecting reprase point / symbolic link support within WinRT.
    // I believe GetFileAttributesEx could be used within WinRT to detect them, but I haven't found a way to get link targets yet.
    return ERROR;

#elif defined (__unix__)

    Utf8String pathUtf8(path);
    struct stat attrs;
    
    if (0 != lstat(pathUtf8.c_str(), &attrs))
        return ERROR;
    
    size_t linkTargetLen = attrs.st_size;
    Utf8String targetPathUtf8;
    targetPathUtf8.resize(linkTargetLen);
    size_t bytesRead = readlink(pathUtf8.c_str(), const_cast<Utf8P>(&targetPathUtf8[0]), (linkTargetLen + 1));
    
    if (-1 == bytesRead)
        return ERROR;

    if (bytesRead != (linkTargetLen + 1))
        { BeAssert(false); }

    return SUCCESS;

#else
#error Unimplemented on this platform.
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
BentleyStatus BeFileName::GetTargetOfSymbolicLink(BeFileNameR target, WCharCP path)
    {
    return GetTargetOfSymbolicLink(target, path, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
BentleyStatus BeFileName::GetTargetOfSymbolicLink(BeFileNameR target, WCharCP path, bool shouldRecurse)
    {
    if (SUCCESS != getImmediateTargetOfSymLink(target, path))
        return ERROR;
    
    while (shouldRecurse && (SUCCESS == getImmediateTargetOfSymLink(target, target.GetName())))
        ;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Generates a temporary filename
* @bsimethod                                                    BernMcCarty     06/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetTempFileName (BeFileName& tempFileName, BeFileName const& pathToUseIn, WCharCP prefixString)
    {
#if defined (BENTLEY_WIN32) || (defined(BENTLEY_WINRT) && _MSC_VER >= 1900)
    BeFileName pathToUse (pathToUseIn);
    if (!*pathToUse.GetName())
        BeGetTempPath (pathToUse);

    WChar      tempName[4096];

    wcscpy (tempName, tempFileName);

    if (0 == ::GetTempFileNameW (pathToUse, prefixString, 0, tempName))
        return BeFileNameStatus::CantCreate;

    tempFileName.SetName (tempName);
    return BeFileNameStatus::Success;
#else
    return BeFileNameStatus::UnknownError;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus BeFileName::BeGetTempPath (BeFileName& tempPath)
    {
#if defined (BENTLEY_WIN32) || (defined(BENTLEY_WINRT) && _MSC_VER >= 1900)
    WChar       tempName[MAX_PATH * 5];

    if (0 == ::GetTempPathW (_countof(tempName), tempName))
        return BeFileNameStatus::CantCreate;

    tempPath = BeFileName (tempName);
    return BeFileNameStatus::Success;
#else
    return BeFileNameStatus::UnknownError;
#endif
    }

BeFileNameStatus BeFileName::GetCwd (WStringR currentDirectory)
    {
#if defined (BENTLEY_WIN32) || (defined(BENTLEY_WINRT) && _MSC_VER >= 1900)
    wchar_t cwdPath[MAX_PATH];
    _wgetcwd (cwdPath, _countof(cwdPath));
    currentDirectory.assign (cwdPath);
    return BeFileNameStatus::Success;
#elif defined(BENTLEY_WINRT) && _MSC_VER < 1900
    return BeFileNameStatus::UnknownError;
#else
    char cwdPath[MAX_PATH];
    getcwd (cwdPath, sizeof(cwdPath));
    currentDirectory.AssignA (cwdPath);
    return BeFileNameStatus::Success;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
BeFileName BeFileName::Combine(std::initializer_list<WCharCP> paths) const
    {
    BeFileName fullPath = *this;

    for (WCharCP path : paths)
        fullPath.AppendToPath(path);

    return fullPath;
    }
