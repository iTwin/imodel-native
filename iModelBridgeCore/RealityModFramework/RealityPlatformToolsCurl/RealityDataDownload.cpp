/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsCurl/RealityDataDownload.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/RealityDataDownload.cpp"

#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <RealityPlatformTools/SimpleRDSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

int RealityDataDownload::s_MaxRetryTentative = 25;

//=======================================================================================
//                              RealityDataDownload
//=======================================================================================

static size_t callback_fwrite_func(void *buffer, size_t size, size_t nmemb, void *pClient)
    {
    if (NULL == pClient)
        return 0;

    struct RealityDataDownload::FileTransfer *out = (struct RealityDataDownload::FileTransfer *)pClient;
    if (!(out->fileStream.IsOpen()))
        {
        if (out->iAppend)
            {
            if (out->fileStream.Open(out->filename.c_str(), BeFileAccess::Write) != BeFileStatus::Success)
                return 0;   // failure, can't open file to write
            }
        else
            {
            if (out->fileStream.Create(out->filename.c_str(), true) != BeFileStatus::Success)
                return 0;   // failure, can't open file to write
            }
        }
    out->iAppend += nmemb;
    uint32_t byteWritten;
    if (out->fileStream.Write(&byteWritten, buffer, (uint32_t)(size*nmemb)) != BeFileStatus::Success)
        byteWritten = 0;

#ifdef TRACE_DEBUG
    fprintf(stderr, "callback_fwrite_func byteWritten %lu\n", byteWritten);
#endif

    return byteWritten;
    }

static int callback_progress_func(void *pClient,
    curl_off_t dltotal,
    curl_off_t dlnow,
    curl_off_t ultotal,
    curl_off_t ulnow)
    {
    (void)ultotal;
    (void)ulnow;

    struct RealityDataDownload::FileTransfer *pFileTrans = (struct RealityDataDownload::FileTransfer *)pClient;

    int statusCode = 0;
    if (NULL != pFileTrans->pHeartbeatFunc)
        statusCode = pFileTrans->pHeartbeatFunc();

    if(statusCode == 0)
        {
        if (pFileTrans->filesize == 0 && dltotal > 0)
            {
            pFileTrans->filesize = (size_t)dltotal;
            pFileTrans->downloadedSizeStep = (size_t)(pFileTrans->filesize * pFileTrans->progressStep);
            }

        if ((size_t)dltotal > pFileTrans->downloadedSizeStep)
            {
            // TODO Alain Robert XXX - The cast to size_t of the float is doubtful
            pFileTrans->downloadedSizeStep += (size_t)pFileTrans->progressStep;

            if (NULL != pFileTrans->pProgressFunc)
                statusCode = (pFileTrans->pProgressFunc)((int) pFileTrans->index, pClient, (size_t) dlnow, dltotal);
            }
        }

#ifdef TRACE_DEBUG
    fprintf(stderr, "callback_progress_func total:%llu now: %llu\n", (size_t)dltotal, (size_t)dlnow);
#endif

    if (0 != statusCode)
        {
        // An error occurred, delete incomplete file.
        pFileTrans->fileStream.Close();
        BeFileName::BeDeleteFile(pFileTrans->filename.c_str());
        }

    return 0;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//* writes the body of a curl reponse to a Utf8String
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

/*static bvector<downloadCap>::itterator findInCap(const Utf8String& capId, const downloadCap& caps)
    {
    return std::find_if(caps.begin(), caps.end(), [&capId](const downloadCap& obj) {return obj.sourceId == capId; })
    }*/

static RealityDataDownload_ProgressCallBack s_RDSProgressCallback = 0;
static RealityDataDownload_StatusCallBack s_RDSStatusCallback = 0;

void RealityDataDownload::ParseRDS()
    {
    m_omittedEntries = bset<size_t>();

    for(size_t i = 0; i < m_nbEntry; ++i)
        {
        WString url = WString(m_pEntries[i].mirrors[0].url.c_str(), true);
        if(url.ContainsI(L"S3MXECPlugin") && !url.EndsWithI(L".zip"))
            {
            m_omittedEntries.insert(i);
            /*size_t pos = url.find_last_of(L"/\\");
            WString guid = url.substr(pos + 1, pos + 37);
            m_RDSEntries.push_back(guid);*/
            }
        }
    
    s_RDSProgressCallback = m_pProgressFunc;
    }

static RealityDataDownload::FileTransfer* s_CurrentRDSDownload;

static void RDSProgressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    if(s_RDSProgressCallback && s_CurrentRDSDownload != nullptr)
        s_RDSProgressCallback((int)(s_CurrentRDSDownload->index), s_CurrentRDSDownload, (size_t)( 10000.00 * repoProgress), 10000);
    }

void RealityDataDownload::DownloadFromRDS()
    {
    RDSRequestManager::Setup();
    
    RealityDataService::SetProjectId(m_projectId);

    for(bset<size_t>::iterator it = m_omittedEntries.begin(); it != m_omittedEntries.end(); ++it)
        {
        Mirror_struct& ms = m_pEntries[*it].mirrors[0];

        WString url = WString(ms.url.c_str(), true);
        size_t pos = url.find_last_of(L"/\\");
        WString guid = url.substr(pos + 1, 36);
        
        BeFileName targetPath(BeFileName::GetDirectoryName(ms.filename.c_str()).c_str());
        targetPath.AppendToPath(guid.c_str());
        targetPath.AppendSeparator();
        
        RealityDataServiceDownload download = RealityDataServiceDownload(targetPath, Utf8String(guid.c_str()));
        if (download.IsValidTransfer())
            {
            s_CurrentRDSDownload = &m_pEntries[*it];
            download.SetProgressCallBack(RDSProgressFunc);
            download.SetProgressStep(0.01);
            download.OnlyReportErrors(true);
            download.Perform();
            }
        Utf8String msg = Utf8PrintfString("RDS download completed for %ls", targetPath.c_str());
        if(m_pStatusFunc)
            m_pStatusFunc((int)(s_CurrentRDSDownload->index), s_CurrentRDSDownload, 0, msg.c_str());
        }
    }

void RealityDataDownload::SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func, float pi_step)
    {
    m_pProgressFunc = pi_func; 
    m_progressStep = pi_step;
    s_RDSProgressCallback = m_pProgressFunc;
    }

RealityDataDownload::RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName) : RealityDataDownload()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pToolHandle = curl_multi_init();
    
    m_nbEntry = pi_Link_FileName.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i=0; i<m_nbEntry; ++i)
        {
        m_pEntries[i].InsertMirror(pi_Link_FileName[i], 0, 0);
        InitEntry(i);
        m_pEntries[i].filename = m_pEntries[i].mirrors[0].filename;
        }
    ParseRDS();
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors& pi_Link_File_wMirrors) : RealityDataDownload()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pToolHandle = curl_multi_init();
    
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
    ParseRDS();
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters) :
    RealityDataDownload()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pToolHandle = curl_multi_init();
    
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
    ParseRDS();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
RealityDataDownload::~RealityDataDownload()
    {
    if (m_pToolHandle != NULL)
        curl_multi_cleanup(m_pToolHandle);
    curl_global_cleanup();

    if (m_pEntries)
        delete[] m_pEntries;
    };

RealityDataDownload::DownloadReport* RealityDataDownload::Perform()
    {
    m_dlReport = new DownloadReport();
    //m_caps = bvector<downloadCap>();
    m_caps = bmap<Utf8String, DownloadCap>();
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pToolHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    m_curEntry = 0;
    bool atLeast1Download = false;
    int maxConnections = (MAX_NB_CONNECTIONS < (int)m_nbEntry ?  MAX_NB_CONNECTIONS : (int)m_nbEntry);

    for (size_t i = 0; i < maxConnections; ++i)
        {
        if (SetupNextEntry())
            atLeast1Download = true;
        else
            break;
        }

    // if everything already downloaded, exit with success.
    if (atLeast1Download)
        {
        int still_running; /* keep number of running handles */
        int repeats = 0;

        do
            {
            CURLMcode mc; /* curl_multi_wait() return code */
            int numfds;

            curl_multi_perform(m_pToolHandle, &still_running);

            /* wait for activity, timeout or "nothing" */
            mc = curl_multi_wait(m_pToolHandle, NULL, 0, 1000, &numfds);

            if (mc != CURLM_OK)
                {
                ReportStatus(-1, NULL, mc, "curl_multi_wait() failed");
#ifdef TRACE_DEBUG
                fprintf(stderr, "curl_multi_wait() failed, code %d.\n", mc);
#endif
                break;
                }

            /* 'numfds' being zero means either a timeout or no file descriptors to
            wait for. Try timeout on first occurrence, then assume no file
            descriptors and no file descriptors to wait for means wait for 100
            milliseconds. */

            if (!numfds)
                {
                repeats++; /* count number of repeated zero numfds */
                if (repeats > 1)
                    std::this_thread::sleep_for(std::chrono::milliseconds(300)); /* sleep 300 milliseconds */
                }
            else
                repeats = 0;

            CURLMsg *msg;
            int nbQueue;
            while ((msg = curl_multi_info_read(m_pToolHandle, &nbQueue)))
                {
                if (msg->msg == CURLMSG_DONE)
                    {
                    char *pClient;
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);
                    struct FileTransfer *pFileTrans = (struct FileTransfer *)pClient;

                    if (pFileTrans->fileStream.IsOpen())
                        pFileTrans->fileStream.Close();

                    if(pFileTrans->mirrors[0].cap != nullptr)
                        {
                        DownloadCap* cap = pFileTrans->mirrors[0].cap;
                        //bvector<downloadCap>::itterator it = findInCap(cap->sourceId, m_caps);
                        if(m_caps.find(cap->sourceId) != m_caps.end())
                            m_caps[cap->sourceId].currentDownloads --;
                        }

                    // Retry on error
                    if (msg->data.result == 56)     // Recv failure, try again
                        {
                        if (pFileTrans->nbRetry < s_MaxRetryTentative)
                            { 
                            ++pFileTrans->nbRetry;
                            pFileTrans->iAppend = 0;
                            ReportStatus((int)pFileTrans->index, pClient, REALITYDATADOWNLOAD_RETRY_TENTATIVE, "Trying again...");
#ifdef TRACE_DEBUG
                            fprintf(stderr, "R: %d - Retry(%d) <%ls>\n", REALITYDATADOWNLOAD_RETRY_TENTATIVE, pFileTrans->nbRetry, pFileTrans->filename.c_str());
#endif
                            SetupRequestandFile(&m_pEntries[pFileTrans->index], true);
                            still_running++;
                            }
                        else
                            {   
                            if(SetupMirror(pFileTrans->index, 56))
                                still_running++;
                            else
                                {
                                // Maximun retry done, return error.
                                ReportStatus((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                                }
                            }
                        }
                    else if (msg->data.result == CURLE_OK)
                        {
                        ReportStatus((int) pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
#ifdef TRACE_DEBUG
                        fprintf(stderr, "R: %d - %s <%ls>\n", msg->data.result, curl_easy_strerror(msg->data.result), pFileTrans->filename.c_str());
#endif

                        FileTransfer* nextSister = pFileTrans->mirrors[0].nextSister;
                        if(nextSister != nullptr)
                            {
                            if (m_pStatusFunc)
                                m_pStatusFunc((int) pFileTrans->index, nextSister, REALITYDATADOWNLOAD_CONTACTING_SISTER, "File retrieved successfully, attempting to retrieve next sister");
                            if (SetupRequestandFile(nextSister) == SetupRequestStatus::Success)
                                {
                                still_running++;
                                }
                            }
                        }
                    else if (msg->data.result != CURLE_OK)
                        { 
                        if(SetupMirror(pFileTrans->index, msg->data.result)) //if there's an error, try next mirror
                            still_running++;

                        else
                            {
                            ReportStatus((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
#ifdef TRACE_DEBUG
                        fprintf(stderr, "R: %d - %s <%ls>\n", msg->data.result, curl_easy_strerror(msg->data.result), pFileTrans->filename.c_str());
#endif
                            }
                        }

                    curl_multi_remove_handle(m_pToolHandle, msg->easy_handle);
                    curl_easy_cleanup(msg->easy_handle);
                    }
                else
                    {
                    ReportStatus(-1, NULL, msg->msg, "CurlMsg failed");
#ifdef TRACE_DEBUG
                        fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
#endif
                    }

                if (still_running < MAX_NB_CONNECTIONS)
                    {
                    // Other URL to download ?
                    if(m_waitingList.size() > 0)
                        {
                        bpair<Utf8String, FileTransfer*> waiter;
                        for (size_t i = 0; i < m_waitingList.size(); i++)
                            {
                            waiter = m_waitingList[i];
                            //bvector<downloadCap>::itterator it = findInCap(waiter->first, m_caps);
                            if ((m_caps.find(waiter.first) != m_caps.end()) && (m_caps[waiter.first].currentDownloads < m_caps[waiter.first].concurrentDownloadCap))
                                {
                                SetupRequestandFile(waiter.second, false);
                                still_running++;
                                m_waitingList.erase(m_waitingList.begin() + i);
                                break;
                                }
                            }
                        }  
                    else if (m_curEntry < m_nbEntry)
                        {
                        if (SetupNextEntry())
                            still_running++;
                        }
                    }
                }

            } while (still_running);
        }
    else
        {
        delete m_dlReport;
        m_dlReport = nullptr;
        }
    DownloadFromRDS();

    return m_dlReport;
    }

void RealityDataDownload::SetProxy(CURL* pCurl, Utf8StringCR proxyUrl, Utf8StringCR proxyCreds)
    {
    curl_easy_setopt(pCurl, CURLOPT_PROXY, proxyUrl.c_str());
    curl_easy_setopt(pCurl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    if (!proxyCreds.empty())
        {
        curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, proxyCreds.c_str());
        }
    }

SetupRequestStatus RealityDataDownload::Login(LoginInfo& loginInfo, AuthInfo& authInfo)
    {
    RawServerResponse response = RawServerResponse();
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        response.toolCode = CURLcode::CURLE_FAILED_INIT;
        return SetupRequestStatus::Error;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;
    if (loginInfo.postBody.length() > 0)
        {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, loginInfo.postBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, loginInfo.postBody.length());
        }
    
    if (!m_proxyUrl.empty())
        SetProxy(curl, m_proxyUrl, m_proxyCreds);

    for (Utf8String header : loginInfo.headers)
        headers = curl_slist_append(headers, header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, loginInfo.loginUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

    curl_easy_setopt(curl, CURLOPT_CAINFO, Utf8String(m_certPath).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    //capture full response for debugging
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(response.header));

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response.body));

    if (response.toolCode == CURLcode::CURLE_FAILED_INIT)
        return SetupRequestStatus::Error;

    response.toolCode = (int)curl_easy_perform(curl);
    if(!response.toolCode)
        {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(response.responseCode));
        struct curl_slist *cookies = NULL;
        response.toolCode = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
        if(!response.toolCode && cookies)
            {
            while(cookies)
                {
                authInfo.cookie.append(cookies->data);
                cookies = cookies->next;
                }
            curl_slist_free_all(cookies);
            }
        }
    curl_easy_cleanup(curl);

    if (response.toolCode != CURLE_OK)
        return SetupRequestStatus::Error;
    else
        return SetupRequestStatus::Success;
    }

//
// false --> Curl error or file already there, skip download.
//
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

    CURL *pCurl = NULL;

    pCurl = curl_easy_init();
    Mirror_struct& currentMirror = ft->mirrors.front();

    DownloadCap* currentCap = nullptr;

    if (currentMirror.cap != nullptr)
        {
        Utf8String& capId = currentMirror.cap->sourceId;
        //auto it = std::find_if(m_caps.begin(), m_caps.end(), [&capId] (const downloadCap& obj) {return obj.sourceId == capId;})
        //auto it = findInCap(capId, m_caps);
        if(m_caps.find(capId) != m_caps.end())
            {
            if (m_caps[capId].currentDownloads < m_caps[capId].concurrentDownloadCap)
                m_caps[capId].currentDownloads++;
            else
                {
                m_waitingList.push_back(make_bpair(capId, ft));
                return SetupRequestStatus::Success;
                }
            }
        else
            {
            currentMirror.cap->currentDownloads = 1;
            if(currentMirror.cap->login != nullptr)
                {
                if(Login(*(currentMirror.cap->login), currentMirror.cap->auth) != SetupRequestStatus::Success)
                    return SetupRequestStatus::Error;
                }
            m_caps[capId] = *(currentMirror.cap);
            }
        currentCap = &m_caps[capId];
        }

    if (pCurl)
        {
        Utf8String header = "";
        if(m_pTokenFunc != nullptr && !currentMirror.tokenType.empty())
            {
            m_pTokenFunc(currentMirror.tokenType, currentMirror.url, header);
            }

        curl_easy_setopt(pCurl, CURLOPT_URL, currentMirror.url.c_str());
        if (!WString::IsNullOrEmpty(m_certPath.c_str()))
            {
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1);
            curl_easy_setopt(pCurl, CURLOPT_CAINFO, Utf8String(m_certPath).c_str());
            }

        struct curl_slist *headers = NULL;
        if(!header.empty())
            headers = curl_slist_append(headers, header.c_str());
        if(currentCap != nullptr)
            {
            for(Utf8String header : currentCap->auth.headers)
                headers = curl_slist_append(headers, header.c_str());
            }
        if(headers != NULL)
            {
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(pCurl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
            }
        else
            curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);

        if(currentCap != nullptr && currentCap->auth.cookie.length() > 0)
            curl_easy_setopt(pCurl, CURLOPT_COOKIE, currentCap->auth.cookie.c_str());
        if(currentCap != nullptr && currentCap->auth.postBody.length() > 0)
            {
            curl_easy_setopt(pCurl, CURLOPT_POST, 1);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, currentCap->auth.postBody.c_str());
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, currentCap->auth.postBody.length());
            }

        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
        if (m_pProxyFunc != nullptr)
            {
            Utf8String proxyUrl, proxyCreds;
            ProxyStatus proxyResult = m_pProxyFunc(currentMirror.url, proxyUrl, proxyCreds);
            switch(proxyResult)
                {
            case ProxyStatus::ReturnedProxy:
                    {
                    SetProxy(pCurl, proxyUrl, proxyCreds);
                    break;
                    }
                case ProxyStatus::DefaultProxy:
                    {
                    SetProxy(pCurl, m_proxyUrl, m_proxyCreds);
                    break;
                    }
                case ProxyStatus::NoProxy:
                    break;
                default: //ProxyStatus::Abort
                    return SetupRequestStatus::Error;
                }
            }
        else if (!m_proxyUrl.empty())
            SetProxy(pCurl, m_proxyUrl, m_proxyCreds);

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, m_timeout);
        curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 80L);

        /* Define our callback to get called when there's data to be written */
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, callback_fwrite_func);
        /* Set a pointer to our struct to pass to the callback */
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, ft);
        /* Switch on full protocol/debug output set 1L*/
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);

        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, callback_progress_func);
        curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, ft);

        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, ft);

        ft->pProgressFunc = m_pProgressFunc;
        ft->pHeartbeatFunc = m_pHeartbeatFunc;
        ft->progressStep = m_progressStep;

        ft->mirrors[0].DownloadStart = std::time(nullptr);

        curl_multi_add_handle((CURLM*)m_pToolHandle, pCurl);
        }
        
    if (pCurl != NULL)
        {
        if(!isRetry)
            {
            TransferReport* tr = new TransferReport();
            tr->url = ft->mirrors[0].url;
            m_dlReport->results.Insert(ft->filename, tr);
            }
        return SetupRequestStatus::Success;
        }
    else
        return SetupRequestStatus::Error;
    }