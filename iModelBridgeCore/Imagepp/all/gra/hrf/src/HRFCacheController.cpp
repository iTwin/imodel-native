//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class HRFCacheController
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>
    //:> must be first for PreCompiledHeader Option
#include <ImagePP/all/h/HRFCacheController.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCStat.h>

/**-----------------------------------------------------------------------------
 Public constructor of the class.
------------------------------------------------------------------------------*/
HRFCacheController::HRFCacheController(BeFileNameCR pi_CachePath, bool pi_ScanSubDir)
    :m_DirCachePath(pi_CachePath),
     m_ScanSubDir(pi_ScanSubDir),
     m_MaxCacheSize(UINT64_MAX),
     m_FilesOlderThan(UINT32_MAX)       // ~68 years
    {
    BeAssert(pi_CachePath.IsDirectory());
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
    m_MoveFolderList = pi_MoveFolderList;
    }

/**-----------------------------------------------------------------------------
This method control the cache directory using the specified flags.
------------------------------------------------------------------------------*/
void HRFCacheController::Control(int32_t pi_CacheControlFlags)
    {

    if(0 == pi_CacheControlFlags)
        {
        // Nothing to do
        return;
        }

    // Get cache file list sorted by access time.
    FileEntryList fileEntryList;
    GetFileInfo(m_DirCachePath, &fileEntryList);

    // Append move folders to the file entry list
    for(MoveFolderList::const_iterator Itr(m_MoveFolderList.begin()); Itr != m_MoveFolderList.end(); ++Itr)
        {
        GetFileInfo(*Itr, &fileEntryList);
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

            if(fileEntry.m_FileName.BeDeleteFile() == BeFileNameStatus::Success)
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
            if(fileEntry.m_FileName.BeDeleteFile() == BeFileNameStatus::Success)
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
        // Move all files to the current folder
        for(FileEntryList::iterator MoveItr(fileEntryList.begin()); MoveItr != fileEntryList.end(); ++MoveItr)
            {
            FileEntry  fileEntry(*MoveItr);
            
            BeFileName filenameAndExt(fileEntry.m_FileName.GetFileNameAndExtension());

            BeFileName newFilename = m_DirCachePath;
            newFilename.AppendToPath(filenameAndExt);

            // Move file only if needed
            if(!newFilename.EqualsI(fileEntry.m_FileName))
                {
                newFilename.BeDeleteFile(); // Delete old one that may exist in the destination folder.
                BeFileNameStatus status = BeFileName::BeMoveFile(fileEntry.m_FileName, newFilename);
                BeAssert(BeFileNameStatus::Success == status);
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
void HRFCacheController::GetFileInfo(BeFileNameCR pi_rPath, FileEntryList* pi_pFileList)
    {
    HPRECONDITION(pi_pFileList != 0);

    BeFileName FindPath(pi_rPath);
    FindPath.AppendToPath(L"*.*");

    BeFileListIterator FileList (FindPath, m_ScanSubDir);
    BeFileName fn;
    while (FileList.GetNextFileName (fn) == SUCCESS)
        {
        // Insert all the file into the list
        if (IsCompliant(fn))
            {
            FileEntry   EntryInfo;

            EntryInfo.m_FileName = fn;

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
bool HRFCacheController::IsCompliant(BeFileNameCR pi_rFileName) const
    {
    HPRECONDITION(!pi_rFileName.empty());

    return pi_rFileName.ContainsI(L".cache.");
    }
