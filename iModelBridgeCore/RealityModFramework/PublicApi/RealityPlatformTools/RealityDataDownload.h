/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatformTools/RealityDataDownload.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <ctime>
#include <Bentley/BeFile.h>
#include <Bentley/bmap.h>
#include <BeXml/BeXml.h>
#include <Bentley/DateTime.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatform/RealityPlatformAPI.h>

//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation, (-1)General error, (-2)Retry the command. 
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ByteCurrent Number of byte currently downloaded.
//! @param[out] ByteTotal   When available, number of total bytes to download.
//! @return If RealityDataDownload_ProgressCallBack returns 0   The download continue.
//! @return If RealityDataDownload_ProgressCallBack returns # 0 The download is canceled for this file.
typedef std::function<int(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)> RealityDataDownload_ProgressCallBack;

// ErrorCode --> Tool error code.
//! Callback function to follow the download progression.
//! @param[out] index       Url index set at the creation
//! @param[out] pClient     Pointer on the structure RealityDataDownload::FileTransfer.
//! @param[out] ErrorCode   Tool error code:(0)Success (xx)Tool error (-1)General error, (-2)Retry the current download. 
//! @param[out] pMsg        Tool English message.
typedef std::function<void(int index, void *pClient, int ErrorCode, const char* pMsg)> RealityDataDownload_StatusCallBack;

//! Callback function to follow the download progression.
//! @return If RealityDataDownload_ProgressCallBack returns 0   All downloads continue.
//! @return If RealityDataDownload_ProgressCallBack returns any other value The download is canceled for all files.
typedef std::function<int()> RealityDataDownload_HeartbeatCallBack;

enum class ProxyStatus
    {
    ReturnedProxy,
    DefaultProxy,
    NoProxy,
    Abort
    };

//! Callback function to set proxy per URL.
//! @param[in]  url          full url of the resource we are trying to download.
//! @param[out] proxyUrl     url of the proxy to use.
//! @param[out] proxyCreds   credentials to pass to the proxy.
//! @return If RealityDataDownload_ProxyCallBack returns ReturnedProxy      The proxy info returned will be used.
//! @return If RealityDataDownload_ProxyCallBack returns DefaultProxy       The proxy specified with SetProxyUrlAndCredentials will be used.
//! @return If RealityDataDownload_ProxyCallBack returns NoProxy            The download will proceed without a proxy.
//! @return If RealityDataDownload_ProxyCallBack returns Abort              The download will be aborted.
typedef std::function<ProxyStatus(Utf8StringCR url, Utf8StringR proxyUrl, Utf8StringR proxyCreds)> RealityDataDownload_ProxyCallBack;

//! Callback function to set the token for the request.
//! @param[in]  tokenType    the type of token to add to the call 
//! @param[out] url          url of the resource we are trying to download.
//! @param[out] header       header passed in the rest call.
typedef std::function<void(Utf8StringCR TokenType, AStringR url, Utf8StringR header)> RealityDataDownload_TokenCallBack;

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

enum class SetupRequestStatus
    {
    Success                     = SUCCESS,
    FromCache,
    Error                       = ERROR,
    };


struct RealityDataDownload : public RefCountedBase
    {
public:

    REALITYDATAPLATFORM_EXPORT static int  s_MaxRetryTentative;
    
    struct LoginInfo
        {
        Utf8String          loginUrl;
        bvector<Utf8String> headers;
        Utf8String          postBody;
        //Utf8String          cookie;
        };

    struct AuthInfo
        {
        bvector<Utf8String> headers;
        Utf8String          postBody;
        Utf8String          cookie;

        AuthInfo():headers(bvector<Utf8String>()), postBody(""), cookie(""){}
        };

    struct DownloadCap
        {
        Utf8String      sourceId;
        int             concurrentDownloadCap;
        int             currentDownloads;
        LoginInfo*      login;
        AuthInfo        auth;

        DownloadCap(const DownloadCap& cap) 
            {
            sourceId = cap.sourceId;
            concurrentDownloadCap = cap.concurrentDownloadCap;
            currentDownloads = cap.currentDownloads;
            login = cap.login;
            auth = cap.auth;
            }

        DownloadCap() : sourceId(""), concurrentDownloadCap(0), currentDownloads(0),
            login(nullptr), auth(AuthInfo()){}

        DownloadCap(Utf8String id, int downloadCap, AuthInfo authInfo, LoginInfo* loginParams = nullptr): 
            sourceId(id), concurrentDownloadCap(downloadCap), currentDownloads(0),
            login(loginParams), auth(authInfo) {}
        };

    struct url_file_pair
        {   
        AString m_url;
        WString m_filePath;
        Utf8String m_tokenType;

        DownloadCap* m_cap;

        url_file_pair(AString url, WString filePath, Utf8String tokenType = "", DownloadCap* cap = nullptr) :
            m_url(url), m_filePath(filePath), m_tokenType(tokenType), m_cap(cap)
            {}
        };    
    
	typedef bvector<url_file_pair>         sisterFileVector; //this vector contains the primary file and its sister files
    typedef bvector<sisterFileVector>      mirrorWSistersVector; //this vector contains mirrors of a set of sister files to download
    typedef bvector<url_file_pair>         mirrorVector; //this vector contains mirrors of the same data to download

    struct FileTransfer;

    struct Mirror_struct
        {
        AString url;
        WString filename;
        Utf8String tokenType;
        FileTransfer* nextSister = nullptr;
        size_t sisterIndex;
        size_t totalSisters;
        std::time_t DownloadStart;

        DownloadCap* cap = nullptr;
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
        RealityDataDownload_HeartbeatCallBack pHeartbeatFunc;
        size_t                  filesize;
        size_t                  downloadedSizeStep;
        float                   progressStep;
        int                     nbRetry;

        void InsertMirror(url_file_pair ufPair, size_t id, size_t sisterCount)
            {
            Mirror_struct ms;
            ms.url = ufPair.m_url;
            ms.filename = ufPair.m_filePath;
            ms.tokenType = ufPair.m_tokenType;
            ms.nextSister = nullptr;
            ms.totalSisters = sisterCount;
            ms.sisterIndex = 0;
            ms.cap = ufPair.m_cap;
            mirrors.push_back(ms);
            }

        FileTransfer(){}

        FileTransfer(FileTransfer* ft):mirrors(bvector<Mirror_struct>()), filename(ft->filename), index(ft->index), 
            fromCache(true), iAppend(ft->iAppend), pProgressFunc(ft->pProgressFunc), filesize(ft->filesize),
            downloadedSizeStep(ft->downloadedSizeStep), progressStep(ft->progressStep), nbRetry(ft->nbRetry)
            {}
        };

    //where the tool download ended, either in success or failure
    struct DownloadResult
        {
        int                     errorCode; //code returned by the tool
        size_t                  downloadProgress; //a percentage of how much of the file was successfully downloaded
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

        REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR report)
            {
#ifndef __APPLE__
            BeXmlWriterPtr writer = BeXmlWriter::Create();
            BeAssert(writer.IsValid());
            writer->SetIndentation(2);

            //writer->WriteDocumentStart(xmlCharEncoding::XML_CHAR_ENCODING_UTF8);

            writer->WriteElementStart("RealityDataDownload_DownloadReport");
                {
                writer->WriteAttribute("PackageId", packageId);
                writer->WriteAttribute("Date", Utf8String(DateTime::GetCurrentTimeUtc().ToString()).c_str());

                for (bmap<WString, TransferReport*>::iterator it = results.begin(); it != results.end(); ++it)
                    {
                    writer->WriteElementStart("File");
                        {
                        writer->WriteAttribute("FileName", Utf8String(it->first).c_str());
                        TransferReport* tr = it->second;
                        writer->WriteAttribute("url", Utf8CP(tr->url.c_str()));
                        writer->WriteAttribute("filesize", tr->filesize);
                        writer->WriteAttribute("timeSpent", (uint64_t)tr->timeSpent);
                        for(size_t i = 0; i < tr->retries.size(); ++i)
                            {
                            writer->WriteElementStart("DownloadAttempt");
                                {
                                writer->WriteAttribute("attemptNo", i+1);
                                writer->WriteAttribute("Toolcode", tr->retries.at(i).errorCode);
                                writer->WriteAttribute("downloadProgress", tr->retries.at(i).downloadProgress);
                                }
                            writer->WriteElementEnd();
                            }
                        }
                    writer->WriteElementEnd();
                    }
                }
                writer->WriteElementEnd();
                writer->ToString(report);
#else
                assert(!"Not compiling on iOS");
#endif
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
    REALITYDATAPLATFORM_EXPORT void SetProxyUrlAndCredentials(Utf8StringCR proxyUrl, Utf8StringCR proxyCreds) 
        { 
        m_proxyUrl = proxyUrl; 
        m_proxyCreds = proxyCreds; 
        };

    //! Set certificate path for https download.
    REALITYDATAPLATFORM_EXPORT void SetCertificatePath(WStringCR certificatePath) { m_certPath = certificatePath; };

    //! Set callback to follow progression of the download.
    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func, float pi_step = 0.01) 
        {
        m_pProgressFunc = pi_func; 
        m_progressStep = pi_step;
        }
    //! Set callback to allow the user to mass cancel all downloads
    REALITYDATAPLATFORM_EXPORT void SetHeartbeatCallBack(RealityDataDownload_HeartbeatCallBack pi_func)
                                                                   {m_pHeartbeatFunc = pi_func;}

    //! Set callback to know to status, download done or error.
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataDownload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; }

    //! Set callback to chose a proxy to use, for each url.
    REALITYDATAPLATFORM_EXPORT void SetProxyCallBack(RealityDataDownload_ProxyCallBack pi_func) { m_pProxyFunc = pi_func; }

    //! Set callback to add authentication token to REST call.
    REALITYDATAPLATFORM_EXPORT void SetTokenCallBack(RealityDataDownload_TokenCallBack pi_func) { m_pTokenFunc = pi_func; }

    //! Start the download progress for all links.
    REALITYDATAPLATFORM_EXPORT DownloadReport* Perform();

    //! Set the timeout to cancel a download, if it hangs
    REALITYDATAPLATFORM_EXPORT void SetTimeout(long timeInS) { m_timeout = timeInS; }

private:
    RealityDataDownload();
    RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName);
    RealityDataDownload(const Link_File_wMirrors& pi_Link_File_wMirrors);
    RealityDataDownload(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters);
    ~RealityDataDownload();

    void InitEntry(size_t i);

    void SetProxy(void* pTool, Utf8StringCR proxyUrl, Utf8StringCR proxyCreds);
    SetupRequestStatus Login(LoginInfo& loginInfo, AuthInfo& authInfo);
    SetupRequestStatus SetupRequestandFile(FileTransfer* ft, bool isRetry = false);
    bool SetupNextEntry();
    bool SetupMirror(size_t index, int errorCode);
    void AddSisterFiles(FileTransfer* ft, bvector<url_file_pair> sisters, size_t index, size_t sisterCount, size_t sisterIndex);

    void ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg);

    void*                       m_pToolHandle;
    size_t                      m_nbEntry;
    size_t                      m_curEntry;
    FileTransfer                *m_pEntries;
    long                        m_timeout = 0L;

    WString                                 m_certPath;
    Utf8String                              m_proxyUrl;
    Utf8String                              m_proxyCreds;
    RealityDataDownload_ProgressCallBack    m_pProgressFunc;
    RealityDataDownload_HeartbeatCallBack   m_pHeartbeatFunc;
    float                                   m_progressStep = 0.01f;
    RealityDataDownload_StatusCallBack      m_pStatusFunc;
    RealityDataDownload_ProxyCallBack       m_pProxyFunc;
    RealityDataDownload_TokenCallBack       m_pTokenFunc;
    DownloadReport*                         m_dlReport;
    bmap<Utf8String, DownloadCap>           m_caps;
    bvector<bpair<Utf8String, FileTransfer*>> m_waitingList;
    };
    
END_BENTLEY_REALITYPLATFORM_NAMESPACE
