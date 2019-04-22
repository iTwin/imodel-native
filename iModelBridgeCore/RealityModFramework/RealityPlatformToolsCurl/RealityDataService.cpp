/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>
#include "../RealityPlatformTools/RealityDataService.cpp"
#include <thread>
#include <chrono>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

static size_t DownloadWriteCallback(void *buffer, size_t size, size_t nmemb, void *pClient)
    {
    if (NULL == pClient)
        return 0;

    RealityDataFileDownload *fileDown = (RealityDataFileDownload *)pClient;
    if (fileDown != nullptr)
        return fileDown->OnWriteData((char*)buffer, size * nmemb);
    else
        return 0;
    }

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

static int download_progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
    if(dltotal < 1)
        return 0;

    RealityDataUrl *request = reinterpret_cast<RealityDataUrl*>(clientp);
    RealityDataFileDownload *fileDown = dynamic_cast<RealityDataFileDownload*>(request);

    if(fileDown != nullptr)
        {
        return fileDown->ProcessProgress((uint64_t)dlnow);
        }
    return 0;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! Callback methods used by the upload/download process.
//=====================================================================================
static size_t CurlReadDataCallback(char* buffer, size_t size, size_t count, RealityDataFileUpload* request)
    {
    return request->OnReadData(buffer, size * count);
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
const TransferReport& RealityDataServiceTransfer::Perform()
    {
    m_currentTransferedAmount = 0;
    m_progress = 0.0;

    m_report = TransferReport();

    if(!IsValidTransfer())
        {
        ReportStatus(0, nullptr, -1, "No files to transfer, please verify that the previous steps completed without failure");
        return m_report;
        }

    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pRequestHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    m_curEntry = 0;

    int min = (MAX_NB_CONNECTIONS < (int)m_filesToTransfer.size()) ? MAX_NB_CONNECTIONS : (int)m_filesToTransfer.size();

    for (int i = 0; i < min; ++i)
        {
        SetupNextEntry();
        }   

    int still_running; /* keep number of running handles */
    int repeats = 0;

    do {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        curl_multi_perform(m_pRequestHandle, &still_running);

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(m_pRequestHandle, NULL, 0, 1000, &numfds);

        if (mc != CURLM_OK)
            {
            ReportStatus(-1, NULL, mc, "curl_multi_wait() failed\n");
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
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        else
            repeats = 0;

        CURLMsg *msg;
        int nbQueue;
        while ((msg = curl_multi_info_read(m_pRequestHandle, &nbQueue)))
            {
            if (msg->msg == CURLMSG_DONE)
                {
                void *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);

                RealityDataUrl *request = reinterpret_cast<RealityDataUrl*>(pClient);
                RealityDataFileTransfer *fileTrans = dynamic_cast<RealityDataFileTransfer*>(request);
                RealityDataFileUpload *fileUp = dynamic_cast<RealityDataFileUpload*>(request);
                RealityDataFileDownload *fileDown = dynamic_cast<RealityDataFileDownload*>(request);

                // Retry on error
                if (msg->data.result == 56 || msg->data.result == 28)     // Recv failure, try again
                    {
                    if (fileTrans != nullptr && fileTrans->nbRetry < 5)
                        {
                        ++fileTrans->nbRetry;
                        fileTrans->Retry();
                        if(fileUp != nullptr)
                            UpdateTransferAmount((int64_t)fileTrans->GetTransferedSize() * -1);
                        SetupRequestforFile((RealityDataUrl*)(fileTrans), 0);
                        still_running++;
                        }
                    else
                        {
                        // Maximun retry done, return error.
                        ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        }
                    }
                else if(fileTrans != nullptr)
                    {
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &(fileTrans->GetResponse().responseCode));
                    fileTrans->GetResponse().toolCode = msg->data.result;
                    if(msg->data.result == CURLE_OK)
                        {
                        if(fileUp != nullptr && !fileUp->FinishedSending())
                            {
                            if(m_pProgressFunc && UpdateTransferAmount((int64_t)fileUp->GetMessageSize()))
                                m_pProgressFunc(fileUp->GetFilename(), ((double)fileUp->GetTransferedSize()) / fileUp->GetFileSize(), m_progress);
                            SetupRequestforFile(fileUp, 0);
                            still_running++;
                            }
                        else
                            {
                            if (m_pProgressFunc) 
                                {
                                if(fileUp!= nullptr) //download now has its own way of handling this
                                    {
                                    UpdateTransferAmount((int64_t)fileUp->GetMessageSize());
                                    m_pProgressFunc(fileTrans->GetFilename(), 1.0, m_progress);
                                    }
                                else if (fileDown != nullptr)
                                    {
                                    fileDown->ConfirmDownload();
                                    }
                                }
                            ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                            }  
                        }
                    else
                        {
                        ReportStatus((int)fileTrans->m_index, pClient, msg->data.result, curl_easy_strerror(msg->data.result));
                        ReportStatus(0, nullptr, -1, Utf8PrintfString("\nServer returned code %ld\n", fileTrans->GetResponse().responseCode).c_str());
                        }
                    }
                           
                    
                curl_multi_remove_handle(m_pRequestHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                ReportStatus(-1, NULL, msg->msg, "CurlMsg failed\n");
                }

            if (m_curEntry < (int)m_filesToTransfer.size() && still_running < MAX_NB_CONNECTIONS)
                {
                if (SetupNextEntry())
                    still_running++;
                }
            }

        } while (still_running);

    return m_report;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataServiceTransfer::SetupRequestforFile(RealityDataUrl* request, bool verifyPeer)
    {
    // If cancel requested, don't queue new files
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return;

    RealityDataFileTransfer* fileTransfer = dynamic_cast<RealityDataFileTransfer*>(request);
    if(fileTransfer != nullptr)
        {
        fileTransfer->SetAzureToken(GetAzureToken());
        //fileTransfer->UpdateTransferedSize();
        }
    else
        return; //unexpected request

    RealityDataFileUpload* fileUpload = dynamic_cast<RealityDataFileUpload*>(request);
    RealityDataFileDownload* fileDownload = dynamic_cast<RealityDataFileDownload*>(request);

    RawServerResponse& rawResponse = fileTransfer->GetResponse();
    rawResponse.clear();
    rawResponse.toolCode = ServerType::Azure;

    CURL *pCurl = PrepareRequestBase(*request, rawResponse, verifyPeer, nullptr);
    if (pCurl)
        {
        curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 0L);

        Utf8String proxyUrl, proxyCreds;
        GetCurrentProxyUrlAndCredentials(proxyUrl, proxyCreds);

        if (!proxyUrl.empty())
            {
            curl_easy_setopt(pCurl, CURLOPT_PROXY, proxyUrl.c_str());
            curl_easy_setopt(pCurl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            if (!proxyCreds.empty())
                {
                curl_easy_setopt(pCurl, CURLOPT_PROXYUSERPWD, proxyCreds.c_str());
                }
            }
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
        
        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, request);

        curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1L); // B/s
        curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 60); //60s

        curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, WriteCallback);
        curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, &(rawResponse.header));

        if(fileUpload != nullptr)
            {
            curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "PUT");

            curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, CurlReadDataCallback);
            curl_easy_setopt(pCurl, CURLOPT_READDATA, fileUpload);

            curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, fileUpload->GetMessageSize());
            fileUpload->StartTimer();
            }
        else if (fileDownload != nullptr)
            {
            /* Define our callback to get called when there's data to be written */
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, DownloadWriteCallback);
            /* Set a pointer to our struct to pass to the callback */
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fileDownload);

            curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, download_progress_callback);
            curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, fileDownload);

            /*if (fileDownload->GetFileStream().Create(fileDownload->GetFilename().c_str(), true) != BeFileStatus::Success)
                {
                ReportStatus(0, nullptr, -1, Utf8PrintfString("\nFailed to create File %s on local machine\nAborting download of this file", fileDownload->GetFilename().c_str()).c_str());
                return;   // failure, can't open file to write
                }*/
            }

        curl_multi_add_handle((CURLM*)m_pRequestHandle, pCurl);
        }
    }

void RealityDataServiceTransfer::InitTool()
    {
    m_pRequestHandle = curl_multi_init();
    }