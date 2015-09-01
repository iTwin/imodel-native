//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheController.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class HRFCacheController
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option
#include <Imagepp/all/h/HRFCacheController.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCStat.h>

/**-----------------------------------------------------------------------------
 Public constructor of the class.
------------------------------------------------------------------------------*/
HRFCacheController::HRFCacheController(const HFCPtr<HFCURL> & pi_CachePath,
                                       bool                  pi_ScanSubDir)
    :m_DirCachePath(pi_CachePath),
     m_ScanSubDir(pi_ScanSubDir),
     m_MaxCacheSize(UINT64_MAX),
     m_FilesOlderThan(ULONG_MAX)       // ~68 years
    {
    HPRECONDITION(pi_CachePath->IsCompatibleWith(HFCURLFile::CLASS_ID));
    }


/**-----------------------------------------------------------------------------
 Public destructor of the class.
------------------------------------------------------------------------------*/
HRFCacheController::~HRFCacheController()
    {
    }

/**-----------------------------------------------------------------------------
 Get and set the maximum cache size(in bytes). We delete files from the oldest to
 the youngest using the last access time stamp.
 Use CACHE_CONTROL_SIZE to enable.
------------------------------------------------------------------------------*/
uint64_t HRFCacheController::GetCacheSize() const
    {
    return m_MaxCacheSize;
    }

void HRFCacheController::SetCacheSize(uint64_t pi_CacheSizeBytes)
    {
    m_MaxCacheSize = pi_CacheSizeBytes;
    }

/**-----------------------------------------------------------------------------
 Get and set how much time(in secondes) we should keep a cache file.
 Use CACHE_CONTROL_OLD to enable.
------------------------------------------------------------------------------*/
time_t HRFCacheController::GetDeleteCacheFilesOlderThan() const
    {
    return m_FilesOlderThan;
    }

void   HRFCacheController::SetDeleteCacheFilesOlderThan(time_t pi_OlderThan)
    {
    m_FilesOlderThan = pi_OlderThan;
    }

/**-----------------------------------------------------------------------------
 Get and set the folders containing cache files that need to be moved to the
 current cache folder. Only files that are not locked will be moved.
 N.B. The size and old rules will also be applied to these directories.
 Use CACHE_CONTROL_MOVE to enable.
------------------------------------------------------------------------------*/
const HRFCacheController::MoveFolderList& HRFCacheController::GetMoveFolderList() const
    {
    return m_MoveFolderList;
    }

void HRFCacheController::SetMoveFolderList(const HRFCacheController::MoveFolderList& pi_MoveFolderList)
    {
#ifdef __HMR_DEBUG
    for(MoveFolderList::const_iterator Itr(pi_MoveFolderList.begin()); Itr != pi_MoveFolderList.end(); ++Itr)
        HPRECONDITION((*Itr)->IsCompatibleWith(HFCURLFile::CLASS_ID));
#endif

    m_MoveFolderList = pi_MoveFolderList;
    }

/**-----------------------------------------------------------------------------
This method control the cache directory using the specified flags.
------------------------------------------------------------------------------*/
void HRFCacheController::Control(int pi_CacheControlFlags)
    {

    if(0 == pi_CacheControlFlags)
        {
        // Nothing to do
        return;
        }

    WString Path(( (HFCURLFile&)*m_DirCachePath).GetHost() +
                 WString(L"\\") +
                 ((HFCURLFile&)*m_DirCachePath).GetPath());

    // Get cache file list sorted by access time.
    FileEntryList fileEntryList;
    GetFileInfo(Path, &fileEntryList);

    // Append move folders to the file entry list
    for(MoveFolderList::const_iterator Itr(m_MoveFolderList.begin()); Itr != m_MoveFolderList.end(); ++Itr)
        {
        HFCPtr<HFCURL> pUrl(*Itr);
        Path = ( (HFCURLFile&)*pUrl).GetHost() + WString(L"\\") + ((HFCURLFile&)*pUrl).GetPath();

        GetFileInfo(Path, &fileEntryList);
        }

    // Delete old files
    if(pi_CacheControlFlags & CACHE_CONTROL_OLD)
        {
        // Get current time.
        time_t currentTime(BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000);    // time_t is in second.

        // From oldest to youngest
        FileEntryList::iterator OldItr(fileEntryList.begin());
        while(OldItr != fileEntryList.end() && (currentTime - OldItr->m_LastAccess) > m_FilesOlderThan)
            {
            FileEntry fileEntry(*OldItr);

            if(BeFileName::BeDeleteFile(fileEntry.m_FileName.c_str()) == BeFileNameStatus::Success)
                {
                OldItr = fileEntryList.erase(OldItr);
                }
            else
                {
                ++OldItr;
                }
            }
        }

    // Control folder size
    if(pi_CacheControlFlags & CACHE_CONTROL_SIZE)
        {
        // Compute the size used by the cache
        uint64_t CacheSize = 0;
        for(FileEntryList::iterator SizeItr(fileEntryList.begin()); SizeItr != fileEntryList.end(); ++SizeItr)
            {
            CacheSize += SizeItr->m_FileSize;
            }

        // From oldest to youngest
        FileEntryList::iterator DeleteItr(fileEntryList.begin());
        while(DeleteItr != fileEntryList.end() && CacheSize > m_MaxCacheSize)
            {
            FileEntry fileEntry(*DeleteItr);

            if(BeFileName::BeDeleteFile(fileEntry.m_FileName.c_str()) == BeFileNameStatus::Success)
                {
                CacheSize-=fileEntry.m_FileSize;
                DeleteItr = fileEntryList.erase(DeleteItr);
                }
            else
                {
                ++DeleteItr;  
                }
            }
        }

    // Move cache folder
    if(pi_CacheControlFlags & CACHE_CONTROL_MOVE && !m_MoveFolderList.empty())
        {
        WString currentFolder(((const HFCURLFile&)*m_DirCachePath).GetHost() + WString(L"\\") + ((const HFCURLFile&)*m_DirCachePath).GetPath());

        // Move all files to the current folder
        for(FileEntryList::iterator MoveItr(fileEntryList.begin()); MoveItr != fileEntryList.end(); ++MoveItr)
            {
            FileEntry  fileEntry(*MoveItr);
            HFCURLFile fileURL(WString(L"file://") + fileEntry.m_FileName.c_str());

            WString newFilename(currentFolder + L"\\" + fileURL.GetFilename());

            // Move file only if needed
            if(BeStringUtilities::Wcsicmp(fileEntry.m_FileName.c_str(), newFilename.c_str()) != 0)
                {
#if defined (_WIN32)
                MoveFileExW(fileEntry.m_FileName.c_str(), newFilename.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
#else
                BeFileName::BeCopyFile(fileEntry.m_FileName.c_str(), newFilename.c_str(), false);
                BeFileName::BeDeleteFile(fileEntry.m_FileName.c_str());
#endif
                }
            }
        fileEntryList.clear();
        m_MoveFolderList.clear();
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
void HRFCacheController::GetFileInfo(const WString& pi_rPath, FileEntryList* pi_pFileList)
    {
    HPRECONDITION(pi_pFileList != 0);

    WString FindPath(pi_rPath + L"\\*.*");

    BeFileListIterator FileList (FindPath.c_str(), m_ScanSubDir);
    BeFileName fn;
    while (FileList.GetNextFileName (fn) == SUCCESS)
        {
        // Insert all the file into the list
        if (IsCompliant(fn.GetName()))
            {
            FileEntry   EntryInfo;

            EntryInfo.m_FileName = fn.GetName();

            time_t AccesTime, ModifTime;
            BeFileName::GetFileTime (NULL, &AccesTime, &ModifTime, EntryInfo.m_FileName.c_str());
            EntryInfo.m_LastAccess = AccesTime;
            if (EntryInfo.m_LastAccess == -1L)   // Not available?
                EntryInfo.m_LastAccess = ModifTime;

            BeFileName::GetFileSize (EntryInfo.m_FileSize, EntryInfo.m_FileName.c_str());
            pi_pFileList->insert(FileEntryList::value_type(EntryInfo));
            }
        }
    }


/**-----------------------------------------------------------------------------
 This method verify if the file name was compliant with the a cache file name.

 @param  pi_rFileName The name of the file

 @return bool true if the file name has the string ".cache."
------------------------------------------------------------------------------*/
bool HRFCacheController::IsCompliant(const WString& pi_rFileName) const
    {
    HPRECONDITION(!pi_rFileName.empty());

    return CaseInsensitiveStringTools().Find(pi_rFileName, L".cache.") != WString::npos;
    }
