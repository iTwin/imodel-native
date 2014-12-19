//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFCacheSize.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFCacheSize
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"


/**
    This class is used to controle the size use by the cache.

    @see HRFLocalCacheFileCreator

*/

class HRFCacheSize : public HFCShareableObject<HRFCacheSize>
    {
public:

    HDECLARE_BASECLASS_ID(1409);

    //:> Primary methods.

    HRFCacheSize(const HFCURL&  pi_CachePath,
                 size_t         pi_CacheSize,
                 bool          pi_ScanSubDir = false);
    virtual                 ~HRFCacheSize();

    void                    ControlDirCacheSize();

private:

    struct FileEntry
        {
        WString m_FileName;
        time_t  m_LastAccess;
        size_t  m_FileSize;

        bool operator<(const FileEntry& pi_rObj) const
            {
            return m_LastAccess < pi_rObj.m_LastAccess;
            };
        };

    typedef list<FileEntry> FileEntryList;

    WString         m_DirCachePath;
    size_t          m_DirCacheSizeMax;
    bool           m_ScanSubDir;
    size_t          m_CurrentDirCacheSize;
    FileEntryList   m_FileList;

    // method
    bool           IsCompliant(const WString&  pi_rFileName) const;
    void            GetFileInfo(const WString&  pi_rPath,
                                FileEntryList*  pi_pFileList);

    HRFCacheSize(const HRFCacheSize&);
    HRFCacheSize& operator=(const HRFCacheSize&);
    };

