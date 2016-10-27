/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataDownload.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <ctime>
#include <Bentley/BeFile.h>
#include <Bentley/bmap.h>
#include "RealityPlatformAPI.h"

//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation, (-1)General error, (-2)Retry the command. 
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ByteCurrent Number of byte currently downloaded.
//! @param[out] ByteTotal   When available, number of total bytes to download.
//! @return If RealityDataDownload_ProgressCallBack returns 0   The download continue.
//! @return If RealityDataDownload_ProgressCallBack returns # 0 The download is canceled for this file.
typedef std::function<int(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)> RealityDataDownload_ProgressCallBack;

// ErrorCode --> Curl error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Curl error code:(0)Success (xx)Curl (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Curl English message.
typedef std::function<void(int index, void *pClient, int ErrorCode, const char* pMsg)> RealityDataDownload_StatusCallBack;

//Special Error codes
#define REALITYDATADOWNLOAD_RETRY_TENTATIVE     -2
#define REALITYDATADOWNLOAD_MIRROR_CHANGE       -3
#define REALITYDATADOWNLOAD_CONTACTING_SISTER   -4

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
//! RealityDataDownload is an utility class to download data from the links included
//! in the package.
//! 
//! @bsiclass
//=======================================================================================

enum class SetupCurlStatus
    {
    Success                     = SUCCESS,
    FromCache,
    Error                       = ERROR,
    };


struct RealityDataDownload : public RefCountedBase
    {
public:

    REALITYDATAPLATFORM_EXPORT static int  s_MaxRetryTentative;

    typedef std::pair<AString, WString>    url_file_pair; //this pair contains the url and the file name
    typedef bvector<url_file_pair>         sisterFileVector; //this vector contains the primary file and its sister files
    typedef bvector<sisterFileVector>      mirrorWSistersVector; //this vector contains mirrors of a set of sister files to download
    typedef bvector<url_file_pair>         mirrorVector; //this vector contains mirrors of the same data to download

    struct FileTransfer;

    struct Mirror_struct
        {
        AString url;
        WString filename;
        FileTransfer* nextSister;
        size_t sisterIndex;
        size_t totalSisters;
        std::time_t DownloadStart;
        };

    struct FileTransfer 
        {
        bvector<Mirror_struct>  mirrors;
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

        void InsertMirror(url_file_pair ufPair, size_t id, size_t sisterCount)
            {
            Mirror_struct ms;
            ms.url = ufPair.first;
            ms.filename = ufPair.second;
            ms.nextSister = nullptr;
            ms.totalSisters = sisterCount;
            ms.sisterIndex = 0;
            mirrors.push_back(ms);
            }

        FileTransfer(){}

        FileTransfer(FileTransfer* ft):mirrors(bvector<Mirror_struct>()), filename(ft->filename), index(ft->index), 
            fromCache(true), iAppend(ft->iAppend), pProgressFunc(ft->pProgressFunc), filesize(ft->filesize),
            downloadedSizeStep(ft->downloadedSizeStep), progressStep(ft->progressStep), nbRetry(ft->nbRetry)
            {}
        };

    //where the curl download ended, either in success or failure
    struct DownloadResult
        {
        int                     errorCode; //code returned by curl
        //a relative measure of how much of the file was loaded = (total filesize * (% downloaded + 1%))
        size_t                  downloadProgress; 
        };

    //results for a single file
    struct TransferReport
        {
        AString                 url; //url that was contacted
        size_t                  filesize; //size of the file
        bvector<DownloadResult> retries; //the results of each retry performed
        //time between when the download was added to the thread pool and when it either succeeded or failed its final retry
        //important to note, that if the source limits the number of parallel downloads, this value may be artificially extended
        std::time_t             timeSpent; 
        };

    //results of all downloads queued for a package
    struct DownloadReport
        {
        size_t                  packageId;
        bmap<WString, TransferReport*> results;
        ~DownloadReport()
            {
            for (bmap<WString, TransferReport*>::iterator it = results.begin(); it != results.end(); ++it)
                delete (it->second);
            }
        };

    //{{url, file},{url, file}}
    typedef bvector<url_file_pair>    UrlLink_UrlFile;
    //{{mirror set:{url, file}, {url, file}}, {mirror set: ...}} 
    typedef bvector<mirrorVector> Link_File_wMirrors;
    //{{mirror set:{sister set:{url, file}, {url, file}}, {sister set:{url, file}}}{mirror set: ...}}
    typedef bvector<mirrorWSistersVector> Link_File_wMirrors_wSisters;
    
    //! Create an instance of RealityDataDownload
    //! @param[in]  pi_Link_FileName A list of (Url link, url file)
    //! @return NULL if error   
    REALITYDATAPLATFORM_EXPORT static RealityDataDownloadPtr Create(const UrlLink_UrlFile& pi_Link_FileName);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownloadPtr Create(const Link_File_wMirrors& pi_Link_File_wMirrors);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownloadPtr Create(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters);

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
    REALITYDATAPLATFORM_EXPORT static bool UnZipFile(const char* pi_srtSrc, const char* pi_strDest);

    //! Set proxy informations
    REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) { m_proxyUrl = proxyUrl; m_proxyCreds = proxyCreds; };

    //! Set certificate path for https download.
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(WStringCR certificatePath) { m_certPath = certificatePath; };

    //! Set callback to follow progression of the download.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func, float pi_step = 0.01) 
                                                                   {m_pProgressFunc = pi_func; m_progressStep = pi_step;};
    //! Set callback to know to status, download done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataDownload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; };

    //! Start the download progress for all links.
    REALITYDATAPLATFORM_EXPORT DownloadReport* Perform();

private:
    RealityDataDownload() { m_pCurlHandle=NULL;};
    RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName);
    RealityDataDownload(const Link_File_wMirrors& pi_Link_File_wMirrors);
    RealityDataDownload(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters);
    ~RealityDataDownload();

    SetupCurlStatus SetupCurlandFile(FileTransfer* ft, bool isRetry = false);
    bool SetupNextEntry();
    bool SetupMirror(size_t index, int errorCode);
    void AddSisterFiles(FileTransfer* ft, bvector<url_file_pair> sisters, size_t index, size_t sisterCount, size_t sisterIndex);

    void ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg);

    void*                       m_pCurlHandle;
    size_t                      m_nbEntry;
    size_t                      m_curEntry;
    FileTransfer                *m_pEntries;

    WString                                 m_certPath;
    Utf8String                              m_proxyUrl;
    Utf8String                              m_proxyCreds;
    RealityDataDownload_ProgressCallBack    m_pProgressFunc;
    float                                   m_progressStep = 0.01;
    RealityDataDownload_StatusCallBack      m_pStatusFunc;
    DownloadReport                          m_dlReport;

    };
    
END_BENTLEY_REALITYPLATFORM_NAMESPACE