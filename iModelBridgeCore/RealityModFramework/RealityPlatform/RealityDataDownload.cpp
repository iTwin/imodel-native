/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataDownload.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>

#include <RealityPlatform/RealityDataDownload.h>

#include <curl/curl.h>
//#include <zlib/zlib.h>
#include <zlib/zip/unzip.h>


//#define TRACE_DEBUG 1

#define MAX_NB_CONNECTIONS          10
#define DEFAULT_STEP_PROGRESSCALL   (64*1024)      // default step if filesize is absent.
#define MAX_FILENAME                512
#define READ_SIZE                   8192

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
    double dltotal,
    double dlnow,
    double ultotal,
    double ulnow)
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

        if (dlnow > pFileTrans->downloadedSizeStep)
            {
            if (pFileTrans->filesize == 0)
                pFileTrans->downloadedSizeStep += pFileTrans->downloadedSizeStep;       // predefine step
            else
                pFileTrans->downloadedSizeStep += (size_t)(pFileTrans->filesize * pFileTrans->progressStep);

            
            if (NULL != pFileTrans->pProgressFunc)
                statusCode = (pFileTrans->pProgressFunc)((int) pFileTrans->index, pClient, (size_t) dlnow, pFileTrans->filesize);
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


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const UrlLink_UrlFile& pi_Link_FileName)
    {
    if (pi_Link_FileName.size() > 0)
        return new RealityDataDownload(pi_Link_FileName);
    else
        return NULL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Spencer.Mason      9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const Link_File_wMirrors& pi_Link_File_wMirrors)
    {
    if (pi_Link_File_wMirrors.size() > 0)
        return new RealityDataDownload(pi_Link_File_wMirrors);
    else
        return NULL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Spencer.Mason      9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters)
{
    if (pi_Link_File_wMirrors_wSisters.size() > 0)
        return new RealityDataDownload(pi_Link_File_wMirrors_wSisters);
    else
        return NULL;
}

RealityDataDownload::RealityDataDownload() : m_pProgressFunc(nullptr), m_pHeartbeatFunc(nullptr), m_pStatusFunc(nullptr),
    m_pProxyFunc(nullptr), m_pTokenFunc(nullptr), m_certPath(WString()), m_curEntry(0)
    {
    m_pCurlHandle = NULL;
    }

void RealityDataDownload::InitEntry(size_t i)
    {
    m_pEntries[i].iAppend = 0;
        m_pEntries[i].nbRetry = 0;
        m_pEntries[i].index = i;

        m_pEntries[i].downloadedSizeStep = DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
        m_pEntries[i].filesize = 0;
        m_pEntries[i].fromCache = true;                                 // from cache if possible by default
        m_pEntries[i].progressStep = 0.01;
    }

RealityDataDownload::RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName) : RealityDataDownload()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    
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
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    
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

void RealityDataDownload::AddSisterFiles(FileTransfer* ft, bvector<url_file_pair> sisters, size_t index, size_t sisterCount, size_t sisterIndex)
    {
    FileTransfer* sisFT = new FileTransfer(ft);
    Mirror_struct ms;
    ms.url = sisters[index].m_url;
    sisFT->filename = ms.filename = sisters[index].m_filePath;
    ms.tokenType = sisters[index].m_tokenType;
    ms.nextSister = nullptr;
    ms.totalSisters = sisterCount;
    ms.sisterIndex = sisterIndex;
    sisFT->mirrors.clear();
    sisFT->mirrors.push_back(ms);

    ft->mirrors.back().nextSister = sisFT;
    index++;
    if (index >= sisters.size())
        return;
    else
        AddSisterFiles(sisFT, sisters, index, sisterCount, sisterIndex + 1);
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters) :
    RealityDataDownload()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    
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
    if (m_pCurlHandle != NULL)
        curl_multi_cleanup(m_pCurlHandle);
    curl_global_cleanup();

    if (m_pEntries)
        delete[] m_pEntries;
    };

RealityDataDownload::DownloadReport* RealityDataDownload::Perform()
    {
    m_dlReport = DownloadReport();
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    m_curEntry = 0;
    bool atLeast1Download = false;
    for (size_t i = 0; i < min(MAX_NB_CONNECTIONS, m_nbEntry); ++i)
        {
        if (SetupNextEntry())
            atLeast1Download = true;
        else
            break;
        }

    // if everything already downloaded, exit with success.
    if (atLeast1Download == false)
        return nullptr;

    int still_running; /* keep number of running handles */
    int repeats = 0;

    do
        {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        curl_multi_perform(m_pCurlHandle, &still_running);

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(m_pCurlHandle, NULL, 0, 1000, &numfds);

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
                Sleep(300); /* sleep 100 milliseconds */
            }
        else
            repeats = 0;

        CURLMsg *msg;
        int nbQueue;
        while ((msg = curl_multi_info_read(m_pCurlHandle, &nbQueue)))
            {
            if (msg->msg == CURLMSG_DONE)
                {
                char *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);
                struct FileTransfer *pFileTrans = (struct FileTransfer *)pClient;

                if (pFileTrans->fileStream.IsOpen())
                    pFileTrans->fileStream.Close();

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
                        SetupCurlandFile(&m_pEntries[pFileTrans->index], true);
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
                        if (SetupCurlandFile(nextSister) == SetupCurlStatus::Success)
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

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                ReportStatus(-1, NULL, msg->msg, "CurlMsg failed");
#ifdef TRACE_DEBUG
                    fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
#endif
                }

            // Other URL to download ?
            if (m_curEntry < m_nbEntry)
                {
                if (SetupNextEntry())
                    still_running++;
                }
            }

        } while (still_running);

    return &m_dlReport;
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

//
// false --> Curl error or file already there, skip download.
//
SetupCurlStatus RealityDataDownload::SetupCurlandFile(FileTransfer* ft, bool isRetry)
    {
    // If cancel requested, don't queue new files
    if(NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return SetupCurlStatus::Success;

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
        return SetupCurlStatus::FromCache;

    // File not in the cache, we will download it
    ft->fromCache = false;

    CURL *pCurl = NULL;

    pCurl = curl_easy_init();
    Mirror_struct& currentMirror = ft->mirrors.front();

    if (pCurl)
        {
        Utf8String header = "";
        if(m_pTokenFunc != nullptr && !currentMirror.tokenType.empty())
            {
            m_pTokenFunc(currentMirror.tokenType, currentMirror.url, header);
            }

        curl_easy_setopt(pCurl, CURLOPT_URL, currentMirror.url);
        if (!WString::IsNullOrEmpty(m_certPath.c_str()))
            {
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1);
            curl_easy_setopt(pCurl, CURLOPT_CAINFO, Utf8String(m_certPath));
            }

        if(!header.empty())
            {
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, header.c_str());
            curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(pCurl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
            }
        else
            curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);

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
                    return SetupCurlStatus::Error;
                }
            }
        else if (!m_proxyUrl.empty())
            SetProxy(pCurl, m_proxyUrl, m_proxyCreds);

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
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

        curl_multi_add_handle((CURLM*)m_pCurlHandle, pCurl);
        }
        
    if (pCurl != NULL)
        {
        if(!isRetry)
            {
            TransferReport* tr = new TransferReport();
            tr->url = ft->mirrors[0].url;
            m_dlReport.results.Insert(ft->filename, tr);
            }
        return SetupCurlStatus::Success;
        }
    else
        return SetupCurlStatus::Error;
    }

bool RealityDataDownload::SetupMirror(size_t index, int errorCode)
{
    if(m_pEntries[index].mirrors.size() <= 1)
        return false;

    char errorMsg[512];
    sprintf(errorMsg, "could not download %ls, error code %d. Attempting to retrieve mirror file %ls",
        m_pEntries[index].mirrors[0].filename.c_str(),
        errorCode,
        m_pEntries[index].mirrors[1].filename.c_str());
    ReportStatus((int)m_pEntries[index].index, &(m_pEntries[index]), REALITYDATADOWNLOAD_MIRROR_CHANGE, errorMsg);
        
    m_pEntries[index].mirrors.erase(m_pEntries[index].mirrors.begin());
    m_pEntries[index].filename = m_pEntries[index].mirrors.front().filename;
    m_pEntries[index].iAppend = 0;
    m_pEntries[index].nbRetry = 0;

    m_pEntries[index].downloadedSizeStep = DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
    m_pEntries[index].filesize = 0;
    m_pEntries[index].fromCache = true;                                 // from cache if possible by default
    m_pEntries[index].progressStep = 0.01;
    SetupCurlandFile(&m_pEntries[index]);
    return true;
}

bool RealityDataDownload::SetupNextEntry()
    {
    SetupCurlStatus status;
    do 
        {
        if (m_curEntry < m_nbEntry)
            {
            status = SetupCurlandFile(&m_pEntries[m_curEntry]);
            ++m_curEntry;
            }
        else
            return false;

        } while (SetupCurlStatus::FromCache == status);

    return true;
    }   


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
void RealityDataDownload::ExtractFileName(WString& pio_rFileName, const AString& pi_Url)
    {
    // Extract filename form URL, the last part of the URL until a '/'or '\' or '='
    WString urlW(pi_Url.c_str(), BentleyCharEncoding::Utf8);
    urlW.ReplaceAll(WCSALT_DIR_SEPARATOR, WCSDIR_SEPARATOR);
    bvector<WString> pathComponents;
    WString delim = WCSDIR_SEPARATOR;
    delim += L"=";
    BeStringUtilities::Split(urlW.c_str(), delim.c_str(), NULL, pathComponents);

    BeFileName::BuildName(pio_rFileName, NULL, pio_rFileName.c_str(), pathComponents[pathComponents.size() - 1].c_str(), NULL);

    }

bool RealityDataDownload::UnZipFile(WString& pi_strSrc, WString& pi_strDest)
    {
    char src[MAX_FILENAME];
    char dest[MAX_FILENAME];

    wcstombs (src, pi_strSrc.c_str(), pi_strSrc.size());
    wcstombs (dest, pi_strDest.c_str(), pi_strDest.size());

    src[pi_strSrc.size()] = 0;
    dest[pi_strDest.size()] = 0;

    return UnZipFile(src, dest);
    }

bool RealityDataDownload::UnZipFile(const char* pi_strSrc, const char* pi_strDest)
    {
    if(!strstr(pi_strSrc, ".zip"))
        return false;
    
    unzFile uf = unzOpen(pi_strSrc);
    if(nullptr == uf)
        return false;

    unz_global_info unzGlobalInfo;
    if(unzGetGlobalInfo (uf, &unzGlobalInfo) != 0)
        return false;

    uLong i;
    char read_buffer[ READ_SIZE ];
    for( i = 0; i < unzGlobalInfo.number_entry; ++i )
        {
        unz_file_info file_info;
        char filename[MAX_FILENAME];
        if ( unzGetCurrentFileInfo(
            uf,
            &file_info,
            filename,
            MAX_FILENAME,
            NULL, 0, NULL, 0 ) != UNZ_OK )
            return false;
        
        char fullpath[MAX_FILENAME];
        sprintf(fullpath, "%s%s", pi_strDest,filename);

        const size_t fullpath_length = strlen(fullpath);
        WString pathString(fullpath, BentleyCharEncoding::Utf8);
        if(!pathString.EndsWithI(L"/")) //a file
            {
            if(unzOpenCurrentFile( uf ) != UNZ_OK)
                return false;

            FILE *out = fopen( fullpath, "wb" );
            if ( out == NULL)
                return false;

            int status = UNZ_OK;
            do
                {
                status = unzReadCurrentFile( uf, read_buffer, READ_SIZE );
                if(status < 0)
                    return false;
                if(status > 0)
                    fwrite( read_buffer, status, 1, out );
                } while (status > 0 );
            fclose( out );
            // Set the date to original file
            DateTime fileTime(DateTime::Kind::Local, (uint16_t)file_info.tmu_date.tm_year, (uint8_t)(file_info.tmu_date.tm_mon + 1), (uint8_t)file_info.tmu_date.tm_mday, (uint8_t)file_info.tmu_date.tm_hour, (uint8_t)file_info.tmu_date.tm_min, (uint8_t)file_info.tmu_date.tm_sec);
            time_t fileModifTime = (time_t)(file_info.dosDate);
            fileTime.ToUnixMilliseconds(fileModifTime);
            fileModifTime /= 1000;

            BeFileName::SetFileTime(WString(fullpath, true).c_str(), &fileModifTime, &fileModifTime);
            }
        else // a folder
            {
            if(!BeFileName::DoesPathExist(pathString.c_str()) && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(pathString.c_str()))
                return false;
            }
        unzCloseCurrentFile( uf );
    
        if( ( i+1 ) < unzGlobalInfo.number_entry )
            {
            if ( unzGoToNextFile( uf ) != UNZ_OK)
                return false;
            }
        }
    
    return (unzClose (uf) == 0);
    }

void RealityDataDownload::ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(m_pStatusFunc)
        m_pStatusFunc(index, pClient, ErrorCode, pMsg);

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
        
    bmap<WString, TransferReport*>::iterator it = m_dlReport.results.find(pEntry->filename);
    if(it == m_dlReport.results.end())
        return;//something went wrong

    TransferReport* tr = it->second;
    tr->filesize = pEntry->filesize;

    DownloadResult dr = DownloadResult();
    dr.errorCode = ErrorCode;
    dr.downloadProgress = pEntry->downloadedSizeStep;
    if(pEntry->filesize != 0)
        dr.downloadProgress /= pEntry->filesize;

    tr->retries.push_back(dr);

    if (ErrorCode != REALITYDATADOWNLOAD_RETRY_TENTATIVE)
        {
        tr->timeSpent = time(nullptr) - pEntry->mirrors[0].DownloadStart;
        }
    }