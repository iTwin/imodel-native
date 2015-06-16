/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeDirectoryIterator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)
    // caller should include windows.h
    #if defined (BENTLEY_WINRT)
        #include <handleapi.h>
        #include <fileapi.h>
        #include <regex>
    #else
        #include <Shlwapi.h>
    #endif
#elif defined (__unix__)
    #include <dirent.h>
    #include <fnmatch.h>
    #include <sys/stat.h>
#else
    #error unknown platform
#endif

#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeDebugLog.h>

#if defined (BENTLEY_WINRT)
static std::wregex makeRegexFromGlob (WCharCP glob)
    {
    WString re;
    for (WCharCP g = glob; *g; ++g)
        {
        switch (*g)
            {
            case '\\':
                re.append (L"\\");
                break;
            case '/':
                re.append (L"\\/");
                break;
            case '.':
                re.append (L"\\.");
                break;
            case '*':
                re.append (L".*");
                break;
            case '?':
                re.append (L".");
                break;
            default:
                re.append (1,*g);
            }
        }

    re.ToLower ();

    return std::wregex (re.c_str());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static WString fromUtf8 (char const* u)
    {
    WString w;
    BeStringUtilities::Utf8ToWChar (w, u);
    return w;
    }
#endif //defined (__unix__)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::FileNamePattern::Parse (BeFileName& dir, WString& glob, BeFileNameCR pathAndPattern)
    {
    WCharCP endPath = wcsrchr (pathAndPattern, DIR_SEPARATOR_CHAR);

    if (NULL == endPath)
        endPath = wcschr (pathAndPattern, '\\');    // support both forward and back slash

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    if (NULL == endPath)
        endPath = wcschr (pathAndPattern, ':');
#endif

    if (endPath)                              // pathAndPattern includes a path
        {
        dir.SetName (WString (pathAndPattern, endPath-pathAndPattern));
        glob = endPath+1;
        }
    else
        {                                      // pathAndPattern is not a path
        if (wcscspn (pathAndPattern, L"?*") < wcslen(pathAndPattern))    // if pathAndPattern contains a wildcard
            {                                  // then it is a wildcard expression in the current directory
            dir.SetName (L".");
            glob = pathAndPattern.GetName();
            } 
        else                                    // else if pat does not contain a wildcard
            {                                   // then pat is the name of a directory
            dir = pathAndPattern;
            glob = L"*";
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentleyApi::FileNamePattern::MatchesGlob (BeFileNameCR name, WCharCP glob)
    {
#if defined (BENTLEY_WINRT)
    WString lname (name);
    lname.ToLower ();
    std::wregex re = makeRegexFromGlob (glob);
    return std::regex_match (lname.begin(),lname.end(),re);
#elif defined (BENTLEY_WIN32)
    return PathMatchSpecW (name, glob) != 0;
#elif defined (__unix__)
    return fnmatch (toUtf8(glob).c_str(), toUtf8(name).c_str(), FNM_CASEFOLD) == 0;
#endif
    }

BEGIN_BENTLEY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DirectoryWalkerImpl
{
#if defined (BENTLEY_WIN32)||defined (BENTLEY_WINRT)

WIN32_FIND_DATAW    m_data;
HANDLE              m_findHandle;
bool                m_finished;
BeFileName          m_topDir;

bool IsValidFile () const           {return 0!=wcscmp(L".",m_data.cFileName) && 0!=wcscmp(L"..",m_data.cFileName);}

DirectoryWalkerImpl (BeFileNameCR dir)
    :
    m_topDir (dir)
    {
    WString dirPat (dir.GetName());
    BeFileName::AppendSeparator (dirPat);
    dirPat.append (L"*");

    m_findHandle = ::FindFirstFileExW (dirPat.c_str(), FindExInfoStandard, &m_data, FindExSearchNameMatch, NULL, 0);
    m_finished = (m_findHandle == INVALID_HANDLE_VALUE);
    if (!m_finished && !IsValidFile())
        ToNext();
    }

~DirectoryWalkerImpl() 
    {
    if (INVALID_HANDLE_VALUE != m_findHandle) 
        ::FindClose (m_findHandle);
    }

/*StatusInt*/int GetCurrentEntry (BeFileNameR name, bool& isDir, bool fullPath)
    {
    if (m_finished)
        return ERROR;
    isDir = 0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    name.BuildName (NULL, fullPath? m_topDir.GetName(): NULL, m_data.cFileName, NULL);
    return SUCCESS;
    }

/*StatusInt*/int ToNext()
    {
    if (m_finished)
        return ERROR;
    do  {
        if (!::FindNextFileW (m_findHandle, &m_data))
            {
            m_finished = true;
            return ERROR;
            }
        }
    while (!IsValidFile());
    return SUCCESS;
    }

#elif defined (__unix__)

DIR*                m_dir;
struct dirent*      m_dp;
BeFileName          m_topDir;
Utf8String          m_topDirPrefix;

bool        IsValidFile () const        {return 0 != strcmp (".", m_dp->d_name) && 0 != strcmp ("..", m_dp->d_name);}

DirectoryWalkerImpl (BeFileNameCR dir)
    :
    m_topDir (dir)
    {
    m_topDirPrefix = toUtf8 (dir);
    if (*m_topDirPrefix.rbegin() != '/')
        m_topDirPrefix.append ("/");

    m_dp = NULL;
    m_dir = opendir (toUtf8(dir).c_str());
    if (NULL == m_dir)
        return;
    m_dp = readdir (m_dir);
    if (NULL != m_dp && !IsValidFile())
        ToNext();
    }

~DirectoryWalkerImpl() 
    {
    if (NULL != m_dir) 
        closedir (m_dir);
    }

/*StatusInt*/int GetCurrentEntry (BeFileNameR name, bool& isDir, bool fullPath)
    {
    if (NULL == m_dp)
        return ERROR;

    name.BuildName (NULL, fullPath? m_topDir.GetName(): NULL, fromUtf8(m_dp->d_name).c_str(), NULL);

    if (m_dp->d_type != DT_LNK)
        isDir = (m_dp->d_type == DT_DIR);
    else
        {
        Utf8String actualpath (m_topDirPrefix);
        actualpath.append (m_dp->d_name);
        struct stat statbuf;
        if (stat (actualpath.c_str(), &statbuf) == -1)  // Note: stat returns info about target of link, not the link itself. It follows links recursively. Use lstat to get info about the link itself.
            {BeAssert(false);}
        isDir = S_ISDIR(statbuf.st_mode);
        }

    return SUCCESS;
    }

/*StatusInt*/int ToNext()
    {
    if (NULL == m_dp)
        return ERROR;
    do  {
        if ((m_dp = readdir (m_dir)) == NULL)
            return ERROR;
        }
    while (!IsValidFile());
    return SUCCESS;
    }

#else
#error unknown runtime 
#endif

}; //DirWalkerImpl

END_BENTLEY_NAMESPACE

USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeDirectoryIterator::BeDirectoryIterator (BeFileNameCR dir) {m_impl = new DirectoryWalkerImpl (dir);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeDirectoryIterator::~BeDirectoryIterator () {delete m_impl;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BeDirectoryIterator::GetCurrentEntry (BeFileName& name, bool& isDir, bool fullPath) {return m_impl->GetCurrentEntry (name, isDir, fullPath);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BeDirectoryIterator::ToNext () {return m_impl->ToNext ();}
