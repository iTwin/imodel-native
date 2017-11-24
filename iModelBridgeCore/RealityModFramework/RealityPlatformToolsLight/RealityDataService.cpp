/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsLight/RealityDataService.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/RealityDataService.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
const TransferReport& RealityDataServiceTransfer::Perform()
    {
    m_currentTransferedAmount = 0;
    m_progress = 0.0;

    m_report = TransferReport();

    if (!IsValidTransfer())
        {
        ReportStatus(0, nullptr, -1, "No files to transfer, please verify that the previous steps completed without failure");
        return m_report;
        }

    if(m_multiRequestCallback)
        m_multiRequestCallback(m_filesToTransfer, m_pProgressFunc);

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
    if (fileTransfer != nullptr)
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

    if(m_setupCallback)
        m_setupCallback(request, verifyPeer);
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

    if (proxyUrl.length() > 0)
        SetProxyUrlAndCredentials(proxyUrl, proxyCreds);

    if (!listable)
        {
        if (properties.length() > 0)
            properties.append(",");
        properties.append("\"Listable\" : false");
        }

    if (!RealityDataService::AreParametersSet())
        {
        ReportStatus(0, nullptr, -1, "Server, Version, Repository and Schema not set, please use RealityDataService::SetServerComponents before calling this function\n");
        return;
        }

    if (CreateUpload(properties) != BentleyStatus::SUCCESS)
        return;
    m_handshakeRequest = new AzureHandshake(m_id, true);
    GetAzureToken();

    RealityDataFileUpload* fileUp;

    if (uploadPath.DoesPathExist() && uploadPath.IsDirectory()) //path is directory, find all documents
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
            if (!uploadPath.IsDirectory() && duplicateSet.find(fileName.GetNameUtf8()) == duplicateSet.end())
                {
                duplicateSet.insert(fileName.GetNameUtf8());
                whiteListed = noColorList || isBlackList;
                for (BeFileName colorFile : colorList)
                    {
                    if (fileName.GetNameUtf8().ContainsI(colorFile.GetNameUtf8()))
                        whiteListed = !isBlackList;
                    }

                if (whiteListed)
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

    if (rawResponse.status == RequestStatus::BADREQ)
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
    BeStringUtilities::Split(root.c_str(), L"/", folders);
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
        downloadLocation.AppendToPath(folders[folders.size() - 1].c_str());

        m_filesToTransfer.push_back(new RealityDataFileDownload(downloadLocation, utf8FileUrl, m_azureServer, 0, filesInRepo[0].second));
        }
    else
        {
        for (size_t i = 0; i < filesInRepo.size(); ++i)
            {
            path = filesInRepo[i].first;

            fileUrl = L"/";
            fileUrl.append(path);
            fileUrl.ReplaceAll(L"\\", L"/");
            BeStringUtilities::WCharToUtf8(utf8FileUrl, fileUrl.c_str());

            path.ReplaceAll(root.c_str(), L""); // if user downloader Folder1/Folder2/Data1, it should download to Data1, not Folder1/Folder2/Data1
            parts = path.ReplaceAll(L"/", L"/"); // only way I've found to count occurences in a string, replace if better exists
            if (parts > 0) //if file is in a directory
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
}