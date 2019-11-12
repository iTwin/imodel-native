/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/RealityDataDownload.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int RealityDataDownload::s_MaxRetryTentative = 25;

//=======================================================================================
//                              RealityDataDownload
//=======================================================================================

RealityDataDownload::RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName) : RealityDataDownload()
    {
    m_nbEntry = pi_Link_FileName.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i=0; i<m_nbEntry; ++i)
        {
        m_pEntries[i].InsertMirror(pi_Link_FileName[i], 0, 0);
        InitEntry(i);
        m_pEntries[i].filename = m_pEntries[i].mirrors[0].filename;
        }
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors& pi_Link_File_wMirrors) : RealityDataDownload()
    {
    m_nbEntry = pi_Link_File_wMirrors.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i = 0; i<m_nbEntry; ++i)
        {
        size_t mirrorCount = pi_Link_File_wMirrors[i].size();
        for(size_t j = 0; j < mirrorCount; ++j)
            m_pEntries[i].InsertMirror(pi_Link_File_wMirrors[i][j], j, 0);
        InitEntry(i);
        m_pEntries[i].filename = m_pEntries[i].mirrors[0].filename;
        }
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters) :
    RealityDataDownload()
    {
    m_nbEntry = pi_Link_File_wMirrors_wSisters.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    size_t sisterCount, mirrorCount;
    for (size_t i = 0; i<m_nbEntry; ++i)
        {
        InitEntry(i);

        mirrorCount = pi_Link_File_wMirrors_wSisters[i].size();
        
        for (size_t j = 0; j < mirrorCount; ++j)
            {
            sisterCount = pi_Link_File_wMirrors_wSisters[i][j].size();
            m_pEntries[i].InsertMirror(pi_Link_File_wMirrors_wSisters[i][j][0], j, sisterCount);
            if(pi_Link_File_wMirrors_wSisters[i][j].size() > 1)
                AddSisterFiles(&m_pEntries[i], pi_Link_File_wMirrors_wSisters[i][j], 1, sisterCount, 1);
            }

        m_pEntries[i].filename = m_pEntries[i].mirrors[0].filename;

        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
RealityDataDownload::~RealityDataDownload()
    {
    if (m_pEntries)
        delete[] m_pEntries;
    };

//----------------------------------------------------------------------------------------
// @bsimethod                           Spencer.Mason                             01/2018
// Without Curl, the RealityDataDownload class loses its purpose. Do not use this class,
// if you are using RealityPlatformToolsLight
//----------------------------------------------------------------------------------------
RealityDataDownload::DownloadReport* RealityDataDownload::Perform()
    {
    m_dlReport = new DownloadReport();
    //m_caps = bvector<downloadCap>();
    m_caps = bmap<Utf8String, DownloadCap>();

    m_curEntry = 0;

    //perform download

    return m_dlReport;
    }

void RealityDataDownload::SetProxy(void* pTool, Utf8StringCR proxyUrl, Utf8StringCR proxyCreds)
    {
    //set proxy
    }

SetupRequestStatus RealityDataDownload::Login(LoginInfo& loginInfo, AuthInfo& authInfo)
    {
    //login
    return SetupRequestStatus::Success;
    }

SetupRequestStatus RealityDataDownload::SetupRequestandFile(FileTransfer* ft, bool isRetry)
    {
    // If cancel requested, don't queue new files
    if(NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return SetupRequestStatus::Success;

    // If file already there, consider the download completed.
    while (ft != nullptr && ft->fromCache && BeFileName::DoesPathExist(ft->filename.c_str()))
        {
        if(m_pStatusFunc)
            m_pStatusFunc((int) ft->index, ft, 0, nullptr);

        ft = ft->mirrors[0].nextSister;

        if (nullptr != ft && m_pStatusFunc)
            m_pStatusFunc((int)ft->index, ft, REALITYDATADOWNLOAD_CONTACTING_SISTER, "File already in cache, attempting to retrieve next sister if exists");
        }
    if (ft == nullptr)
        return SetupRequestStatus::FromCache;

    // File not in the cache, we will download it
    ft->fromCache = false;

    //setup file

    return SetupRequestStatus::Success;
    }