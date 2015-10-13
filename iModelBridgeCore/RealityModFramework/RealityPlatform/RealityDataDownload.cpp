/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataDownload.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/RefCounted.h>

#include <RealityPlatform/RealityDataDownload.h>

#include <stdio.h>
#include <curl/curl.h>

#define MAX_RETRY_ON_ERROR  25
#define MAX_NB_CONNECTIONS  10

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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
                pFileTrans->downloadedSizeStep = (size_t)(dltotal * pFileTrans->progressStep);
                }
            }

        if (dlnow > pFileTrans->downloadedSizeStep)
            {
            pFileTrans->downloadedSizeStep += (size_t)(dltotal * pFileTrans->progressStep);

            (pFileTrans->pProgressFunc)((int)pFileTrans->index, pClient, (size_t)dlnow, (size_t)dltotal);
            }
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

RealityDataDownload::RealityDataDownload(const UrlLink_UrlFile& pi_Link_FileName)
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    m_pProgressFunc = NULL;
    m_pStatusFunc   = NULL;

    m_nbEntry = pi_Link_FileName.size();
    m_pEntries = new FileTransfer[m_nbEntry];
    for (size_t i=0; i<m_nbEntry; ++i)
        {
        m_pEntries[i].url = pi_Link_FileName[i].first;
        m_pEntries[i].filename = pi_Link_FileName[i].second;
        m_pEntries[i].iAppend = 0;
        m_pEntries[i].nbRetry = 0;

        m_pEntries[i].downloadedSizeStep = 0;
        m_pEntries[i].filesize = 0;
        m_pEntries[i].progressStep = 0.01;
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

        for (m_curEntry = 0; m_curEntry < min(MAX_NB_CONNECTIONS, m_nbEntry); ++m_curEntry)
        SetupCurlandFile(m_curEntry);

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
//              fprintf(stderr, "curl_multi_wait() failed, code %d.\n", mc);
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
                    if (pFileTrans->nbRetry < MAX_RETRY_ON_ERROR)
                        { 
                        ++pFileTrans->nbRetry;
//                        pFileTrans->iAppend = 0;
//                        if (m_pStatusFunc)            // Send status retry ? Application should know or not ?
//                            m_pStatusFunc((int)pFileTrans->index, pClient, -2, "Trying again...");
                        SetupCurlandFile(pFileTrans->index);
                        still_running++;
                        }
                    else
                        {   // Maximun retry done, return error.
                        if (m_pStatusFunc)
                            m_pStatusFunc((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        }
                    }
                else
                    {
                    if (m_pStatusFunc)
                        m_pStatusFunc((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
//                  fprintf(stderr, "R: %d - %s <%ls>\n", msg->data.result, curl_easy_strerror(msg->data.result), pFileTrans->filename.c_str());
                    }

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                if (m_pStatusFunc)
                    m_pStatusFunc(-1, NULL, msg->msg, "CurlMsg failed");
//                  fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
                }

            // Other URL to download ?
            if (m_curEntry < m_nbEntry)
                {
                SetupCurlandFile(m_curEntry);
                m_curEntry++;
                still_running++;
                }
            }

        } while (still_running);

    return true;
    }
    


bool RealityDataDownload::SetupCurlandFile(size_t pi_index)
{
    CURL *pCurl = NULL;

    pCurl = curl_easy_init();
    if (pCurl)
    {
        curl_easy_setopt(pCurl, CURLOPT_URL, m_pEntries[pi_index].url);

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L); //Timeout for the complete download.
        curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 80L);

        curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);


        /* Define our callback to get called when there's data to be written */
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, callback_fwrite_func);
        /* Set a pointer to our struct to pass to the callback */
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &(m_pEntries[pi_index]));

        curl_easy_setopt(pCurl, CURLOPT_FTPPORT, "-");

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
            if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD) != S_OK)
                return false;

            InZipFile.vt = VT_BSTR;
            InZipFile.bstrVal = lpZipFile;
            pISD->NameSpace(InZipFile, &pZippedFile);
            if (!pZippedFile)
                {
                pISD->Release();
                return false;
                }

            OutFolder.vt = VT_BSTR;
            OutFolder.bstrVal = lpFolder;
            pISD->NameSpace(OutFolder, &pDestination);
            if (!pDestination)
                {
                pZippedFile->Release();
                pISD->Release();
                return false;
                }

            pZippedFile->Items(&pFilesInside);
            if (!pFilesInside)
                {
                pDestination->Release();
                pZippedFile->Release();
                pISD->Release();
                return false;
                }

            pFilesInside->get_Count(&FilesCount);
            if (FilesCount < 1)
                {
                pFilesInside->Release();
                pDestination->Release();
                pZippedFile->Release();
                pISD->Release();
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

            return retval;
            }
        __finally
            {
            if (res = S_OK)
                CoUninitialize();
            }
        }
    return false;
    }

#else
#error "Windows specific code function: void UnZipFile(CString strSrc, CString strDest)"
#endif