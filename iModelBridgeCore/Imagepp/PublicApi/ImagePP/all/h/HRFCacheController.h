//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFCacheController
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

/**
    This class is used to control the size use by the cache.

    @see HRFLocalCacheFileCreator

*/

BEGIN_IMAGEPP_NAMESPACE

class HRFCacheController: public HFCShareableObject<HRFCacheController>
    {
public:

    typedef bvector<BeFileName> MoveFolderList;

    enum CacheControlFlags
        {
        CACHE_CONTROL_NONE          = 0x00,
        CACHE_CONTROL_SIZE          = 0x01,
        CACHE_CONTROL_OLD           = 0x02,
        CACHE_CONTROL_MOVE          = 0x04
        };

    HDECLARE_BASECLASS_ID(HRFCacheControllerId_Base);

    //:> Primary methods.
    IMAGEPP_EXPORT                     HRFCacheController(BeFileNameCR pi_CachePath, bool pi_ScanSubDir = false);

    IMAGEPP_EXPORT virtual             ~HRFCacheController();

    IMAGEPP_EXPORT uint64_t           GetCacheSize() const;
    IMAGEPP_EXPORT void                SetCacheSize(uint64_t pi_CacheSizeBytes);

    IMAGEPP_EXPORT time_t              GetDeleteCacheFilesOlderThan() const;
    IMAGEPP_EXPORT void                SetDeleteCacheFilesOlderThan(time_t pi_OlderThan);

    IMAGEPP_EXPORT const MoveFolderList& GetMoveFolderList() const;
    IMAGEPP_EXPORT void                  SetMoveFolderList(const MoveFolderList& pi_MoveFolderList);

    IMAGEPP_EXPORT void                Control(int32_t pi_CacheControlFlags);

private:

    struct FileEntry
        {
        BeFileName  m_FileName;
        time_t      m_LastAccess;
        uint64_t    m_FileSize;

        bool operator<(const FileEntry& pi_rObj) const
            {
            return m_LastAccess < pi_rObj.m_LastAccess;
            };
        };

    typedef std::multiset<FileEntry> FileEntryList;

    // Cache directory
    BeFileName      m_DirCachePath;
    bool            m_ScanSubDir;

    // Control settings
    uint64_t        m_MaxCacheSize;         // In bytes
    time_t          m_FilesOlderThan;       // Number of secondes
    MoveFolderList  m_MoveFolderList;       // List of folders to move to the current directory

    // method
    bool            IsCompliant(BeFileNameCR pi_rFileName) const;
    void            GetFileInfo(BeFileNameCR pi_rPath, FileEntryList* pi_pFileList);

    // Disable methods
    HRFCacheController(const HRFCacheController&);
    HRFCacheController& operator=(const HRFCacheController&);
    };
END_IMAGEPP_NAMESPACE

