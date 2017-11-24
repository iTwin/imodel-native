/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsComplete/RealityDataService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>
#include "../RealityPlatformTools/RealityDataService.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//! Callback methods used by the upload/download process.
//=====================================================================================
static size_t CurlReadDataCallback(void* buffer, size_t size, size_t count, RealityDataFileUpload* request)
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

    for (int i = 0; i < std::min(MAX_NB_CONNECTIONS, (int)m_filesToTransfer.size()); ++i)
        {
        SetupNextEntry();
        }   

    int still_running; /* keep number of running handles */
    int repeats = 0;

    do
        {
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
                Sleep(300);
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
                                if(fileUp!= nullptr)
                                    UpdateTransferAmount((int64_t)fileUp->GetMessageSize());
                                else
                                    UpdateTransferAmount(1);
                                m_pProgressFunc(fileTrans->GetFilename(), 1.0, m_progress);
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
        fileTransfer->UpdateTransferedSize();
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

            if (fileDownload->GetFileStream().Create(fileDownload->GetFilename().c_str(), true) != BeFileStatus::Success)
                {
                ReportStatus(0, nullptr, -1, Utf8PrintfString("\nFailed to create File %s on local machine\nAborting download of this file", fileDownload->GetFilename().c_str()).c_str());
                return;   // failure, can't open file to write
                }
            }

        curl_multi_add_handle((CURLM*)m_pRequestHandle, pCurl);
        }
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceUpload::RealityDataServiceUpload(BeFileName uploadPath, Utf8String id, Utf8String properties, 
    bool overwrite, bool listable, RealityDataServiceTransfer_StatusCallBack pi_func, bvector<BeFileName> colorList, 
    bool isBlackList, Utf8String proxyUrl, Utf8String proxyCreds) :
    RealityDataServiceTransfer(), m_overwrite(overwrite)
    { 
    m_id = id;
    m_azureTokenTimer = 0;
    m_progress = 0.0; 
    m_progressStep = 0.01; 
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false; 
    m_currentTransferedAmount = 0;
    m_fullTransferSize = 0;
    m_handshakeRequest = nullptr;
    m_pStatusFunc = pi_func;

    if(proxyUrl.length() > 0)
        SetProxyUrlAndCredentials(proxyUrl, proxyCreds);

    if(!listable)
        {
        if(properties.length() > 0)
            properties.append(",");
        properties.append("\"Listable\" : false");
        }

    if (!RealityDataService::AreParametersSet())
        {
        ReportStatus(0, nullptr, -1, "Server, Version, Repository and Schema not set, please use RealityDataService::SetServerComponents before calling this function\n");
        return;
        }

    if(CreateUpload(properties) != BentleyStatus::SUCCESS)
        return;
    m_handshakeRequest = new AzureHandshake(m_id, true);
    GetAzureToken();

    RealityDataFileUpload* fileUp;

    if(uploadPath.DoesPathExist() && uploadPath.IsDirectory()) //path is directory, find all documents
        {
        uploadPath.AppendSeparator();

        BeFileName root(uploadPath);

        uploadPath.AppendToPath(L"*");
        BeFileListIterator fileIt = BeFileListIterator(uploadPath.GetName(), true);

        BeFileName fileName;
        //BeFileListIterator returns the same filenames twice for every subfolder containing it
        // i.e. folder/folder2/test.txt would appear 4 times
        // the bset is used to avoid adding multiple uploads for a single file
        bset<Utf8String> duplicateSet = bset<Utf8String>(); 
        size_t i = 0;
        bool whiteListed;
        bool noColorList = colorList.empty();
        while (fileIt.GetNextFileName(fileName) == BentleyStatus::SUCCESS) 
            {
            if(!uploadPath.IsDirectory() && duplicateSet.find(fileName.GetNameUtf8()) == duplicateSet.end())
                {
                duplicateSet.insert(fileName.GetNameUtf8());
                whiteListed = noColorList || isBlackList;
                for (BeFileName colorFile : colorList)
                    {
                    if(fileName.GetNameUtf8().ContainsI(colorFile.GetNameUtf8()))
                        whiteListed = !isBlackList;
                    }
            
                if(whiteListed)
                    {
                    fileUp = new RealityDataFileUpload(fileName, root, m_azureServer, i++);
                    m_filesToTransfer.push_back(fileUp);
                    m_fullTransferSize += fileUp->GetFileSize();
                    }
                }
            }
        }
    else if (uploadPath.DoesPathExist())
        {
        fileUp = new RealityDataFileUpload(uploadPath, uploadPath.GetDirectoryName(), m_azureServer, 0);
        m_filesToTransfer.push_back(fileUp);
        m_fullTransferSize = fileUp->GetFileSize();
        }

    m_pRequestHandle = curl_multi_init();
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
RealityDataServiceDownload::RealityDataServiceDownload(BeFileName targetLocation, Utf8String serverId, 
    RealityDataServiceTransfer_StatusCallBack pi_func, Utf8String proxyUrl, Utf8String proxyCreds) :
    RealityDataServiceTransfer()
    {
    m_id = serverId;
    m_azureTokenTimer = 0;
    m_progress = 0.0;
    m_progressStep = 0.01;
    m_progressThreshold = 0.01;
    m_onlyReportErrors = false;
    m_currentTransferedAmount = 0;
    m_handshakeRequest = nullptr;
    m_pStatusFunc = pi_func;

    if (proxyUrl.length() > 0)
        SetProxyUrlAndCredentials(proxyUrl, proxyCreds);

    m_handshakeRequest = new AzureHandshake(m_id, false);
    GetAzureToken();

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_id);
    RawServerResponse rawResponse = RawServerResponse();
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, rawResponse);

    // If no files listed ... something went wrong
    if (filesInRepo.size() == 0)
        rawResponse.status = RequestStatus::BADREQ;

    if(rawResponse.status == RequestStatus::BADREQ)
        {
        ReportStatus(0, nullptr, -1, "Error performing SAS request on Azure server\n");
        return;
        }

    BeFileName downloadLocation;
    WString path;
    WString fileUrl;
    Utf8String utf8FileUrl;
    size_t parts;

    WString root;
    BeStringUtilities::Utf8ToWChar(root, m_id.c_str());
    bvector<WString> folders;
    BeStringUtilities::Split(root.c_str(),L"/", folders);
    WString guid = folders[0];
    guid.append(L"/");
    root.ReplaceAll(guid.c_str(), L""); //remove guid from root

    if ((folders.size() > 1) && !m_id.EndsWith("/")) //if path ends with "/" it is a folder; otherwise, it is a single document
        {
        path = filesInRepo[0].first;

        fileUrl = L"/";
        fileUrl.append(path);
        fileUrl.ReplaceAll(L"\\", L"/");
        BeStringUtilities::WCharToUtf8(utf8FileUrl, fileUrl.c_str());

        downloadLocation = targetLocation;
        downloadLocation.AppendToPath(folders[folders.size()-1].c_str());

        m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, utf8FileUrl, m_azureServer, 0, filesInRepo[0].second));
        }
    else
        {
        for( size_t i = 0; i < filesInRepo.size(); ++i)
            {
            path = filesInRepo[i].first;

            fileUrl = L"/";
            fileUrl.append(path);
            fileUrl.ReplaceAll(L"\\", L"/");
            BeStringUtilities::WCharToUtf8(utf8FileUrl, fileUrl.c_str());

            path.ReplaceAll(root.c_str(), L""); // if user downloader Folder1/Folder2/Data1, it should download to Data1, not Folder1/Folder2/Data1
            parts = path.ReplaceAll(L"/", L"/"); // only way I've found to count occurences in a string, replace if better exists
            if(parts > 0) //if file is in a directory
                {
                downloadLocation = targetLocation;
                downloadLocation.AppendToPath(path.c_str());
                downloadLocation.PopDir();

                if (!downloadLocation.DoesPathExist())
                    BeFileName::CreateNewDirectory(downloadLocation.c_str());
                }

            downloadLocation = targetLocation;
            downloadLocation.AppendToPath(path.c_str());
        
            m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, utf8FileUrl, m_azureServer, i, filesInRepo[i].second));
            }
        }

    m_fullTransferSize = m_filesToTransfer.size();

    m_pRequestHandle = curl_multi_init();
    }