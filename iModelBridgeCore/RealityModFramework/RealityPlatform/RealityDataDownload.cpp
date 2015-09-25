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
        (pFileTrans->pProgressFunc)((int)pFileTrans->index, pClient, (size_t)dlnow, (size_t)dltotal);

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
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, 10);

    for (m_curEntry = 0; m_curEntry < m_nbEntry; ++m_curEntry)
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
                Sleep(100); /* sleep 100 milliseconds */
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

                m_pStatusFunc((int)pFileTrans->index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
//                fprintf(stderr, "R: %d - %s <%ls>\n", msg->data.result, curl_easy_strerror(msg->data.result), pFileTrans->filename.c_str());

                if (pFileTrans->fileStream.IsOpen())
                    pFileTrans->fileStream.Close();

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
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

        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 120L);
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