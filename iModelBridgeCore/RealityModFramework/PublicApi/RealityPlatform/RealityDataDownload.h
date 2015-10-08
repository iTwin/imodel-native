/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataDownload.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "RealityPlatformAPI.h"
#include <Bentley/BeFile.h>

// 
// index --> Url index set at the create
// index == -1 --> General error, pClient will be NULL.
// ErrorCode --> Curl error code.
// If RealityDataDownload_ProgressCallBack returns # 0 --> cancel the transfer
typedef int(__cdecl *RealityDataDownload_ProgressCallBack)(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal);
typedef void(__cdecl *RealityDataDownload_StatusCallBack) (int index, void *pClient, int ErrorCode, const char* pMsg);


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
//! RealityDataDownload
//! @bsiclass
//=======================================================================================
struct RealityDataDownload : public RefCountedBase
{
public:
    struct FileTransfer 
        {
        AString  url;
        WString  filename;
        size_t   index;


        // internal used only
        BeFile                  fileStream;
        size_t                  iAppend;
        RealityDataDownload_ProgressCallBack pProgressFunc;
        int                     nbRetry;
        };

    typedef bvector<std::pair<AString, WString>>    UrlLink_UrlFile;

    //! Create an instance of RealityDataDownload
    //! @param[in]  pi_Link_FileName A list of (Url link, url file)
    //! @return NULL if error   
    REALITYDATAPLATFORM_EXPORT static RealityDataDownloadPtr Create(const UrlLink_UrlFile& pi_Link_FileName);

    // pio_rFileName could already contain the first part of the path, like "C:\\Data\\"
    //               the filename extract from the url, will be concatenated. 
    // Note:
    //      Not working very well for OSM link.

    //! Utility method to extract filename from the url link if possible.
    //! @param[in/out] pio_rFileName Could already contain the first part of the path, like "C:\\Data\\"
    //!                              the filename extract from the url, will be concatenated. 
    //! @param[in] pi_Url            Url link string.
    REALITYDATAPLATFORM_EXPORT static void RealityDataDownload::ExtractFileName(WString& pio_rFileName, const AString& pi_Url);

    //! Set callback to follow progression of the download.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func) {m_pProgressFunc = pi_func;};
    //! Set callback to know to status, download done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataDownload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; };

    //! Start the download progress for all links.
    REALITYDATAPLATFORM_EXPORT bool Perform();

private:
    RealityDataDownload() { m_pCurlHandle=NULL;};
    RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName);
    ~RealityDataDownload();

    bool SetupCurlandFile(size_t pi_index);

    void*                       m_pCurlHandle;
    size_t                      m_nbEntry;
    size_t                      m_curEntry;
//    HArrayAutoPtr<FileTransfer> m_pEntries;
    FileTransfer                *m_pEntries;

    RealityDataDownload_ProgressCallBack    m_pProgressFunc;
    RealityDataDownload_StatusCallBack      m_pStatusFunc;

};

END_BENTLEY_REALITYPLATFORM_NAMESPACE