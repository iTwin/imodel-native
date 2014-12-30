/*-------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeFileListIterator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
    #include <objbase.h>
    #if defined (BENTLEY_WINRT)
        #include <handleapi.h>
        #include <fileapi.h>
    #endif
#elif defined (__unix__)
    #include <unistd.h>
    #include <dirent.h>
    #include <fnmatch.h>
    #include <sys/stat.h>
#else
    #error unknown compiler
#endif
#include "../BentleyInternal.h"
#include <Bentley/WString.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeDebugLog.h>

#define DELETE_AND_CLEAR(ptr)   {if(ptr){delete (ptr);ptr=NULL;}}
#define TO_BOOL(x)              (0 != (x))

BEGIN_BENTLEY_NAMESPACE

#if defined (BENTLEY_WIN32)||defined(BENTLEY_WINRT)

struct FileFinder;

//! Results of a pattern match
struct FindData
    {
private:
    friend FileFinder;
    WIN32_FIND_DATAW    m_data;

    bool IsDirectory () const           {return TO_BOOL(m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);}

public:
    void GetName (WStringR nm) const    {nm = m_data.cFileName;}
    bool IsValidFile () const           {return 0!=wcscmp(L".",m_data.cFileName) && 0!=wcscmp(L"..",m_data.cFileName);}
    };

//! Finds all files matching a name pattern
struct FileFinder
    {
private:
    HANDLE              m_findHandle;
    FindData            m_findData;
    WString             m_pattern;
    bool                m_finished;
    
    FindData const*     Finished() {m_finished=true; return NULL;}

public:
    FileFinder (WCharCP pattern) 
        : 
        m_findHandle(INVALID_HANDLE_VALUE), m_finished(false) 
        {
        BeFileName::FixPathName (m_pattern, pattern, true);
        }

    ~FileFinder() 
        {
        if (INVALID_HANDLE_VALUE != m_findHandle) 
            ::FindClose (m_findHandle);
        }

    bool                IsFinished () const {return m_finished;}

    bool                IsDirectory (FindData const* d) {return d? d->IsDirectory (): false;}

    FindData const*     DoFindFirstFile ()
        {
        m_findHandle = ::FindFirstFileExW (m_pattern.c_str(), FindExInfoStandard, &m_findData.m_data, FindExSearchNameMatch, NULL, 0);
        if (INVALID_HANDLE_VALUE == m_findHandle)
            return Finished();
        return &m_findData;
        }

    FindData const*     DoFindNextFile ()
        {
        if (!::FindNextFileW (m_findHandle, &m_findData.m_data))
            return Finished ();
        return &m_findData;
        }

    FindData const*     GetCurrent() const
        {
        return IsFinished()? NULL: &m_findData;
        }
    };

#elif defined (__unix__)

struct FileFinder;

//! Results of a pattern match
struct FindData
    {
private:
    friend struct FileFinder;
    struct dirent* m__dp;

    bool        IsDirectory (CharCP topDirPrefix) const
        {
        if (NULL == m__dp)
            {
            BeAssert (false);
            return false;
            }

        if (m__dp->d_type == DT_DIR)
            return true;

        if (m__dp->d_type != DT_LNK && m__dp->d_type != DT_UNKNOWN)
            return false;
            
        std::string actualpath (topDirPrefix);
        actualpath.append ("/");
        actualpath.append (m__dp->d_name);
#if defined (DO_I_NEED_THIS)
        char resolvedPath[MAX_PATH];
        if (NULL == realpath (actualpath.c_str(), resolvedPath))
            {
            BeAssert(false);
            return false;
            }
#endif
        struct stat statbuf;
        if (stat (actualpath.c_str(), &statbuf) == -1)  // Note: stat returns info about target of link, not the link itself. It follows links recursively. Use lstat to get info about the link itself.
            {
                perror (actualpath.c_str());
                BeAssert(false);
            }
        return S_ISDIR(statbuf.st_mode);
        }

    void Clear() { if (m__dp) {free (m__dp); m__dp = NULL;} }

    char const* GetNameUtf8() const            {if (m__dp) return m__dp->d_name; BeAssert(false); return "";}
public:
    FindData() : m__dp(NULL) {;}
    ~FindData() {Clear();}

    StatusInt DoReadDir (DIR* dir, Utf8CP dirpath)
        {
        Clear ();

        long name_max = pathconf (dirpath, _PC_NAME_MAX);
        if (name_max == -1)         /* Limit not defined, or error */
            name_max = 255;         /* Take a guess */
        size_t len = offsetof(struct dirent, d_name) + name_max + 1;

        m__dp = (struct dirent*) malloc(len);

        struct dirent* result;

        if (0 != readdir_r (dir, m__dp, &result) || (result == NULL))
            return ERROR;
        return SUCCESS;
        }

    void        GetName (WStringR nm) const {nm.AssignUtf8 (GetNameUtf8());}
    bool        IsValidFile () const        {return 0 != strcmp (".", GetNameUtf8()) && 0 != strcmp ("..", GetNameUtf8());}
    };

//! Finds all files matching a name pattern
struct FileFinder
    {
private:
    Utf8String          m_glob;
    Utf8String          m_dirName;
    DIR*                m_dir;
    FindData            m_entry;
    bool                m_finished;

    FindData const*     Finished() {m_finished=true; return NULL;}

    FindData const*     GetNextEntry ()
        {
        if (NULL == m_dir)
            return Finished();
        if (SUCCESS != m_entry.DoReadDir (m_dir, m_dirName.c_str()))
            return Finished();
        return &m_entry;
        }

    FindData const*     GetNextMatchingEntry ()
        {
        FindData const* e;
        while ((e = GetNextEntry()) != NULL)
            {
            if (fnmatch (m_glob.c_str(), e->GetNameUtf8(), FNM_CASEFOLD) == 0)
                return e;
            }
        return Finished();
        }

public:
    FileFinder (WCharCP pat) : m_dir(NULL), m_finished(false) 
        {
        // NB: We must use the same logic as BeFileNameIterator to isolate the directory.
        //      That's because BeFileNameIterator::GetCurrentFileName uses what it thinks is the directory to create the fullpath of the current entry.
        WString wdir, wfn, wfx, fixedPath;
        BeFileName::FixPathName(fixedPath, pat, true);
        BeFileName::ParseName (NULL, &wdir, &wfn, &wfx, fixedPath.c_str());

        BeStringUtilities::WCharToUtf8 (m_dirName, wdir.c_str());
        if (!wfx.empty())
            wfn.append(L".").append (wfx);
        BeStringUtilities::WCharToUtf8 (m_glob, wfn.c_str());
        }

    ~FileFinder() 
        {
        if (NULL != m_dir) 
            closedir (m_dir);
        }

    bool                IsDirectory (FindData const* d) {return d? d->IsDirectory (m_dirName.c_str()): false;}

    bool                IsFinished () const {return m_finished;}

    FindData const*     DoFindFirstFile ()
        {
        m_dir = opendir (m_dirName.c_str());
        if (NULL == m_dir)
            return Finished();
        return GetNextMatchingEntry ();
        }

    FindData const*     DoFindNextFile ()
        {
        if (IsFinished())
            return NULL;
        return GetNextMatchingEntry ();
        }

    FindData const*     GetCurrent() const
        {
        return IsFinished()? NULL: &m_entry;
        }
    };


#else
#error unknown runtime
#endif

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   02/09
+===============+===============+===============+===============+===============+======*/
class BeFileNameIterator
    {
    BeFileNameIterator* m_subIter;
    FileFinder  m_finder;
    bool        m_recursive;
    WString     m_inPath;
    WString     m_deviceName;
    WString     m_dirName;

public:
    BeFileNameIterator (WCharCP inPath, bool recursive)
        :
        m_finder (inPath),
        m_inPath (inPath)
        {
        m_subIter = NULL;
        m_recursive = recursive;

        WString fixedPath;
        BeFileName::FixPathName(fixedPath, inPath, true);
        BeFileName::ParseName (&m_deviceName, &m_dirName, NULL, NULL, fixedPath.c_str());

        //  Set current to the first valid file or directory
        m_finder.DoFindFirstFile ();
        if (m_finder.IsFinished())
            return;

        if (!IsValidFile())
            MoveToNextValidFile();

        if (m_finder.IsFinished())
            return;
        
        if (recursive && IsDirectory())
            StartSubIter (GetCurrentFileName());
        }

    ~BeFileNameIterator ()
        {
        DELETE_AND_CLEAR (m_subIter);
        }

    BeFileName GetCurrentFileName (bool fullPath = true) const
        {
        WString fname;
        m_finder.GetCurrent()->GetName (fname);
    
        BeFileName name;
        if (fullPath)    
            name.BuildName (m_deviceName.c_str(), m_dirName.c_str(), fname.c_str(), NULL);
        else
            name.SetName (fname.c_str());

        return name;
        }

    bool IsDirectory () {return m_finder.IsDirectory(m_finder.GetCurrent());}
    bool IsValidFile () {return m_finder.GetCurrent()->IsValidFile();}
    
    void MoveToNextValidFile() 
        {
        FindData const* d=NULL; 
        do  {
            d = m_finder.DoFindNextFile();
            } 
        while (NULL != d && !IsValidFile());
        }

    void StartSubIter (WCharCP name)
        {
        BeFileName filter (name);
        filter.AppendToPath (L"*");
        m_subIter = new BeFileNameIterator (filter, true);
        }

    StatusInt GetNextFileName (BeFileName& name)
        {
        if (m_subIter)
            {
            if (SUCCESS == m_subIter->GetNextFileName(name))
                return  SUCCESS;

            DELETE_AND_CLEAR(m_subIter);
            }

        if (m_finder.IsFinished ())
            return  ERROR;

        //  Grab the name of the current file or directory
        name = GetCurrentFileName();
        bool isDir = IsDirectory();

        //  Move current to the next valid file or directory
        MoveToNextValidFile();

        if (m_recursive && isDir)
            {
            StartSubIter(name.GetName());
            return  GetNextFileName(name);
            }

        return  SUCCESS;
        }
    };

END_BENTLEY_NAMESPACE
USING_NAMESPACE_BENTLEY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileListIterator::~BeFileListIterator () 
    {
    DELETE_AND_CLEAR (m_finder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeFileListIterator::GetNextFileName (BeFileName& name)
    {
    if (m_finder && (SUCCESS == m_finder->GetNextFileName (name)))
        return  SUCCESS;

    do
        {
        DELETE_AND_CLEAR (m_finder);
        size_t start = m_paths.find_first_not_of (PATH_SEPARATOR_CHAR, m_curr);
              m_curr = m_paths.find (PATH_SEPARATOR_CHAR, start);

        if (m_curr == start)
            {
            m_curr = WString::npos;
            return  ERROR;
            }

        m_finder = new BeFileNameIterator (m_paths.substr (start, m_curr-start).c_str(), m_recursive);
        if (SUCCESS == m_finder->GetNextFileName (name))
            return  SUCCESS;

        } while (m_curr<WString::npos);

    return  ERROR;
    }
