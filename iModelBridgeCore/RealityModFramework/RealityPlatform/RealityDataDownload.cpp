/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataDownload.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>

#include <RealityPlatform/RealityDataDownload.h>

#include <curl/curl.h>

//#define TRACE_DEBUG 1

#define MAX_NB_CONNECTIONS          10
#define DEFAULT_STEP_PROGRESSCALL   (64*1024)      // default step if filesize is absent.

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

    if (NULL != pFileTrans->pProgressFunc)
        {
        if (dltotal > 0)
            {
            if (pFileTrans->filesize == 0)
                {
                pFileTrans->filesize = (size_t)dltotal;
                pFileTrans->downloadedSizeStep = (size_t)(pFileTrans->filesize * pFileTrans->progressStep);
                }
            }

        if (dlnow > pFileTrans->downloadedSizeStep)
            {
            if (pFileTrans->filesize == 0)
                pFileTrans->downloadedSizeStep += pFileTrans->downloadedSizeStep;       // predefine step
            else
                pFileTrans->downloadedSizeStep += (size_t)(pFileTrans->filesize * pFileTrans->progressStep);

            int statusCode = (pFileTrans->pProgressFunc)((int) pFileTrans->index, pClient, (size_t) dlnow, pFileTrans->filesize);
            if (0 != statusCode)
                {
                // An error occurred, delete incomplete file.
                pFileTrans->fileStream.Close();
                BeFileName::BeDeleteFile(pFileTrans->filename.c_str());
                }
                

            return statusCode;
            }
        }

#ifdef TRACE_DEBUG
    fprintf(stderr, "callback_progress_func total:%llu now: %llu\n", (size_t)dltotal, (size_t)dlnow);
#endif

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

RealityDataDownload::RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName)
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    m_pProgressFunc = nullptr;
    m_pStatusFunc = nullptr;
    m_certPath = WString();

    m_curEntry = 0;
    m_nbEntry = pi_Link_FileName.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i=0; i<m_nbEntry; ++i)
        {
        m_pEntries[i].urls = bvector<AString>();
        m_pEntries[i].urls.push_back(pi_Link_FileName[i].first);
        m_pEntries[i].filename = pi_Link_FileName[i].second;
        m_pEntries[i].iAppend = 0;
        m_pEntries[i].nbRetry = 0;

        m_pEntries[i].downloadedSizeStep = DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
        m_pEntries[i].filesize = 0;
        m_pEntries[i].fromCache = true;                                 // from cache if possible by default
        m_pEntries[i].progressStep = 0.01;
        }
    }

RealityDataDownload::RealityDataDownload(const Link_File_wMirrors& pi_Link_File_wMirrors)
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    m_pProgressFunc = nullptr;
    m_pStatusFunc = nullptr;
    m_certPath = WString();

    m_curEntry = 0;
    m_nbEntry = pi_Link_File_wMirrors.size();
    size_t mirrorCount;
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i = 0; i<m_nbEntry; ++i)
        {
        mirrorCount = pi_Link_File_wMirrors[i].first.size();

        for(size_t j = 0; j < mirrorCount; ++j)
            {
            m_pEntries[i].urls = pi_Link_File_wMirrors[i].first;
            m_pEntries[i].filename = pi_Link_File_wMirrors[i].second;
            m_pEntries[i].iAppend = 0;
            m_pEntries[i].nbRetry = 0;

            m_pEntries[i].downloadedSizeStep = DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
            m_pEntries[i].filesize = 0;
            m_pEntries[i].fromCache = true;                                 // from cache if possible by default
            m_pEntries[i].progressStep = 0.01;
            }
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



bool RealityDataDownload::Perform()
    {

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
        return true;

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
            if (m_pStatusFunc)
                m_pStatusFunc(-1, NULL, mc, "curl_multi_wait() failed");
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
                        if (m_pStatusFunc)            // Send status retry ? Application should know or not ?
                            m_pStatusFunc((int)pFileTrans->index, pClient, REALITYDATADOWNLOAD_RETRY_TENTATIVE, "Trying again...");
#ifdef TRACE_DEBUG
                        fprintf(stderr, "R: %d - Retry(%d) <%ls>\n", REALITYDATADOWNLOAD_RETRY_TENTATIVE, pFileTrans->nbRetry, pFileTrans->filename.c_str());
#endif
                        SetupCurlandFile(pFileTrans->index);
                        still_running++;
                        }
                    else
                        {   
                        if(SetupMirror(pFileTrans->index, 56))
                            {
                            pFileTrans->nbRetry = 0;
                            still_running++;
                            }
                        else
                            {
                                // Maximun retry done, return error.
                            if (m_pStatusFunc)
                                m_pStatusFunc((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                            }
                        }
                    }
                else if ((msg->data.result != CURLE_OK) && SetupMirror(pFileTrans->index, msg->data.result)) //if there's an error, try next mirror
                    {
                    pFileTrans->nbRetry = 0;
                    still_running++;
                    }
                else
                    {
                    if (m_pStatusFunc)
                        m_pStatusFunc((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
#ifdef TRACE_DEBUG
                    fprintf(stderr, "R: %d - %s <%ls>\n", msg->data.result, curl_easy_strerror(msg->data.result), pFileTrans->filename.c_str());
#endif
                    }

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                if (m_pStatusFunc)
                    m_pStatusFunc(-1, NULL, msg->msg, "CurlMsg failed");
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

    return true;
    }
    

//
// false --> Curl error or file already there, skip download.
//
bool RealityDataDownload::SetupCurlandFile(size_t pi_index)
    {
    // If file already there, consider the download completed.
    if (m_pEntries[pi_index].fromCache && BeFileName::DoesPathExist(m_pEntries[pi_index].filename.c_str()))
        return true;

    // File not in the cache, we will download it
    m_pEntries[pi_index].fromCache = false;

    CURL *pCurl = NULL;

    pCurl = curl_easy_init();
    if (pCurl)
        {
        curl_easy_setopt(pCurl, CURLOPT_URL, m_pEntries[pi_index].urls.front());
        if (!WString::IsNullOrEmpty(m_certPath.c_str()))
            {
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1);
            curl_easy_setopt(pCurl, CURLOPT_CAINFO, Utf8String(m_certPath));
            }
        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
        if (!m_proxyUrl.empty())
            {
            curl_easy_setopt(pCurl, CURLOPT_PROXY, m_proxyUrl.c_str());
            curl_easy_setopt(pCurl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            if (!m_proxyCreds.empty())
                {
                curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, m_proxyCreds.c_str());
                }
            }
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
        curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 80L);

        /* Define our callback to get called when there's data to be written */
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, callback_fwrite_func);
        /* Set a pointer to our struct to pass to the callback */
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &(m_pEntries[pi_index]));
        /* Switch on full protocol/debug output set 1L*/
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);

        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, callback_progress_func);
        curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, &(m_pEntries[pi_index]));

        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, &(m_pEntries[pi_index]));

        m_pEntries[pi_index].index = pi_index;
        m_pEntries[pi_index].pProgressFunc = m_pProgressFunc;
        m_pEntries[pi_index].progressStep = m_progressStep;

        curl_multi_add_handle((CURLM*)m_pCurlHandle, pCurl);
        }
        
    return (pCurl != NULL);
    }

bool RealityDataDownload::SetupMirror(size_t index, int errorCode)
{
    if(m_pEntries[index].urls.size() <= 1)
        return false;

    if(m_pStatusFunc)
        {
        char errorMsg[512];
        sprintf(errorMsg, "could not download %s, error code %d. Attempting to contact mirror %s",
            m_pEntries[index].urls[0].c_str(),
            errorCode,
            m_pEntries[index].urls[1].c_str());
        m_pStatusFunc((int)m_pEntries[index].index, &(m_pEntries[index]), REALITYDATADOWNLOAD_RETRY_TENTATIVE, errorMsg);
        }

    m_pEntries[index].urls.erase(m_pEntries[index].urls.begin());
    SetupCurlandFile(index);
    return true;
}

bool RealityDataDownload::SetupNextEntry()
    {
    do 
        {
        if (m_curEntry < m_nbEntry)
            {
            SetupCurlandFile(m_curEntry);
            if (m_pEntries[m_curEntry].fromCache)
                {
                if (m_pStatusFunc)
                    m_pStatusFunc((int)m_curEntry, &(m_pEntries[m_curEntry]), 0, nullptr);
                }
            ++m_curEntry;
            }
        else
            return false;

        } while (m_pEntries[m_curEntry - 1].fromCache);

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


#if defined (_WIN32)

#include <Objbase.h>
#include <Shldisp.h>
#include <Shellapi.h>

bool RealityDataDownload::UnZipFile(WString& pi_strSrc, WString& pi_strDest)
{
    BSTR lpZipFile = ::SysAllocString(pi_strSrc.c_str());
    BSTR lpFolder = ::SysAllocString(pi_strDest.c_str());

    IShellDispatch *pISD;

    Folder  *pZippedFile = 0L;
    Folder  *pDestination = 0L;

    long FilesCount = 0;
    IDispatch* pItem = 0L;
    FolderItems *pFilesInside = 0L;

    VARIANT Options, OutFolder, InZipFile, Item;
    HRESULT res = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (res == S_OK || res == S_FALSE || res == RPC_E_CHANGED_MODE)
        {
        res = S_OK;
        __try{
            if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **) &pISD) != S_OK)
                {
                BeFileName::BeDeleteFile(pi_strSrc.c_str());
                return false;
                }
                

            InZipFile.vt = VT_BSTR;
            InZipFile.bstrVal = lpZipFile;
            pISD->NameSpace(InZipFile, &pZippedFile);
            if (!pZippedFile)
                {
                pISD->Release();
                BeFileName::BeDeleteFile(pi_strSrc.c_str());
                return false;
                }

            OutFolder.vt = VT_BSTR;
            OutFolder.bstrVal = lpFolder;
            pISD->NameSpace(OutFolder, &pDestination);
            if (!pDestination)
                {
                pZippedFile->Release();
                pISD->Release();
                BeFileName::BeDeleteFile(pi_strSrc.c_str());
                return false;
                }

            pZippedFile->Items(&pFilesInside);
            if (!pFilesInside)
                {
                pDestination->Release();
                pZippedFile->Release();
                pISD->Release();
                BeFileName::BeDeleteFile(pi_strSrc.c_str());
                return false;
                }

            pFilesInside->get_Count(&FilesCount);
            if (FilesCount < 1)
                {
                pFilesInside->Release();
                pDestination->Release();
                pZippedFile->Release();
                pISD->Release();
                BeFileName::BeDeleteFile(pi_strSrc.c_str());
                return false;
                }

            pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);

            Item.vt = VT_DISPATCH;
            Item.pdispVal = pItem;

            Options.vt = VT_I4;
            Options.lVal = 1024 | 512 | 16 | 4;//http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

            bool retval = pDestination->CopyHere(Item, Options) == S_OK;

            pItem->Release(); pItem = 0L;
            pFilesInside->Release(); pFilesInside = 0L;
            pDestination->Release(); pDestination = 0L;
            pZippedFile->Release(); pZippedFile = 0L;
            pISD->Release(); pISD = 0L;

            if (!retval)
                BeFileName::BeDeleteFile(pi_strSrc.c_str());

            return retval;
            }
        __finally
            {
            if (res = S_OK)
                CoUninitialize();
            }
        }
    BeFileName::BeDeleteFile(pi_strSrc.c_str());
    return false;
    }

#else
#error "Windows specific code function: void UnZipFile(CString strSrc, CString strDest)"
#endif