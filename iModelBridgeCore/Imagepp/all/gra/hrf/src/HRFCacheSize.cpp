//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheSize.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFCacheSize
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>    //:> must be first for PreCompiledHeader Option
#include <Imagepp/all/h/HRFCacheSize.h>
#include <Imagepp/all/h/HFCURLFile.h>



/**-----------------------------------------------------------------------------
 Public constructor of the class.
------------------------------------------------------------------------------*/
HRFCacheSize::HRFCacheSize(const HFCURL&  pi_rCachePath,
                           size_t         pi_CacheSize,
                           bool          pi_ScanSubDir)
    : m_DirCachePath(pi_rCachePath.GetURL()),
      m_DirCacheSizeMax(pi_CacheSize),
      m_ScanSubDir(pi_ScanSubDir)

    {
    HPRECONDITION(pi_rCachePath.IsCompatibleWith(HFCURLFile::CLASS_ID));
    HPRECONDITION(pi_CacheSize > 0);
    }


/**-----------------------------------------------------------------------------
 Public destructor of the class.
------------------------------------------------------------------------------*/
HRFCacheSize::~HRFCacheSize()
    {
    }

/**-----------------------------------------------------------------------------
 This methode remove scan the cache directory and remove the older cache if
 the size of the directory was greater than the max size.
------------------------------------------------------------------------------*/
void HRFCacheSize::ControlDirCacheSize()
    {
    m_FileList.clear();
    WString Path(((const HFCURLFile&)m_DirCachePath).GetHost() +
                 WString(L"\\") +
                 ((const HFCURLFile&)m_DirCachePath).GetPath());
    GetFileInfo(Path,
                &m_FileList);

    m_FileList.sort();

    // calcul the size used by the cache
    size_t CacheSize = 0;
    FileEntryList::iterator Itr(m_FileList.begin());
    while (Itr != m_FileList.end())
        {
        CacheSize += Itr->m_FileSize;
        Itr++;
        }

    if (CacheSize >= m_DirCacheSizeMax)
        {
        // delete older cache
        FileEntryList::iterator TmpItr;
        Itr = m_FileList.begin();
        while (CacheSize > m_DirCacheSizeMax && Itr != m_FileList.end())
            {
            if (_wremove(Itr->m_FileName.c_str()) == 0)
                {
                CacheSize -= Itr->m_FileSize;
                TmpItr = Itr;
                Itr++;

                m_FileList.erase(TmpItr);
                }
            else
                Itr++;
            }
        }
    }


//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

/**-----------------------------------------------------------------------------
 This method extract the file information of each compliant file into the cache
 directory.

 @param pi_rPath The path of the directory to scan
 @param pi_pList The list must be clear for the first call, this method add
                 all entries at the end of the list
------------------------------------------------------------------------------*/
void HRFCacheSize::GetFileInfo(const WString& pi_rPath, FileEntryList* pi_pFileList)
    {
    HPRECONDITION(pi_pFileList != 0);

    WString CurDir(pi_rPath + L"\\");
    WString FindPath(CurDir + L"*.*");
    struct _wfinddata_t FileInfo;

    intptr_t hSearch = _wfindfirst(FindPath.c_str(), &FileInfo);

    if (hSearch != -1)
        {
        WString     Name;
        FileEntry   EntryInfo;
        // Insert all the file into the list
        do
            {
            // skip "." and ".."
            if ((wcscmp(FileInfo.name, L".") != 0) &&
                (wcscmp(FileInfo.name, L"..") != 0))
                {
                Name = CurDir;
                Name += FileInfo.name;

                if ((FileInfo.attrib & _A_SUBDIR) != _A_SUBDIR)
                    {
                    if (IsCompliant(Name))
                        {
                        EntryInfo.m_FileName = Name;
                        if (FileInfo.time_access != -1L)
                            EntryInfo.m_LastAccess = FileInfo.time_access;
                        else
                            EntryInfo.m_LastAccess = FileInfo.time_write;

                        EntryInfo.m_FileSize = FileInfo.size;
                        m_FileList.push_back(EntryInfo);
                        }
                    }
                else if (m_ScanSubDir)
                    {
                    GetFileInfo(Name, pi_pFileList);
                    }
                }
            }
        while (_wfindnext(hSearch, &FileInfo) == 0);

        _findclose(hSearch);
        }
    }


/**-----------------------------------------------------------------------------
 This method verify if the file name was compliant with the a cache file name.

 @param  pi_rFileName The name of the file

 @return bool true if the file name has the string ".cache."
------------------------------------------------------------------------------*/
bool HRFCacheSize::IsCompliant(const WString& pi_rFileName) const
    {
    HPRECONDITION(!pi_rFileName.empty());

    return CaseInsensitiveStringTools().Find(pi_rFileName, L".cache.") != WString::npos;
    }
