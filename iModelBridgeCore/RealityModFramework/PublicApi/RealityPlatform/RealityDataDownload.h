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

//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation, (-1)General error, (-2)Retry the command. 
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ByteCurrent Number of byte currently downloaded.
//! @param[out] ByteTotal   When available, number of total bytes to download.
//! @return If RealityDataDownload_ProgressCallBack returns 0   The download continue.
//! @return If RealityDataDownload_ProgressCallBack returns # 0 The download is canceled for this file.
typedef int(__cdecl *RealityDataDownload_ProgressCallBack)(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal);

// ErrorCode --> Curl error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Curl error code:(0)Success (xx)Curl (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Curl English message.
typedef void(__cdecl *RealityDataDownload_StatusCallBack) (int index, void *pClient, int ErrorCode, const char* pMsg);


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
//! RealityDataDownload is an utility class to download data from the links included
//! in the package.
//! 
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
        bool     fromCache;         // as input: skip the download if the file already exist
                                    // as output: specify if the file was downloaded or not.


        // internal used only
        BeFile                  fileStream;
        size_t                  iAppend;
        RealityDataDownload_ProgressCallBack pProgressFunc;
        size_t                  filesize;
        size_t                  downloadedSizeStep;
        float                   progressStep;
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
    REALITYDATAPLATFORM_EXPORT static void ExtractFileName(WString& pio_rFileName, const AString& pi_Url);
    REALITYDATAPLATFORM_EXPORT static bool UnZipFile(WString& pi_strSrc, WString& pi_strDest);

    //! Set callback to follow progression of the download.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func, float pi_step = 0.01) 
                                                                   {m_pProgressFunc = pi_func; m_progressStep = pi_step;};
    //! Set callback to know to status, download done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataDownload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; };

    //! Start the download progress for all links.
    REALITYDATAPLATFORM_EXPORT bool Perform();

private:
    RealityDataDownload() { m_pCurlHandle=NULL;};
    RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName);
    ~RealityDataDownload();

    bool SetupCurlandFile(size_t pi_index);
    bool SetupNextEntry();

    void*                       m_pCurlHandle;
    size_t                      m_nbEntry;
    size_t                      m_curEntry;
//    HArrayAutoPtr<FileTransfer> m_pEntries;
    FileTransfer                *m_pEntries;

    RealityDataDownload_ProgressCallBack    m_pProgressFunc;
    float                                   m_progressStep;
    RealityDataDownload_StatusCallBack      m_pStatusFunc;

};

END_BENTLEY_REALITYPLATFORM_NAMESPACE