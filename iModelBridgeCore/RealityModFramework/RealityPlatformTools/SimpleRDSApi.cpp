/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatformTools/SimpleRDSApi.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

void RDSRequestManager::Setup(Utf8String serverUrl)
    {
    Utf8String serverName = serverUrl;
    if(serverName.empty())
        serverName = MakeBuddiCall(L"RealityDataServices");
    WSGServer server = WSGServer(serverName, false);
    
    RawServerResponse versionResponse = RawServerResponse();
    Utf8String version = server.GetVersion(versionResponse);
    if (versionResponse.responseCode > 399)
        {
        ReportError("cannot reach server");
        return;
        }
    
    RealityDataService::SetServerComponents(serverName, version, "S3MXECPlugin--Server", "S3MX", WSGRequest::GetInstance().GetCertificatePath().GetNameUtf8());
    }

ConnectedNavNode::ConnectedNavNode(const NavNode& node)
    {
    Clone(node);
    }

void ConnectedNavNode::Clone(const NavNode& node)
    {
    m_navString = node.GetNavString();
    m_typeSystem = node.GetTypeSystem();
    m_schemaName = node.GetSchemaName();
    m_className = node.GetECClassName();
    m_instanceId = node.GetInstanceId();
    m_label = node.GetLabel();

    m_rootNode = node.GetRootNode();
    m_rootId = node.GetRootId();
    }

ConnectedResponse ConnectedNavNode::GetRootNodes(bvector<ConnectedNavNode>& nodes)
    {
    ConnectedResponse response = ConnectedResponse();

    RawServerResponse rawResponse = RawServerResponse();
    bvector<NavNode> rootNodes = NodeNavigator::GetInstance().GetRootNodes(RealityDataService::GetServerName(), RealityDataService::GetRepoName(), rawResponse);

    for (size_t i = 0; i < rootNodes.size(); i++)
        nodes.push_back(ConnectedNavNode(rootNodes[i]));

    response.Clone(rawResponse);

    return response;
    }

ConnectedResponse ConnectedNavNode::GetChildNodes(bvector<ConnectedNavNode>& nodes)
    {
    ConnectedResponse response = ConnectedResponse();

    RawServerResponse rawResponse = RawServerResponse();
    bvector<NavNode> rootNodes = NodeNavigator::GetInstance().GetChildNodes(RealityDataService::GetServerName(), RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName(), GetNavString(), rawResponse);

    for (size_t i = 0; i < rootNodes.size(); i++)
        nodes.push_back(ConnectedNavNode(rootNodes[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataLocation::ConnectedRealityDataLocation(Utf8String guid)
    {
    m_identifier = guid;
    GetDataLocation();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataLocation::ConnectedRealityDataLocation(const ConnectedRealityDataLocation& object)
: RealityDataLocation(object)
    {
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataLocation::ConnectedRealityDataLocation(const RealityDataLocation& object)
: RealityDataLocation(object)
    {
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataLocation::Clone(const RealityDataLocation& location)
    {
    m_identifier       = location.GetIdentifier();
    m_provider         = location.GetProvider();
    m_location         = location.GetLocation();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataLocation::GetDataLocation()
    {
    ConnectedResponse response = ConnectedResponse();
    RealityDataLocationRequest ptt(m_identifier);

    RawServerResponse rawResponse;
    RealityDataLocation location;
    RealityDataService::Request(ptt, location, rawResponse);
    Clone(location);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataLocation::RetrieveAllDataLocations(bvector<ConnectedRealityDataLocation>& dataLocations)
    {
    ConnectedResponse response;

    AllRealityDataLocationsRequest dataLocationRequest;

    RawServerResponse ultimateResponse;
    ultimateResponse.status = RequestStatus::OK;
    bvector<RealityDataLocation> tempDataLocations;

    tempDataLocations = RealityDataService::Request(dataLocationRequest, ultimateResponse);
    for (RealityDataLocation currentLocation : tempDataLocations)
        {
        ConnectedRealityDataLocation temp(currentLocation);
        dataLocations.push_back(temp);
        }

    response.Clone(ultimateResponse);

    return response;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
ConnectedRealityDataPublicKey::ConnectedRealityDataPublicKey(Utf8String guid)
    {
    m_identifier = guid;
    GetPublicKey();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
ConnectedRealityDataPublicKey::ConnectedRealityDataPublicKey(const ConnectedRealityDataPublicKey& object)
: RealityDataPublicKey(object)
    {
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
ConnectedRealityDataPublicKey::ConnectedRealityDataPublicKey(const RealityDataPublicKey& object)
: RealityDataPublicKey(object)
    {
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
void ConnectedRealityDataPublicKey::Clone(const RealityDataPublicKey& publicKey)
    {
    m_identifier = publicKey.GetIdentifier();
    m_realityDataId = publicKey.GetRealityDataId();
    m_userId = publicKey.GetUserId();
    m_description = publicKey.GetDescription();
    m_ultimateId = publicKey.GetUltimateId();
    m_creationDateTime = publicKey.GetCreationDateTime();
    m_modifiedDateTime = publicKey.GetModifiedDateTime();
    m_authorizedUserIds = publicKey.GetAuthorizedUserIds();
    m_validUntilDate = publicKey.GetValidUntilDate();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataPublicKey::GetPublicKey()
    {
    ConnectedResponse response = ConnectedResponse();
    RealityDataPublicKeyRequest ptt(m_identifier);

    RawServerResponse rawResponse;
    RealityDataPublicKey publicKey;
    RealityDataService::Request(ptt, publicKey, rawResponse);
    Clone(publicKey);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2019
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataPublicKey::RetrieveAllPublicKeys(bvector<ConnectedRealityDataPublicKey>& publicKeys)
    {
    ConnectedResponse response;

    AllRealityDataPublicKeysRequest publicKeysRequest;

    RawServerResponse ultimateResponse;
    ultimateResponse.status = RequestStatus::OK;
    bvector<RealityDataPublicKey> tempPublicKeys;

    tempPublicKeys = RealityDataService::Request(publicKeysRequest, ultimateResponse);
    for (RealityDataPublicKey currentPublicKey : tempPublicKeys)
        {
        ConnectedRealityDataPublicKey temp(currentPublicKey);
        publicKeys.push_back(temp);
        }

    response.Clone(ultimateResponse);

    return response;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataEnterpriseStat::GetEnterpriseStats()
    {
    ConnectedResponse response = ConnectedResponse();
    RealityDataEnterpriseStatRequest ptt("");

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataEnterpriseStat stat;
    RealityDataService::Request(ptt, stat, rawResponse);
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
// @bsimethod                                   Alain.Robert                04/2018
//-------------------------------------------------------------------------------------
ConnectedRealityDataServiceStat::ConnectedRealityDataServiceStat(const RealityDataServiceStat& stat)
    {
    Clone(stat);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2018
//-------------------------------------------------------------------------------------
ConnectedRealityDataUserStat::ConnectedRealityDataUserStat(const RealityDataUserStat& stat)
    {
    Clone(stat);
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
// @bsimethod                                   Alain.Robert                04/2018
//-------------------------------------------------------------------------------------
void ConnectedRealityDataServiceStat::Clone(const RealityDataServiceStat& stat)
    {
    m_nbRealityData = stat.GetNbRealityData();
    m_totalSizeKB = stat.GetTotalSizeKB();
    m_ultimateId = stat.GetUltimateId();
    m_serviceId = stat.GetServiceId();
    m_date = stat.GetDate();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                04/2018
//-------------------------------------------------------------------------------------
void ConnectedRealityDataUserStat::Clone(const RealityDataUserStat& stat)
    {
    m_nbRealityData = stat.GetNbRealityData();
    m_totalSizeKB = stat.GetTotalSizeKB();
    m_userId = stat.GetUserId();
    m_userEmail = stat.GetUserEmail();
    m_ultimateId = stat.GetUltimateId();
    m_serviceId = stat.GetServiceId();
    m_dataLocationGuid = stat.GetDataLocationGuid();
    m_date = stat.GetDate();
    }	
	
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityDataRelationship::ConnectedRealityDataRelationship(RealityDataRelationshipPtr relationship)
    {
    Clone(relationship);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataRelationship::Clone(RealityDataRelationshipPtr relationship)
    {
    if(relationship == nullptr)
        return;
    m_realityDataId = relationship->GetRealityDataId();
    m_relatedId = relationship->GetRelatedId();
    m_relationType = relationship->GetRelationType();
    m_modifiedDateTime = relationship->GetModifiedDateTime();
    m_createdDateTime = relationship->GetCreationDateTime();

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataRelationship::RetrieveAllForRDId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if(m_realityDataId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set realityData id, first";
        return response;
        }
        
    return RetrieveAllForRDId(relationshipVector, m_realityDataId);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataRelationship::RetrieveAllForRDId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector, Utf8String rdId)
    {
    ConnectedResponse response = ConnectedResponse();

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataRelationshipByRealityDataIdRequest idReq = RealityDataRelationshipByRealityDataIdRequest(rdId);

    bvector<RealityDataRelationshipPtr> tmpVector = RealityDataService::Request(idReq, rawResponse);

    for (size_t i = 0; i < tmpVector.size(); i++)
        relationshipVector.push_back(new ConnectedRealityDataRelationship(tmpVector[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataRelationship::RetrieveAllForProjectId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector)
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_relatedId.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set related id, first";
        return response;
        }
    
    return RetrieveAllForProjectId(relationshipVector, m_relatedId);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataRelationship::RetrieveAllForProjectId(bvector<ConnectedRealityDataRelationshipPtr>& relationshipVector, Utf8String projectId)
    {
    ConnectedResponse response = ConnectedResponse();

    RawServerResponse rawResponse = RawServerResponse();
    RealityDataRelationshipByProjectIdRequest idReq = RealityDataRelationshipByProjectIdRequest(projectId);

    bvector<RealityDataRelationshipPtr> tmpVector = RealityDataService::Request(idReq, rawResponse);

    for (size_t i = 0; i < tmpVector.size(); i++)
        relationshipVector.push_back(new ConnectedRealityDataRelationship(tmpVector[i]));

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityDataRelationship::CreateOnServer()
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
ConnectedResponse ConnectedRealityDataRelationship::Delete()
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

ConnectedResponse ConnectedRealityDataDocument::RetrieveAllForRealityData(bvector<bpair<Utf8String, uint64_t>>& docVector, Utf8String realityDataGUID)
    {
    ConnectedResponse response = ConnectedResponse();

    AllRealityDataByRootId rdsRequest = AllRealityDataByRootId(realityDataGUID);

    RawServerResponse handshakeResponse = rdsRequest.GetAzureRedirectionRequestUrl();

    if (!rdsRequest.IsAzureBlobRedirected())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "Failure retrieving Azure token\n";
        if (handshakeResponse.body.ContainsI("InstanceNotFound") || handshakeResponse.body.ContainsI("does not exist"))
            response.simpleMessage.append("This entity seems to have been removed. Perhaps by another user\n");
        return response;
        }

    RawServerResponse sasResponse = RawServerResponse();
    bvector<bpair<WString, uint64_t>> filesInRepo = RealityDataService::Request(rdsRequest, sasResponse);

    for(size_t i = 0; i < filesInRepo.size(); i++)
        {
        docVector.push_back(make_bpair(Utf8PrintfString("%s/%s", realityDataGUID.c_str(), Utf8String(filesInRepo[i].first.c_str()).c_str()), filesInRepo[i].second));
        }

    response.Clone(sasResponse);
    return response;
    }

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
    if(docptr == nullptr)
        return;
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
ConnectedRealityDataDocument::ConnectedRealityDataDocument(Utf8String navString)
    {
    m_id = navString;
    GetInfo();
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
ConnectedResponse ConnectedRealityDataDocument::Upload(BeFileName filePath, Utf8String serverPath, bool overwrite, bool listable)
    {
    ConnectedResponse response = ConnectedResponse();

    NavNode node = NavNode(RealityDataService::GetSchemaName(), serverPath, "ECObjects", "RealityData");

    RealityDataServiceUpload upload = RealityDataServiceUpload(filePath, node.GetNavString(), "", overwrite, listable);
    if (upload.IsValidTransfer())
        {
        upload.OnlyReportErrors(true);
        const TransferReport& ur = upload.Perform();
        Utf8String report;
        if(ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("document uploaded to %s", serverPath.c_str());
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
            response.simpleMessage = Utf8PrintfString("document downloaded from %s", serverPath.c_str());
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
    RawServerResponse rawResponse = RealityDataService::BasicRequest(&request, "changedInstance");

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
ConnectedRealityDataFolder::ConnectedRealityDataFolder(Utf8String navString) 
    {
    m_id = navString;
    GetInfo();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectedRealityDataFolder::Clone(RealityDataFolderPtr folderptr)
    {
    if (folderptr == nullptr)
        return;
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
ConnectedResponse ConnectedRealityDataFolder::Upload(BeFileName filePath, Utf8String serverPath, bool overwrite, bool listable)
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

    NavNode node = NavNode(RealityDataService::GetSchemaName(), serverPath, "ECObjects", "RealityData");

    RealityDataServiceUpload upload = RealityDataServiceUpload(filePath, node.GetNavString(), "", overwrite, listable);
    if (upload.IsValidTransfer())
        {
        upload.OnlyReportErrors(true);
        const TransferReport& ur = upload.Perform();
        if (ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("folder uploaded to %s", serverPath.c_str());
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
            response.simpleMessage = Utf8PrintfString("folder downloaded from %s", serverPath.c_str());
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
    RawServerResponse rawResponse = RealityDataService::BasicRequest(&request, "changedInstance");

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
    if(rd == nullptr)
        return;
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
    m_creatorId = rd->GetCreatorId();
    m_group = rd->GetGroup();
    m_totalSize = rd->GetTotalSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedRealityData::ConnectedRealityData(Utf8String guid)
    {
    m_identifier = guid;
    GetInfo();
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

    return RetrieveAllForUltimateId(dataVector, m_ultimateId);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::RetrieveAllForUltimateId(bvector<ConnectedRealityDataPtr>& dataVector, Utf8String ultimateId)
    {
    ConnectedResponse response = ConnectedResponse();

    RealityDataListByUltimateIdPagedRequest ultimateReq = RealityDataListByUltimateIdPagedRequest(ultimateId, 0, 2500);

    RawServerResponse ultimateResponse = RawServerResponse();
    ultimateResponse.status = RequestStatus::OK;
    bvector<RealityDataPtr> partialVec;

    while (ultimateResponse.status == RequestStatus::OK)
        {//When LASTPAGE has been added, loop will exit
        partialVec = RealityDataService::Request(ultimateReq, ultimateResponse);
        for (RealityDataPtr rdPtr : partialVec)
            {
            ConnectedRealityDataPtr tempPtr = new ConnectedRealityData(rdPtr.get());
            dataVector.push_back(tempPtr);
            }
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
    if (m_identifier.empty())
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
ConnectedResponse ConnectedRealityData::UpdateInfo()
    {
    ConnectedResponse response = ConnectedResponse();
    if (m_identifier.empty())
        {
        response.simpleSuccess = false;
        response.simpleMessage = "must set ultimate id, first";
        return response;
        }

    RawServerResponse rawResponse = RawServerResponse();

    Utf8String propertyString = "";
    if(!m_organizationId.empty())
        propertyString.append(Utf8PrintfString("\"OrganizationId\" : \"%s\",", m_organizationId.c_str()));
    if (!m_ultimateId.empty())
        propertyString.append(Utf8PrintfString("\"UltimateId\" : \"%s\",", m_ultimateId.c_str()));
    if (!m_ultimateSite.empty())
        propertyString.append(Utf8PrintfString("\"UltimateSite\" : \"%s\",", m_ultimateSite.c_str()));
    if (!m_name.empty())
        propertyString.append(Utf8PrintfString("\"Name\" : \"%s\",", m_name.c_str()));
    if (!m_dataset.empty())
        propertyString.append(Utf8PrintfString("\"Dataset\" : \"%s\",", m_dataset.c_str()));
    if (!m_group.empty())
        propertyString.append(Utf8PrintfString("\"Group\" : \"%s\",", m_group.c_str()));
    if (!m_description.empty())
        propertyString.append(Utf8PrintfString("\"Description\" : \"%s\",", m_description.c_str()));
    if (!m_rootDocument.empty())
        propertyString.append(Utf8PrintfString("\"RootDocument\" : \"%s\",", m_rootDocument.c_str()));
    if (!GetClassificationTag().empty())
        propertyString.append(Utf8PrintfString("\"Classification\" : \"%s\",", GetClassificationTag().c_str()));
    if (!m_realityDataType.empty())
        propertyString.append(Utf8PrintfString("\"Type\" : \"%s\",", m_realityDataType.c_str()));
    propertyString.append(m_streamed ? "\"Streamed\" : true," : "\"Streamed\" : false,"); 
    if (!GetFootprintString().empty())
        propertyString.append(Utf8PrintfString("\"Footprint\" : \"%s\",", GetFootprintString().c_str()));
    if (!m_thumbnailDocument.empty())
        propertyString.append(Utf8PrintfString("\"ThumbnailDocument\" : \"%s\",", m_thumbnailDocument.c_str()));
    if (!m_metadataUrl.empty())
        propertyString.append(Utf8PrintfString("\"MetadataUrl\" : \"%s\",", m_metadataUrl.c_str()));
    if (!m_copyright.empty())
        propertyString.append(Utf8PrintfString("\"Copyright\" : \"%s\",", m_copyright.c_str()));
    if (!m_metadataUrl.empty())
        propertyString.append(Utf8PrintfString("\"MetadataUrl\" : \"%s\",", m_metadataUrl.c_str()));
    if (!m_termsOfUse.empty())
        propertyString.append(Utf8PrintfString("\"TermsOfUse\" : \"%s\",", m_termsOfUse.c_str()));
    if (!m_resolution.empty())
        propertyString.append(Utf8PrintfString("\"ResolutionInMeters\" : \"%s\",", m_resolution.c_str()));
    if (!m_accuracy.empty())
        propertyString.append(Utf8PrintfString("\"AccuracyInMeters\" : \"%s\",", m_accuracy.c_str()));
    if (!GetVisibilityTag().empty())
        propertyString.append(Utf8PrintfString("\"Visibility\" : \"%s\",", GetVisibilityTag().c_str()));
    propertyString.append(m_listable ? "\"Listable\" : true," : "\"Listable\" : false,");
    if (!m_owner.empty())
        propertyString.append(Utf8PrintfString("\"OwnedBy\" : \"%s\",", m_owner.c_str()));
    // CreatorId is not set but automatically generated
    propertyString.append(m_hidden ? "\"Hidden\" : true," : "\"Hidden\" : false,");
    propertyString.append(m_delegatePermissions ? "\"DelegatePermissions\" : true," : "\"DelegatePermissions\" : false,");
    
    RealityDataChangeRequest changeReq = RealityDataChangeRequest(m_identifier, propertyString);
    RealityDataService::Request(changeReq, rawResponse);

    response.Clone(rawResponse);

    return response;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
ConnectedResponse ConnectedRealityData::Upload(BeFileName filePath, Utf8StringR serverPath, bool overwrite, bool listable)
    {
    m_identifier = serverPath;

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

    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    if (!m_organizationId.empty())
        properties.Insert(RealityDataField::OrganizationId, m_organizationId);
    if (!m_ultimateId.empty())
        properties.Insert(RealityDataField::UltimateId, m_ultimateId);
    if (!m_ultimateSite.empty())
        properties.Insert(RealityDataField::UltimateSite, m_ultimateSite);
    if (!m_containerName.empty())
        properties.Insert(RealityDataField::ContainerName, m_containerName);
    if (!m_dataLocationGuid.empty())
        properties.Insert(RealityDataField::DataLocationGuid, m_dataLocationGuid);
    if (!m_name.empty())
        properties.Insert(RealityDataField::Name, m_name);
    if (!m_dataset.empty())
        properties.Insert(RealityDataField::Dataset, m_dataset);
    if (!m_description.empty())
        properties.Insert(RealityDataField::Description, m_description);
    if (!m_rootDocument.empty())
        properties.Insert(RealityDataField::RootDocument, m_rootDocument);
    if (m_classification != Classification::UNDEFINED_CLASSIF)
        properties.Insert(RealityDataField::Classification, GetClassificationTag());
    if (!m_realityDataType.empty())
        properties.Insert(RealityDataField::Type, m_realityDataType);
    properties.Insert(RealityDataField::Streamed, m_streamed ? "true" : "false");
    if (!m_footprintString.empty() || !m_footprint.empty())
        properties.Insert(RealityDataField::Footprint, GetFootprintString());
    properties.Insert(RealityDataField::ApproximateFootprint, m_approximateFootprint ? "true" : "false");
    if (!m_thumbnailDocument.empty())
        properties.Insert(RealityDataField::ThumbnailDocument, m_thumbnailDocument);
    if (!m_metadataUrl.empty())
        properties.Insert(RealityDataField::MetadataUrl, m_metadataUrl);
    if (!m_copyright.empty())
        properties.Insert(RealityDataField::Copyright, m_copyright);
    if (!m_termsOfUse.empty())
        properties.Insert(RealityDataField::TermsOfUse, m_termsOfUse);
    if (!m_resolution.empty())
        properties.Insert(RealityDataField::ResolutionInMeters, m_resolution);
    if (!m_accuracy.empty())
        properties.Insert(RealityDataField::AccuracyInMeters, m_accuracy);
    if ((m_visibility != Visibility::UNDEFINED_VISIBILITY))
        properties.Insert(RealityDataField::Visibility, GetVisibilityTag());
    properties.Insert(RealityDataField::Listable, m_listable ? "true" : "false");
    if (!m_owner.empty())
        properties.Insert(RealityDataField::OwnedBy, m_owner);
    if (!m_group.empty())
        properties.Insert(RealityDataField::Group, m_group);

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    RealityDataServiceUpload upload = RealityDataServiceUpload(filePath, m_identifier, propertyString, overwrite, listable);
    if (upload.IsValidTransfer())
        {
        m_identifier = upload.GetRealityDataId();
        upload.OnlyReportErrors(true);
        const TransferReport& ur = upload.Perform();
        if (ur.results.empty())
            {
            response.simpleSuccess = true;
            response.simpleMessage = Utf8PrintfString("repo uploaded to %s", m_identifier.c_str());
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
            response.simpleMessage = Utf8PrintfString("files downloaded from %s", serverPath.c_str());
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
    RawServerResponse rawResponse = RawServerResponse();
    RealityDataService::Request(realityDataReq, rawResponse);

    response.Clone(rawResponse);

    return response;
    }