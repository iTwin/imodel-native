#include <RealityPlatform/SimpleRDSApi.h>
#include <CCApi/CCPublic.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedResponse::Clone(const RawServerResponse& raw)
    {
    header = raw.header;
    body = raw.body;
    responseCode = raw.responseCode;
    curlCode = raw.curlCode;
    status = raw.status;

    simpleSuccess = (raw.curlCode == 0) && (raw.responseCode < 400);
    simpleMessage = raw.body;
    }

RDSRequestManager* RDSRequestManager::s_instance = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
RDSRequestManager::RDSRequestManager(RDS_FeedbackFunction callbackFunction, RDS_FeedbackFunction errorCallback) :
    m_callback(callbackFunction), m_errorCallback(errorCallback)
    {
    Utf8String serverName = MakeBuddiCall();
    WSGServer server = WSGServer(serverName, true);
    
    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = server.GetVersion(versionResponse);
    if (versionResponse.responseCode > 399)
        {
        m_errorCallback("cannot reach server");
        return;
        }

    RealityDataService::SetServerComponents(serverName, version, "S3MXECPlugin--Server", "S3MX");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
Utf8String RDSRequestManager::MakeBuddiCall()
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        m_errorCallback("Connection client does not seem to be installed");
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        m_errorCallback("Connection client does not seem to be running");
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        m_errorCallback("Connection client does not seem to be logged in");
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        m_errorCallback("Connection client user does not seem to have accepted EULA");
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        m_errorCallback("Connection client does not seem to have an active session\n");
        return "";
        }

    wchar_t* buddiUrl;
    UINT32 strlen = 0;

    CCApi_GetBuddiUrl(api, L"RealityDataServices", NULL, &strlen);
    strlen += 1;
    buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
    CCApi_GetBuddiUrl(api, L"RealityDataServices", buddiUrl, &strlen);

    char* charServer = new char[strlen];
    wcstombs(charServer, buddiUrl, strlen);

    CCApi_FreeApi(api);

    return Utf8String(charServer);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void RDSRequestManager::Report(Utf8String message)
    {
    if(m_callback)
        m_callback(message.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void RDSRequestManager::ReportError(Utf8String message)
    {
    if (m_errorCallback)
        m_errorCallback(message.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataEnterpriseStat::GetStats()
    {
    ConnectedResponse response = ConnectedResponse();
    RealityDataEnterpriseStatRequest* ptt = new RealityDataEnterpriseStatRequest("");

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataEnterpriseStat stat;
    RealityDataService::Request(*ptt, stat, rawResponse);
    Clone(stat);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataEnterpriseStat::ConnectedRealityDataEnterpriseStat(const RealityDataEnterpriseStat& stat)
    {
    Clone(stat);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataEnterpriseStat::GetAllStats(bvector<ConnectedRealityDataEnterpriseStat>& statVec)
    {
    ConnectedResponse response = ConnectedResponse();
    RawServerResponse rawResponse = RawServerResponse();
    RealityDataAllEnterpriseStatsRequest* ptt = new RealityDataAllEnterpriseStatsRequest();
    bvector<RealityDataEnterpriseStat> stats;
    stats = RealityDataService::Request(*ptt, rawResponse);
     
    for (int i = 0; i < stats.size(); i++)
        statVec.push_back(ConnectedRealityDataEnterpriseStat(stats[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataEnterpriseStat::Clone(const RealityDataEnterpriseStat& stat)
    {
    m_nbRealityData = stat.GetNbRealityData();
    m_totalSizeKB = stat.GetTotalSizeKB();
    m_organizationId = stat.GetOrganizationId();
    m_ultimateId = stat.GetUltimateId();
    m_ultimateSite = stat.GetUltimateSite();
    m_date = stat.GetDate();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataProjectRelationship::ConnectedRealityDataProjectRelationship(RealityDataProjectRelationshipPtr relationship)
    {
    Clone(relationship);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataProjectRelationship::Clone(RealityDataProjectRelationshipPtr relationship)
    {
    m_realityDataId = relationship->GetRealityDataId();
    m_relatedId = relationship->GetRelatedId();
    m_relationType = relationship->GetRelationType();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataProjectRelationship::RetrieveAllForRDId(bvector<ConnectedRealityDataProjectRelationshipPtr>& relationshipVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if(m_realityDataId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set realityData id, first";
        return response;
        }
        
    RawServerResponse rawResponse = RawServerResponse();
    RealityDataProjectRelationshipByRealityDataIdRequest idReq = RealityDataProjectRelationshipByRealityDataIdRequest(m_realityDataId);
   
    bvector<RealityDataProjectRelationshipPtr> tmpVector = RealityDataService::Request(idReq, rawResponse);

    for(int i = 0; i < tmpVector.size(); i++)
        relationshipVector.push_back(new ConnectedRealityDataProjectRelationship(tmpVector[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataProjectRelationship::RetrieveAllForProjectId(bvector<ConnectedRealityDataProjectRelationshipPtr>& relationshipVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_relatedId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set related id, first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataProjectRelationshipByProjectIdRequest idReq = RealityDataProjectRelationshipByProjectIdRequest(m_relatedId);

    bvector<RealityDataProjectRelationshipPtr> tmpVector = RealityDataService::Request(idReq, rawResponse);

    for (int i = 0; i < tmpVector.size(); i++)
        relationshipVector.push_back(new ConnectedRealityDataProjectRelationship(tmpVector[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataProjectRelationship::Create()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_realityDataId.empty() || m_relatedId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set properties, first";
        return response;
        }

    RealityDataRelationshipCreateRequest relReq = RealityDataRelationshipCreateRequest(m_realityDataId, m_relatedId);

    RawServerResponse rawResponse = RawServerResponse();
    Utf8String simpleMessage = RealityDataService::Request(relReq, rawResponse);

    response.Clone(rawResponse);
    response.simpleMessage = simpleMessage;

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataProjectRelationship::Delete()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_realityDataId.empty() || m_relatedId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set properties, first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete(m_realityDataId, m_relatedId);

    Utf8String simpleMessage = RealityDataService::Request(relReq, rawResponse);

    response.Clone(rawResponse);
    response.simpleMessage = simpleMessage;

    return response;
    }

/*ConnectedResponse ConnectedRealityDataDocument::RetrieveAllForRealityData(bvector<ConnectedRealityDataDocumentPtr>& docVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_realityDataId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set realityData id, first";
        return response;
        }

    AzureHandshake* handshake = new AzureHandshake(m_realityDataId, false);
    RawServerResponse handshakeResponse = RealityDataService::BasicRequest((RealityDataUrl*)handshake);
    Utf8String azureServer;
    Utf8String azureToken;
    int64_t tokenTimer;
    BentleyStatus handshakeStatus = handshake->ParseResponse(handshakeResponse.body, azureServer, azureToken, tokenTimer);
    delete handshake;
    if (handshakeStatus != BentleyStatus::SUCCESS)
        {
        response.simpleSuccess = false;
        response.simpleMessage = "Failure retrieving Azure token\n";
        if (handshakeResponse.body.ContainsI("InstanceNotFound") || handshakeResponse.body.ContainsI("does not exist"))
            response.simpleMessage.append("This entity seems to have been removed. Perhaps by another user\n");
        return response;
        }

    //AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(m_realityDataId);

    return response;*/
    //todo
    /*RawServerResponse sasResponse = RawServerResponse();
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, sasResponse);

    std::string str;
    size_t placeholder = 0;
    size_t step;
    size_t size = filesInRepo.size();
    while (m_lastCommand != Command::Cancel && m_lastCommand != Command::Quit && placeholder < size)
        {
        std::getline(*s_inputSource, str);
        if (Utf8String(str.c_str()).Trim().EqualsI("Cancel"))
            m_lastCommand = Command::Cancel;
        else if (Utf8String(str.c_str()).Trim().EqualsI("Quit"))
            m_lastCommand = Command::Quit;
        else
            {
            step = (size < (placeholder + 20)) ? size : placeholder + 20;
            DisplayInfo("Input \"Cancel\" to quit at any time, otherwise press enter to proceed to the next page\n", DisplayOption::Tip);
            for (; placeholder < step; ++placeholder)
                {
                DisplayInfo(Utf8String(WPrintfString(L"%s %lu bytes \n", filesInRepo[placeholder].first.c_str(), filesInRepo[placeholder].second)));
                }
            }
        }*/
    //}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataDocument::ConnectedRealityDataDocument(RealityDataDocumentPtr docptr)
    {
    Clone(docptr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataDocument::Clone(RealityDataDocumentPtr docptr)
    {
    m_containerName = docptr->GetContainerName();
    m_name = docptr->GetName();
    m_id = docptr->GetId();
    m_folderId = docptr->GetFolderId();
    m_accessUrl = docptr->GetAccessUrl();
    m_realityDataId = docptr->GetRealityDataId();
    m_contentType = docptr->GetContentType();
    m_size = docptr->GetSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataDocument::GetInfo()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_id.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set server path to document (id), first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataDocumentByIdRequest request = RealityDataDocumentByIdRequest(m_id);
    RealityDataDocumentPtr docptr = RealityDataService::Request(request, rawResponse);
    
    Clone(docptr);
    response.Clone(rawResponse);
    
    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataDocument::Upload(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    RealityDataServiceUpload upload = RealityDataServiceUpload(filePath, serverPath, "", true, true);
    if (upload.IsValidTransfer())
        {
        upload.OnlyReportErrors(true);
        const TransferReport& ur = upload.Perform();
        Utf8String report;
        if(ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("document uploaded to %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            response.simpleMessage = "upload failed";
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no document to upload";
        }

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataDocument::Download(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    if (!filePath.DoesPathExist())
        {
        if (BeFileName::CreateNewDirectory(filePath.GetName()) != BeFileNameStatus::Success)
            {
            response.simpleSuccess = false;
            response.simpleMessage = "could not create file path";
            return response;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(filePath, serverPath);
    if (download.IsValidTransfer())
        {
        download.OnlyReportErrors(true);
        const TransferReport& dr = download.Perform();
        if (dr.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("document downloaded from %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            response.simpleMessage = "download failed";
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no document to download";
        }

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataDocument::Delete()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_id.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set server path to document (id), first";
        return response;
        }

    RealityDataDeleteDocument request = RealityDataDeleteDocument(m_id);
    RawServerResponse rawResponse = RealityDataService::BasicRequest(&request);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataFolder::ConnectedRealityDataFolder(RealityDataFolderPtr folderptr)
    {
    Clone(folderptr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataFolder::Clone(RealityDataFolderPtr folderptr)
    {
    m_id = folderptr->GetId();
    m_name = folderptr->GetName();
    m_parentId = folderptr->GetParentId();
    m_realityDataId = folderptr->GetRealityDataId();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataFolder::GetInfo() 
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_id.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set server path to folder (id), first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataFolderByIdRequest request = RealityDataFolderByIdRequest(m_id);
    RealityDataFolderPtr folderptr = RealityDataService::Request(request, rawResponse);
    
    Clone(folderptr);
    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataFolder::Upload(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    if (!filePath.DoesPathExist())
        {
        if (BeFileName::CreateNewDirectory(filePath.GetName()) != BeFileNameStatus::Success)
            {
            response.simpleSuccess = false;
            response.simpleMessage = "could not create file path";
            return response;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(filePath, serverPath);
    if (download.IsValidTransfer())
        {
        download.OnlyReportErrors(true);
        const TransferReport& ur = download.Perform();
        if (ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("folder uploaded to %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            ur.ToXml(response.simpleMessage);
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no folder to upload";
        }

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataFolder::Download(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    if (!filePath.DoesPathExist())
        {
        if (BeFileName::CreateNewDirectory(filePath.GetName()) != BeFileNameStatus::Success)
            {
            response.simpleSuccess = false;
            response.simpleMessage = "could not create file path";
            return response;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(filePath, serverPath);
    if (download.IsValidTransfer())
        {
        download.OnlyReportErrors(true);
        const TransferReport& dr = download.Perform();
        if (dr.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("folder downloaded from %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            dr.ToXml(response.simpleMessage);
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no folder to download";
        }

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataFolder::Delete()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_id.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set server path to document (id), first";
        return response;
        }

    RealityDataDeleteFolder request = RealityDataDeleteFolder(m_id);
    RawServerResponse rawResponse = RealityDataService::BasicRequest(&request);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityData::ConnectedRealityData(RealityDataPtr rd)
    {
    Clone(rd);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityData::Clone(RealityDataPtr rd)
    {
    m_identifier = rd->GetIdentifier();
    m_name = rd->GetName();
    m_resolution = rd->GetResolution();
    m_resolutionValue = rd->GetResolutionValue();
    m_accuracy = rd->GetAccuracy();
    m_accuracyValue = rd->GetAccuracyValue();
    m_classification = rd->GetClassification();
    m_dataset = rd->GetDataset();
    m_footprint = rd->GetFootprint();
    m_footprintString = rd->GetFootprintString();
    m_footprintExtent = rd->GetFootprintExtent();
    m_description = rd->GetDescription();
    m_visibility = rd->GetVisibility();
    m_visibilityString = rd->GetVisibilityTag();

    m_realityDataType = rd->GetRealityDataType();
    m_streamed = rd->IsStreamed();
    m_creationDate = rd->GetCreationDateTime();
    m_modifiedDate = rd->GetModifiedDateTime();
    m_thumbnailDocument = rd->GetThumbnailDocument();
    m_organizationId = rd->GetOrganizationId();
    m_containerName = rd->GetContainerName();
    m_dataLocationGuid = rd->GetDataLocationGuid();
    m_rootDocument = rd->GetRootDocument();
    m_metadataUrl = rd->GetMetadataUrl();
    m_ultimateId = rd->GetUltimateId();
    m_ultimateSite = rd->GetUltimateSite();
    m_copyright = rd->GetCopyright();
    m_termsOfUse = rd->GetTermsOfUse();
    m_listable = rd->IsListable();
    m_owner = rd->GetOwner();
    m_group = rd->GetGroup();
    m_totalSize = rd->GetTotalSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::RetrieveAllForUltimateId(bvector<ConnectedRealityDataPtr>& dataVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_ultimateId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set ultimate id, first";
        return response;
        }

    RealityDataListByUltimateIdPagedRequest ultimateReq = RealityDataListByUltimateIdPagedRequest(m_ultimateId, 0, 2500);

    RawServerResponse ultimateResponse = RawServerResponse();
    ultimateResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> partialVec;

    while (ultimateResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(ultimateReq, ultimateResponse);
        dataVector.insert(dataVector.end(), dataVector.begin(), dataVector.end());
        }
    
    response.Clone(ultimateResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::GetInfo()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_ultimateId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set ultimate id, first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();

    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_identifier);
    RealityDataPtr entity = RealityDataService::Request(idReq, rawResponse);

    Clone(entity);
    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::Upload(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    if (!filePath.DoesPathExist())
        {
        if (BeFileName::CreateNewDirectory(filePath.GetName()) != BeFileNameStatus::Success)
            {
            response.simpleSuccess = false;
            response.simpleMessage = "could not create file path";
            return response;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(filePath, serverPath);
    if (download.IsValidTransfer())
        {
        download.OnlyReportErrors(true);
        const TransferReport& ur = download.Perform();
        if (ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("repo uploaded to %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            ur.ToXml(response.simpleMessage);
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no files to upload";
        }   

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::Download(BeFileName filePath, Utf8String serverPath)
    {
    ConnectedResponse response = ConnectedResponse();

    if (!filePath.DoesPathExist())
        {
        if (BeFileName::CreateNewDirectory(filePath.GetName()) != BeFileNameStatus::Success)
            {
            response.simpleSuccess = false;
            response.simpleMessage = "could not create file path";
            return response;
            }
        }

    RealityDataServiceDownload download = RealityDataServiceDownload(filePath, serverPath);
    if (download.IsValidTransfer())
        {
        download.OnlyReportErrors(true);
        const TransferReport& dr = download.Perform();
        if (dr.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("files downloaded from %s", serverPath);
            }
        else
            {
            response.simpleSuccess = false;
            dr.ToXml(response.simpleMessage);
            }
        }
    else
        {
        response.simpleSuccess = false;
        response.simpleMessage = "failed to communicate with server; or no files to upload";
        }

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::Delete()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_identifier.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set server path to document (id), first";
        return response;
        }
    RealityDataDelete realityDataReq = RealityDataDelete(m_identifier);
    RawServerResponse rawResponse = RealityDataService::BasicRequest(&realityDataReq);

    response.Clone(rawResponse);

    return response;
    }