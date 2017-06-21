//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.hpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include "SMStreamingDataStore.h"
#include "SMSQLiteStore.h"
#include "..\Threading\LightThreadPool.h"
#include <condition_variable>
#include <TilePublisher\TilePublisher.h>
#include <CloudDataSource\DataSourceAccount.h>
#include <CloudDataSource\DataSourceAccountWSG.h>
#include <CloudDataSource\DataSourceBuffered.h>

#include <ImagePP\all\h\HCDCodecZlib.h>
#include <ImagePP\all\h\HCDException.h>
#include <ImagePP\all\h\HFCAccessMode.h>
#include "ScalableMesh\ScalableMeshLib.h"

USING_NAMESPACE_IMAGEPP

template<class EXTENT> SMStreamingStore<EXTENT>::SMStreamingSettings::SMStreamingSettings(const Json::Value & config)
    {
    if (config.isMember("guid"))
        {
        m_guid = config["guid"].asCString();
        }
    if (config.isMember("data_type"))
        {
        auto const& data_type = config["data_type"].asString();
        if (data_type == "3DTiles") m_dataType = DataType::CESIUM3DTILES;
        else if (data_type == "SM3DTiles") m_dataType = DataType::SMCESIUM3DTILES;
        else if (data_type == "SMGroups") m_dataType = DataType::SMGROUPS;
        else
            {
            assert(!"Unknown data type for streaming");
            }
        }
    if (config.isMember("server"))
        {
        auto const& server_config = config["server"];
        if (!(server_config.isMember("type") && server_config.isMember("settings")))
            {
            assert(!"Type and settings must be defined in the config file");
            }
        auto const& server_type = server_config["type"].asString();
        if (server_type == "rds")
            {
            m_location = ServerLocation::RDS;
            m_commMethod = CommMethod::CURL;
            }
        else if (server_type == "local")
            {
            m_location = ServerLocation::LOCAL;
            m_commMethod = CommMethod::CURL;
            }
        else if (server_type == "azure")
            {
            m_location = ServerLocation::AZURE;
            m_commMethod = CommMethod::CURL;
            }
        else
            {
            }
        auto const& server_settings = server_config["settings"];
        if (server_settings.isMember("id"))
            {
            m_serverID = server_settings["id"].asCString();
            }
        if (server_settings.isMember("url"))
            {
            m_url = server_settings["url"].asCString();
            }
        if (server_settings.isMember("authentication"))
            {
            auto const& auth = server_settings["authentication"];
            if (auth.isMember("public") && auth["public"] == true)
                {
                m_public = true;
                s_stream_from_wsg = false;
                }
            else
                {
                s_stream_from_wsg = true;
                }
            }
        }
    }


template <class EXTENT> SMStreamingStore<EXTENT>::SMStreamingStore(const WString& path, bool compress, bool areNodeHeadersGrouped, bool isVirtualGrouping, WString headers_path, FormatType formatType)
    : SMSQLiteSisterFile(nullptr),
     m_pathToHeaders(headers_path.c_str()),
     m_use_node_header_grouping(areNodeHeadersGrouped),
     m_use_virtual_grouping(isVirtualGrouping),
     m_formatType(formatType)
    {
    assert(!"Must not use this constructor");
    //InitializeDataSourceAccount(dataSourceManager, path);

    if (m_pathToHeaders.empty())
        {
        // Set default path to headers relative to root directory
        m_pathToHeaders = s_stream_using_cesium_3d_tiles_format ? L""/*L"data"*/ : L"headers";

        if (m_use_node_header_grouping && m_use_virtual_grouping)
            {
            m_NodeHeaderFetchDistributor = new SMNodeDistributor<SMNodeGroup::DistributeData>();
            SMNodeGroup::SetWorkTo(*m_NodeHeaderFetchDistributor);
            }
        }

    m_pathToHeaders.setSeparator(GetDataSourceAccount()->getPrefixPath().getSeparator());

    m_transform.InitIdentity();

    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    //                           and do this in the Cloud API...
    //if (s_stream_from_disk)
    //    {
    //    // Create base directory structure to store information if not already done
    //    BeFileName path (m_dataSourceAccount->getPrefixPath().c_str());
    //    path.AppendToPath(m_pathToHeaders.c_str());
    //    BeFileNameStatus createStatus = BeFileName::CreateNewDirectory(path);
    //    assert(createStatus == BeFileNameStatus::Success || createStatus == BeFileNameStatus::AlreadyExists);
    //    }
    }

template <class EXTENT> SMStreamingStore<EXTENT>::SMStreamingStore(const SMStreamingSettingsPtr& settings)
    : SMSQLiteSisterFile(nullptr),
      m_settings(settings)
    {
    m_transform.InitIdentity();

    if (m_pathToHeaders.empty())
        {
        // Set default path to headers relative to root directory
        m_pathToHeaders = s_stream_using_cesium_3d_tiles_format ? L""/*L"data"*/ : L"headers";

        if (m_use_node_header_grouping && m_use_virtual_grouping)
            {
            m_NodeHeaderFetchDistributor = new SMNodeDistributor<SMNodeGroup::DistributeData>();
            SMNodeGroup::SetWorkTo(*m_NodeHeaderFetchDistributor);
            }
        }

    //m_pathToHeaders.setSeparator(GetDataSourceAccount()->getPrefixPath().getSeparator());

    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    //                           and do this in the Cloud API...
    //if (settings->IsLocal())
    //    {
    //    // Create base directory structure to store information if not already done
    //    BeFileName path (m_dataSourceAccount->getPrefixPath().c_str());
    //    path.AppendToPath(m_pathToHeaders.c_str());
    //    BeFileNameStatus createStatus = BeFileName::CreateNewDirectory(path);
    //    assert(createStatus == BeFileNameStatus::Success || createStatus == BeFileNameStatus::AlreadyExists);
    //    }
    }

template <class EXTENT> SMStreamingStore<EXTENT>::~SMStreamingStore()
    {
    }

template <class EXTENT> DataSourceStatus SMStreamingStore<EXTENT>::InitializeDataSourceAccount(DataSourceManager& dataSourceManager, const SMStreamingSettingsPtr& settings)
    {
    DataSourceStatus                            status;
    DataSourceAccount                       *   account;
    DataSourceAccount::AccountName              account_name;
    DataSourceURL                               account_prefix;
    DataSourceAccount::AccountIdentifier        account_identifier;
    DataSourceAccount::AccountKey               account_key;
    DataSourceService                       *   service;
    DataSourceService::ServiceName              service_name;
    std::unique_ptr<std::function<string()>> wsgCallback = nullptr;
    std::unique_ptr<std::function<string(const Utf8String& docGuid)>> sasCallback = nullptr;
    Utf8String sslCertificatePath;

    if (settings->IsLocal() && settings->IsUsingCURL())
        {
        service_name = L"DataSourceServiceCURL";
        account_name = L"LocalCURLAccount";
        BeFileName url(settings->GetURL().c_str());
        if (m_settings->IsPublishing() && !BeFileName::DoesPathExist(url.c_str())) 
            {
            BeFileName::CreateNewDirectory(url.c_str());
            }
        else
            {
            m_masterFileName = BEFILENAME(GetFileNameAndExtension, url);
            url = BeFileName(BEFILENAME(GetDirectoryName, url).c_str());
            }
        account_prefix = DataSourceURL((L"file:///" + url).c_str());
        }
    else if (settings->IsLocal())
        {
        service_name = L"DataSourceServiceFile";
        account_name = L"LocalFileAccount";
        account_prefix = DataSourceURL(settings->GetURL().c_str());
        }
    else if (!settings->IsPublic())
        {
        service_name = L"DataSourceServiceWSG";
        account_name = L"WSGAccount";
        account_prefix = DataSourceURL(settings->GetURL().c_str());
        account_identifier = settings->GetServerID().c_str();

        Utf8String tokenUtf8 = ScalableMesh::ScalableMeshLib::GetHost().GetWsgTokenAdmin().GetToken();
        assert(!tokenUtf8.empty());

        account_key = WString(tokenUtf8.c_str(), BentleyCharEncoding::Utf8).c_str(); // WSG token in this case

        sslCertificatePath = ScalableMesh::ScalableMeshLib::GetHost().GetSSLCertificateAdmin().GetSSLCertificatePath();
        assert(!sslCertificatePath.empty());

        wsgCallback.reset(new std::function<string()>([]() -> std::string
            {
            return ScalableMesh::ScalableMeshLib::GetHost().GetWsgTokenAdmin().GetToken().c_str();
            }));
        }
    else if (settings->IsDataFromRDS() && settings->IsUsingCURL())
        {
        service_name = L"DataSourceServiceAzureCURL";
        //account_name = (L"AzureCURLAccount" + settings->GetGUID()).c_str();
        account_name = L"AzureCURLAccount";
        account_key = WString(settings->GetUtf8GUID().c_str(), BentleyCharEncoding::Utf8).c_str(); // the key is the reality data guid
        BeFileName url(settings->GetURL().c_str());
        m_masterFileName = BEFILENAME(GetFileNameAndExtension, url);
        account_prefix = DataSourceURL(settings->GetGUID().c_str());
        account_prefix.append(DataSourceURL(BEFILENAME(GetDirectoryName, url).c_str()));
        account_identifier = settings->GetServerID().c_str();

        sasCallback.reset(new std::function<string(const Utf8String& docGuid)>([](const Utf8String& docGuid) -> std::string
            {
            if (ScalableMesh::ScalableMeshLib::IsInitialized())
                return ScalableMesh::ScalableMeshLib::GetHost().GetSASTokenAdmin().GetToken(docGuid).c_str();
            return std::string();
            }));
        }
    else if (settings->IsDataFromAzure() && settings->IsUsingCURL())
        {
        // NEEDS_WORK_SM_STREAMING: Add method to specify Azure CDN endpoints such as BlobEndpoint = https://scalablemesh.azureedge.net
        service_name = L"DataSourceServiceAzureCURL";
        account_name = L"AzureCURLAccount";
        assert(!"Not implemented...");
        }
    else if (settings->IsDataFromAzure())
        {
        // NEEDS_WORK_SM_STREAMING: Use WAStorage library here...
        assert(!"Not implemented...");
        }
    else
        {
        assert(!"Unknown server type for streaming");
        }

    account_name.append(L"_" + std::to_wstring(settings->GetSMID()));

    if ((service = dataSourceManager.getService(service_name)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Unknown_Service);

    // Create an account on the file service streaming
    if ((account = service->createAccount(account_name, account_identifier, account_key)) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Account_Not_Found);

    if (wsgCallback != nullptr) account->setWSGTokenGetterCallback(*wsgCallback.get());
    if (sasCallback != nullptr) account->SetSASTokenGetterCallback(*sasCallback.get());
    account->setAccountSSLCertificatePath(sslCertificatePath.c_str());
    account->setPrefixPath(account_prefix);
    auto* casted_account = dynamic_cast<DataSourceAccountWSG*>(account);
    if (casted_account != nullptr)
        {
        //casted_account->setOrganizationID(orgID);
        casted_account->setUseDirectAzureCalls(s_use_qa_azure);
        }

    if (!settings->IsLocal())
        {
        // Setup Caching service + set up local file based caching
        DataSourceService                       *   serviceCaching;
        DataSourceAccount                       *   accountCaching;
        if ((serviceCaching = dataSourceManager.getService(DataSourceService::ServiceName(L"DataSourceServiceFile"))) == nullptr)
            return DataSourceStatus(DataSourceStatus::Status_Error_Unknown_Service);

        if ((accountCaching = serviceCaching->createAccount(DataSourceAccount::AccountName(L"CacheAccount"), DataSourceAccount::AccountIdentifier(), DataSourceAccount::AccountKey())) == nullptr)
            return DataSourceStatus(DataSourceStatus::Status_Error_Account_Not_Found);

        accountCaching->setPrefixPath(DataSourceURL(L"C:\\Temp\\SMStreamingCache"));

        account->setCaching(*accountCaching, DataSourceURL());
        }

    this->SetDataSourceAccount(account);

    return DataSourceStatus();
    }

template <class EXTENT> uint64_t SMStreamingStore<EXTENT>::GetNextID() const
    {
    // NEEDS_WORK_STREAMING: do we need this for streaming?
    //assert(!"Not implemented yet");
    return 0; 
    }
            
template <class EXTENT> void SMStreamingStore<EXTENT>::Close()
    {
    }
            
template <class EXTENT> bool SMStreamingStore<EXTENT>::StoreMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader != NULL && indexHeader->m_rootNodeBlockID.IsValid())
        {
        Json::Value masterHeader;
        masterHeader["balanced"] = indexHeader->m_balanced;
        masterHeader["depth"] = (uint32_t)indexHeader->m_depth;
        masterHeader["rootNodeBlockID"] = ConvertBlockID(indexHeader->m_rootNodeBlockID);
        masterHeader["splitThreshold"] = indexHeader->m_SplitTreshold;
        masterHeader["singleFile"] = false;
        masterHeader["isTerrain"] = indexHeader->m_isTerrain;

        auto buffer = Json::StyledWriter().write(masterHeader);
        uint64_t buffer_size = buffer.size();

        DataSourceURL    dataSourceURL(L"MasterHeader.sscm");

        DataSource *dataSource = m_dataSourceAccount->getOrCreateThreadDataSource();
        assert(dataSource != nullptr);

        if (dataSource->open(dataSourceURL, DataSourceMode_Write).isFailed())
            {
            assert(false); // could not open master header data source!
            return false;
            }

        if (dataSource->write((const DataSource::Buffer*)buffer.c_str(), buffer_size).isFailed())
            {
            assert(false); // error writing to master header data source!
            return false;
            }

        if (dataSource->close().isFailed())
            {
            assert(false); // error closing master header data source!
            return false;
            }
        }

    return true;
    }
    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::LoadMasterHeader(SMIndexMasterHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader == NULL || !m_nodeHeaderGroups.empty()) return 0;

    SMGroupGlobalParameters::StrategyType groupMode = SMGroupGlobalParameters::StrategyType::NONE;
    if (s_stream_from_grouped_store) groupMode = SMGroupGlobalParameters::StrategyType::NORMAL;
    if (m_use_node_header_grouping && m_use_virtual_grouping) groupMode = SMGroupGlobalParameters::StrategyType::VIRTUAL;
    if (s_stream_using_cesium_3d_tiles_format) groupMode = SMGroupGlobalParameters::StrategyType::CESIUM;
    if (s_import_from_bim_exported_cesium_3d_tiles) groupMode = SMGroupGlobalParameters::StrategyType::BIMCESIUM;
    bool isGrouped = true;
    //wchar_t buffer[10000];
    //swprintf(buffer, m_masterFileName.c_str());
    //switch (groupMode)
    //    {
    //    case SMGroupGlobalParameters::StrategyType::NONE:
    //    {
    //    swprintf(buffer, L"MasterHeader.sscm");
    //    isGrouped = false;
    //    break;
    //    }
    //    case SMGroupGlobalParameters::StrategyType::NORMAL:
    //    {
    //    swprintf(buffer, L"MasterHeaderWith%sGroups.bin", L"");
    //    break;
    //    }
    //    case SMGroupGlobalParameters::StrategyType::VIRTUAL:
    //    {
    //    swprintf(buffer, L"MasterHeaderWith%sGroups.bin", L"Virtual");
    //    break;
    //    }
    //    case SMGroupGlobalParameters::StrategyType::CESIUM:
    //    {
    //    swprintf(buffer, L"MasterHeaderWith%sGroups%s.bin", L"Cesium", L"-compressed");
    //    break;
    //    }
    //    case SMGroupGlobalParameters::StrategyType::BIMCESIUM:
    //    {
    //    swprintf(buffer, L"graz/graz_AppData.json");
    //    break;
    //    }
    //    default:
    //    {
    //    assert(!"Unknown grouping type");
    //    return 0;
    //    }
    //    }


    if (m_settings->IsCesium3DTiles())
        {
        //headerSize = readSize;
        //Json::Value masterHeader;
        //Json::Reader    reader;
        //char* jsonBlob = reinterpret_cast<char *>(dest.get());
        //reader.parse(jsonBlob, jsonBlob + readSize, masterHeader);
        //assert(masterHeader.isMember("models") && masterHeader["models"].isMember("30") && masterHeader["models"]["30"].isMember("tilesetUrl"));
        //
        //BeFileName tilesetUrl (masterHeader["models"]["30"]["tilesetUrl"].asString());
        auto rootNodeBlockID = 0; // start node ID at 0

        indexHeader->m_SplitTreshold = 10000; // default, not used
        indexHeader->m_balanced = true; // default, should be true
        indexHeader->m_depth = 0; // default, unknown
        indexHeader->m_isTerrain = false; // default, always non terrain
        indexHeader->m_singleFile = false; // default, always multifile
        indexHeader->m_isCesiumFormat = true; // default, always cesium datasets
        indexHeader->m_rootNodeBlockID = rootNodeBlockID != ISMStore::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

        BeFileName baseUrl(m_masterFileName.c_str());
        auto tilesetDir = BEFILENAME(GetDirectoryName, baseUrl);
        auto tilesetName = BEFILENAME(GetFileNameAndExtension, baseUrl);

        SMGroupGlobalParameters::Ptr groupParameters = SMGroupGlobalParameters::Create(groupMode, this->GetDataSourceAccount());
        SMGroupCache::Ptr groupCache = SMGroupCache::Create(&m_nodeHeaderCache);
        m_CesiumGroup = SMNodeGroup::Create(groupParameters, groupCache, rootNodeBlockID);
        m_CesiumGroup->DeclareRoot();
        m_CesiumGroup->SetURL(DataSourceURL(tilesetName.c_str()));
        m_CesiumGroup->SetDataSourcePrefix(tilesetDir);
        m_CesiumGroup->ResetNodeIDGenerator();
        m_CesiumGroup->DownloadNodeHeader(indexHeader->m_rootNodeBlockID.m_integerID);
        Json::Value* masterJSONPtr = nullptr;
        if ((masterJSONPtr = m_CesiumGroup->GetSMMasterHeaderInfo()) != nullptr)
            {
            // Override defaults by given values
            auto const& masterJSON = *masterJSONPtr;
            indexHeader->m_SplitTreshold = masterJSON["SplitTreshold"].asUInt();
            indexHeader->m_balanced = masterJSON["Balanced"].asBool();
            indexHeader->m_depth = masterJSON["Depth"].asUInt();
            indexHeader->m_isTerrain = masterJSON["IsTerrain"].asBool();
            indexHeader->m_terrainDepth = masterJSON["MeshDataDepth"].asUInt();
            indexHeader->m_resolution = masterJSON["DataResolution"].asDouble();
            }
        //Utf8String wkt;
        //m_CesiumGroup->GetWKTString(wkt);
        //m_settings->SetGCSString(wkt);
        }
    else
        {
        std::unique_ptr<DataSource::Buffer[]>            dest;
        DataSource                                *      dataSource;
        DataSource::DataSize                             readSize;
        DataSourceBuffer::BufferSize                     destSize = 20 * 1024 * 1024;
        DataSourceURL dataSourceURL(m_masterFileName.c_str());

        dataSource = this->InitializeDataSource(dest, destSize);
        if (dataSource == nullptr)
            {
            assert(false); // problem initializing a datasource
            return 0;
            }

        if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
            {
            assert(false); // problem opening a datasource
            return 0;
            }

        if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
            {
            assert(false); // problem reading a datasource
            return 0;
            }

        dataSource->close();

        this->GetDataSourceAccount()->destroyDataSource(dataSource);
        if (isGrouped)
            {
            // initialize codec
            bvector<uint8_t> masterHeader(decltype(readSize)(*reinterpret_cast<decltype(readSize)*>(dest.get())));
            HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(readSize - sizeof(readSize));
            const size_t computedDataSize = pCodec->DecompressSubset(dest.get() + sizeof(readSize),
                readSize - sizeof(readSize),
                masterHeader.data(),
                masterHeader.size());
            assert(computedDataSize != 0 && computedDataSize == masterHeader.size());

            dest.reset(new uint8_t[masterHeader.size()]);
            memcpy(dest.get(), masterHeader.data(), masterHeader.size());
            headerSize = masterHeader.size();

            size_t position = 0;

            uint32_t sizeOfOldMasterHeaderPart;
            memcpy(&sizeOfOldMasterHeaderPart, dest.get() + position, sizeof(sizeOfOldMasterHeaderPart));
            position += sizeof(sizeOfOldMasterHeaderPart);
            assert(sizeOfOldMasterHeaderPart == sizeof(SQLiteIndexHeader));

            SQLiteIndexHeader oldMasterHeader;
            memcpy(&oldMasterHeader, dest.get() + position, sizeof(SQLiteIndexHeader));
            position += sizeof(SQLiteIndexHeader);
            indexHeader->m_SplitTreshold = oldMasterHeader.m_SplitTreshold;
            indexHeader->m_balanced = oldMasterHeader.m_balanced;
            indexHeader->m_depth = oldMasterHeader.m_depth;
            indexHeader->m_isTerrain = oldMasterHeader.m_isTerrain;
            indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
            assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
            indexHeader->m_isCesiumFormat = groupMode == SMGroupGlobalParameters::StrategyType::CESIUM;

            auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
            //auto group = this->GetGroup(HPMBlockID(rootNodeBlockID));
            SMGroupGlobalParameters::Ptr groupParameters = SMGroupGlobalParameters::Create(groupMode, this->GetDataSourceAccount());
            SMGroupCache::Ptr groupCache = SMGroupCache::Create(&m_nodeHeaderCache);
            m_CesiumGroup = SMNodeGroup::Create(groupParameters, groupCache, rootNodeBlockID);
            m_CesiumGroup->DeclareRoot();
            m_CesiumGroup->SetURL(L"n_0.json");
            m_CesiumGroup->SetDataSourcePrefix(L"data");

            indexHeader->m_rootNodeBlockID = rootNodeBlockID != ISMStore::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

            short storedGroupMode = m_use_virtual_grouping;
            memcpy(&storedGroupMode, reinterpret_cast<char *>(dest.get()) + position, sizeof(storedGroupMode));
            assert((storedGroupMode == SMGroupGlobalParameters::StrategyType::VIRTUAL) == s_is_virtual_grouping); // Trying to load streaming master header with incoherent grouping strategies
            position += sizeof(storedGroupMode);

            //m_nodeHeaderGroups.push_back(group);
            //for (auto childGroup : group->GetChildGroups())
            //    {
            //    m_nodeHeaderGroups.push_back(childGroup.second);
            //    }

            //for (auto headerPtr : group->groupGetJsonNodeHeaders())
            //    {
            //    auto result = m_nodeHeaderCache.insert(std::pair<uint32_t, Json::Value>(headerPtr.first, *headerPtr.second));
            //    assert(result.second == true);
            //    }
            //
            //for (auto childGroup : group->GetChildGroups())
            //    {
            //    for (auto headerPtr : childGroup->groupGetJsonNodeHeaders())
            //        {
            //        auto result = m_nodeHeaderCache.insert(std::pair<uint32_t, Json::Value>(headerPtr.first, *headerPtr.second));
            //        assert(result.second == true);
            //        }
            //    }

            //// Parse rest of file -- group information
            //while (position < headerSize)
            //    {
            //    uint64_t group_id;
            //        uint32_t group_id_tmp;
            //        memcpy(&group_id_tmp, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_id_tmp));
            //        position += sizeof(group_id_tmp);
            //        group_id = group_id_tmp;

            //    uint64_t group_totalSizeOfHeaders(0);
            //    if (storedGroupMode == SMNodeGroup::StrategyType::VIRTUAL)
            //        {
            //        memcpy(&group_totalSizeOfHeaders, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_totalSizeOfHeaders));
            //        position += sizeof(group_totalSizeOfHeaders);
            //        }

            //    size_t group_numNodes;
            //    memcpy(&group_numNodes, reinterpret_cast<char *>(dest.get()) + position, sizeof(group_numNodes));
            //    position += sizeof(group_numNodes);

            //    auto group = SMNodeGroupPtr(new SMNodeGroup(this->GetDataSourceAccount(),
            //                                                     (uint32_t)group_id,
            //                                                     SMNodeGroup::StrategyType(storedGroupMode),
            //                                                     group_numNodes,
            //                                                     group_totalSizeOfHeaders));

            //    // NEEDS_WORK_SM_STREAMING : group datasource doesn't need to depend on type of grouping
            //    switch (storedGroupMode)
            //        {
            //        case SMNodeGroup::StrategyType::NORMAL:
            //            {
            //            group->SetDataSourcePrefix((m_pathToHeaders + L"\\g\\g_").c_str());
            //            break;
            //            }
            //        case SMNodeGroup::StrategyType::VIRTUAL:
            //            {
            //            group->SetDataSourcePrefix((m_pathToHeaders + L"\\n_").c_str());
            //            break;
            //            }
            //        case SMNodeGroup::StrategyType::CESIUM:
            //            {
            //            if (s_stream_from_wsg)
            //                {
            //                group->SetDataSourcePrefix(L"n_");
            //                }
            //            else
            //                {
            //                group->SetDataSourcePrefix(L"data\\n_");
            //                //group->SetDataSourcePrefix(L"n_");
            //                }
            //            group->SetDataSourceExtension(L".json");
            //            break;
            //            }
            //        default:
            //            {
            //            assert(!"Unknown grouping type");
            //            return 0;
            //            }
            //        }
            //    group->SetDistributor(*m_NodeHeaderFetchDistributor);
            //    m_nodeHeaderGroups.push_back(group);

            //    vector<uint64_t> nodeIds(group_numNodes);
            //    memcpy(nodeIds.data(), reinterpret_cast<char *>(dest.get()) + position, group_numNodes * sizeof(uint64_t));
            //    position += group_numNodes * sizeof(uint64_t);

            //    group->GetHeader()->resize(group_numNodes);
            //    transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId)
            //        {
            //        return SMNodeHeader{ nodeId, uint32_t(-1), 0 };
            //        });
            //    }
            }
        else
            {
            Json::Reader    reader;
            Json::Value     masterHeader;

            headerSize = readSize;

            if (!reader.parse(reinterpret_cast<char *>(dest.get()), reinterpret_cast<char *>(&(dest.get()[readSize])), masterHeader)
                || !masterHeader.isMember("rootNodeBlockID"))
                {
                assert(false); // error reading Master Header
                return 0;
                }
            indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
            indexHeader->m_balanced = masterHeader["balanced"].asBool();
            indexHeader->m_depth = masterHeader["depth"].asUInt();
            // NEW_SSTORE_RB Temporary fix until terrain is correctly implemented for streaming
            indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();

            auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
            indexHeader->m_rootNodeBlockID = rootNodeBlockID != ISMStore::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
            indexHeader->m_isCesiumFormat = masterHeader.isMember("fileFormat") && masterHeader["fileFormat"].asString() == "Cesium3DTiles";
            /* Needed?
                        if (masterHeader.isMember("singleFile"))
                            {
                            indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                            HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
                            }
            */
            }
        }

    return headerSize;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToBinary(const SMIndexNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, size_t& po_pDataSize)
    {
    assert(po_pBinaryData == nullptr && po_pDataSize == 0);

    po_pBinaryData.reset(new Byte[3000]);

    const auto filtered = pi_pHeader->m_filtered;
    memcpy(po_pBinaryData.get() + po_pDataSize, &filtered, sizeof(filtered));
    po_pDataSize += sizeof(filtered);
    const auto parentBlockID = pi_pHeader->m_parentNodeID.IsValid() ? ConvertBlockID(pi_pHeader->m_parentNodeID) : ISMStore::GetNullNodeID();
    memcpy(po_pBinaryData.get() + po_pDataSize, &parentBlockID, sizeof(parentBlockID));
    po_pDataSize += sizeof(parentBlockID);
    const auto subNodeNoSplitID = pi_pHeader->m_SubNodeNoSplitID.IsValid() ? ConvertBlockID(pi_pHeader->m_SubNodeNoSplitID) : ISMStore::GetNullNodeID();
    memcpy(po_pBinaryData.get() + po_pDataSize, &subNodeNoSplitID, sizeof(subNodeNoSplitID));
    po_pDataSize += sizeof(subNodeNoSplitID);
    const auto level = pi_pHeader->m_level;
    memcpy(po_pBinaryData.get() + po_pDataSize, &level, sizeof(level));
    po_pDataSize += sizeof(level);
    const auto isBranched = pi_pHeader->m_IsBranched;
    memcpy(po_pBinaryData.get() + po_pDataSize, &isBranched, sizeof(isBranched));
    po_pDataSize += sizeof(isBranched);
    const auto isLeaf = pi_pHeader->m_IsLeaf;
    memcpy(po_pBinaryData.get() + po_pDataSize, &isLeaf, sizeof(isLeaf));
    po_pDataSize += sizeof(isLeaf);
    const auto splitThreshold = pi_pHeader->m_SplitTreshold;
    memcpy(po_pBinaryData.get() + po_pDataSize, &splitThreshold, sizeof(splitThreshold));
    po_pDataSize += sizeof(splitThreshold);
    const auto totalCount = pi_pHeader->m_totalCount;
    memcpy(po_pBinaryData.get() + po_pDataSize, &totalCount, sizeof(totalCount));
    po_pDataSize += sizeof(totalCount);
    const auto nodeCount = pi_pHeader->m_nodeCount;
    memcpy(po_pBinaryData.get() + po_pDataSize, &nodeCount, sizeof(nodeCount));
    po_pDataSize += sizeof(nodeCount);
    const auto arePoints3d = pi_pHeader->m_arePoints3d;
    memcpy(po_pBinaryData.get() + po_pDataSize, &arePoints3d, sizeof(arePoints3d));
    po_pDataSize += sizeof(arePoints3d);
    const auto isTextured = pi_pHeader->m_isTextured;
    memcpy(po_pBinaryData.get() + po_pDataSize, &isTextured, sizeof(isTextured));
    po_pDataSize += sizeof(isTextured);
    const auto nbFaceIndexes = pi_pHeader->m_nbFaceIndexes;
    memcpy(po_pBinaryData.get() + po_pDataSize, &nbFaceIndexes, sizeof(nbFaceIndexes));
    po_pDataSize += sizeof(nbFaceIndexes);
    const auto graphID = pi_pHeader->m_graphID.IsValid() ? ConvertBlockID(pi_pHeader->m_graphID) : ISMStore::GetNullNodeID();
    memcpy(po_pBinaryData.get() + po_pDataSize, &graphID, sizeof(graphID));
    po_pDataSize += sizeof(graphID);

    memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_nodeExtent, 6 * sizeof(double));
    po_pDataSize += 6 * sizeof(double);

    const auto contentExtentDefined = pi_pHeader->m_contentExtentDefined;
    memcpy(po_pBinaryData.get() + po_pDataSize, &contentExtentDefined, sizeof(contentExtentDefined));
    po_pDataSize += sizeof(contentExtentDefined);
    if (contentExtentDefined)
        {
        memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_contentExtent, 6 * sizeof(double));
        po_pDataSize += 6 * sizeof(double);
        }

    /* Indice IDs */
    const auto idx = pi_pHeader->m_ptsIndiceID[0].IsValid() ? ConvertBlockID(pi_pHeader->m_ptsIndiceID[0]) : ISMStore::GetNullNodeID();
    memcpy(po_pBinaryData.get() + po_pDataSize, &idx, sizeof(idx));
    po_pDataSize += sizeof(idx);


    /* Mesh components and clips */
    const auto numberOfMeshComponents = pi_pHeader->m_numberOfMeshComponents;
    memcpy(po_pBinaryData.get() + po_pDataSize, &numberOfMeshComponents, sizeof(numberOfMeshComponents));
    po_pDataSize += sizeof(numberOfMeshComponents);
    for (size_t componentIdx = 0; componentIdx < pi_pHeader->m_numberOfMeshComponents; componentIdx++)
        {
        const auto component = pi_pHeader->m_meshComponents[componentIdx];
        memcpy(po_pBinaryData.get() + po_pDataSize, &component, sizeof(component));
        po_pDataSize += sizeof(component);
        }

    const auto nbClipSetsIDs = (uint32_t)pi_pHeader->m_clipSetsID.size();
    memcpy(po_pBinaryData.get() + po_pDataSize, &nbClipSetsIDs, sizeof(nbClipSetsIDs));
    po_pDataSize += sizeof(nbClipSetsIDs);
    for (size_t i = 0; i < nbClipSetsIDs; ++i)
        {
        const auto clip = ConvertNeighborID(pi_pHeader->m_clipSetsID[i]);
        memcpy(po_pBinaryData.get() + po_pDataSize, &clip, sizeof(clip));
        po_pDataSize += sizeof(clip);
        }

    /* Children and Neighbors */
    const auto nbChildren = isLeaf || (!isBranched  && !pi_pHeader->m_SubNodeNoSplitID.IsValid()) ? 0 : (!isBranched ? 1 : pi_pHeader->m_numberOfSubNodesOnSplit);
    memcpy(po_pBinaryData.get() + po_pDataSize, &nbChildren, sizeof(nbChildren));
    po_pDataSize += sizeof(nbChildren);
    for (size_t childInd = 0; childInd < nbChildren; childInd++)
        {
        const auto id = ConvertChildID(pi_pHeader->m_apSubNodeID[childInd]);
        memcpy(po_pBinaryData.get() + po_pDataSize, &id, sizeof(id));
        po_pDataSize += sizeof(id);
        }

    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        const auto numNeighbors = pi_pHeader->m_apNeighborNodeID[neighborPosInd].size();
        memcpy(po_pBinaryData.get() + po_pDataSize, &numNeighbors, sizeof(numNeighbors));
        po_pDataSize += sizeof(numNeighbors);
        for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
            {
            const auto nodeId = ConvertNeighborID(pi_pHeader->m_apNeighborNodeID[neighborPosInd][neighborInd]);
            memcpy(po_pBinaryData.get() + po_pDataSize, &nodeId, sizeof(nodeId));
            po_pDataSize += sizeof(nodeId);
            }
        }
    auto nbDataSizes = pi_pHeader->m_blockSizes.size();
    memcpy(po_pBinaryData.get() + po_pDataSize, &nbDataSizes, sizeof(nbDataSizes));
    po_pDataSize += sizeof(nbDataSizes);

    memcpy(po_pBinaryData.get() + po_pDataSize, pi_pHeader->m_blockSizes.data(), nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
    po_pDataSize += (uint32_t)(nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToCesium3DTile(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, std::unique_ptr<Byte>& po_pBinaryData, size_t& po_pDataSize) const
    {
#ifndef VANCOUVER_API
    // Get Cesium 3D tiles required properties
    Json::Value tile;
    tile["asset"]["version"] = "0.0";

    auto& rootTile = tile["root"];
    SMStreamingStore<EXTENT>::SerializeHeaderToCesium3DTileJSON(header, blockID, rootTile);

    Json::Value& children = rootTile["children"];
    for (auto& childExtent : header->m_childrenExtents)
        {
        Json::Value child;
        child["content"]["url"] = Utf8String(("n_" + std::to_string(childExtent.first) + ".json").c_str());
        TilePublisher::WriteBoundingVolume(child, childExtent.second);
        children.append(child);
        }
    rootTile["children"] = children;

    auto utf8Node = Json::FastWriter().write(tile);
    po_pBinaryData.reset(new Byte[utf8Node.size()]);
    memcpy(po_pBinaryData.get(), utf8Node.data(), utf8Node.size());
    po_pDataSize = (uint32_t)utf8Node.size();
#else
    assert(!"Not implemented");
#endif
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToCesium3DTileJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& tile)
    {
    // compute node tolerance (for the geometric error)
    // Different attempts to compute the Cesium 3D tiles "geometric error" value:
    //DVec3d      diagonal = DVec3d::FromStartEnd(cesiumRange.low, cesiumRange.high);
    //if (!useContentExtent) diagonal.SetComponent(0.0, 2);
    //double tolerance = cesiumRange.Volume() / max<int64_t>((int64_t)header->m_nodeCount, 100000);
    //double tolerance = diagonal.Magnitude() / 1000;

    // But looking at other Cesium 3D tiles datasets (Marseille, Orlando) the computed tolerance seem to be something like this
    // and seems to work reasonably well as long as the bounding volume tightly fits the data :
    double tolerance = header->m_geometricResolution > 0 ? header->m_geometricResolution : (header->m_geometryResolution == -1 ? 64 / pow(2, header->m_level) : header->m_geometryResolution); // SM_NEEDS_WORK : Is this going to work for all datasets? What value should this be?

    assert(tolerance > 0);
    // SM_NEEDS_WORK : is there a transformation to apply at some point?
    //transformDbToTile.Multiply(transformedRange, transformedRange);

    DRange3d tileRange = header->m_nodeExtent;
    tile["refine"] = "replace";
    tile["geometricError"] = tolerance;
    TilePublisher::WriteBoundingVolume(tile, tileRange);

    if (header->m_nodeCount > 0)
        {
        if (header->m_contentExtentDefined)
            {
            DRange3d contentRange = !header->m_contentExtent.IsNull() ? header->m_contentExtent : header->m_nodeExtent;
            TilePublisher::WriteBoundingVolume(tile["content"], contentRange);
            }
        tile["content"]["url"] = Utf8String(("p_" + std::to_string(blockID.m_integerID) + ".b3dm").c_str());
        }



    // SM_NEEDS_WORK_STREAMING : Get scalable mesh required properties
    Json::Value smHeader;
    SMStreamingStore<EXTENT>::SerializeHeaderToJSON(header, blockID, smHeader);

    tile["SMHeader"] = smHeader;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SerializeHeaderToJSON(const SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block)
    {
    block["id"] = ConvertBlockID(blockID);
    block["level"] = (ISMStore::NodeID)header->m_level;
    block["parentID"] = header->m_parentNodeID.IsValid() ? ConvertBlockID(header->m_parentNodeID) : ISMStore::GetNullNodeID();
    //block["isLeaf"] = header->m_IsLeaf;
    //block["isBranched"] = header->m_IsBranched;

    //size_t nbChildren = header->m_IsLeaf || (!header->m_IsBranched  && !header->m_SubNodeNoSplitID.IsValid()) ? 0 : (!header->m_IsBranched ? 1 : header->m_numberOfSubNodesOnSplit);

    //block["nbChildren"] = nbChildren;

    //auto& children = block["children"];

    //if (nbChildren > 1)
    //    {
    //    for (size_t childInd = 0; childInd < nbChildren; childInd++)
    //        {
    //        Json::Value& child = childInd >= children.size() ? children.append(Json::Value()) : children[(int)childInd];
    //        child["index"] = (uint8_t)childInd;
    //        child["id"] = header->m_apSubNodeID[childInd].IsValid() ? ConvertChildID(header->m_apSubNodeID[childInd]) : ISMStore::GetNullNodeID();
    //        }
    //    }
    //else if (nbChildren == 1)
    //    {
    //    Json::Value& child = children.empty() ? children.append(Json::Value()) : children[0];
    //    child["index"] = 0;
    //    child["id"] = header->m_SubNodeNoSplitID.IsValid() ? ConvertChildID(header->m_SubNodeNoSplitID) : ConvertChildID(header->m_apSubNodeID[0]);
    //    }
    //
    //auto& neighbors = block["neighbors"];
    //int neighborInfoInd = 0;
    //for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
    //    {
    //    for (size_t neighborInd = 0; neighborInd < header->m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
    //        {
    //        Json::Value& neighbor = (uint32_t)neighborInfoInd >= neighbors.size() ? neighbors.append(Json::Value()) : neighbors[(uint32_t)neighborInfoInd];
    //        neighbor["nodePos"] = (uint8_t)neighborPosInd;
    //        neighbor["nodeId"] = ConvertNeighborID(header->m_apNeighborNodeID[neighborPosInd][neighborInd]);
    //        neighborInfoInd++;
    //        }
    //    }
    //
    //auto& extent = block["nodeExtent"];
    //
    //extent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
    //extent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
    //extent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
    //extent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
    //extent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
    //extent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);
    //
    //if (header->m_contentExtentDefined)
    //    {
    //    block["contentExtentDefined"] = true;
    //    auto& contentExtent = block["contentExtent"];
    //    contentExtent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
    //    contentExtent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
    //    contentExtent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
    //    contentExtent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
    //    contentExtent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
    //    contentExtent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
    //    }
    //else
    //    {
    //    block["contentExtentDefined"] = false;
    //    }


    block["totalCount"] = header->m_totalCount;
    block["nodeCount"] = header->m_nodeCount;
    block["arePoints3d"] = header->m_arePoints3d;

    block["nbFaceIndexes"] = header->m_nbFaceIndexes;
    block["nbIndiceID"] = (int)header->m_ptsIndiceID.size();

    auto& indiceID = block["indiceID"];
    Json::Value& indice = indiceID.append(Json::Value());
    indice = header->m_ptsIndiceID[0].IsValid() ? ConvertBlockID(header->m_ptsIndiceID[0]) : ISMStore::GetNullNodeID();
    //for (size_t i = 0; i < header->m_ptsIndiceID.size(); i++)
    //    {
    //    Json::Value& indice = (uint32_t)i >= indiceID.size() ? indiceID.append(Json::Value()) : indiceID[(uint32_t)i];
    //    indice = header->m_ptsIndiceID[i].IsValid() ? ConvertBlockID(header->m_ptsIndiceID[i]) : ISMStore::GetNullNodeID();
    //    }

    if (header->m_isTextured /*&& !header->m_textureID.empty() && IsValidID(header->m_textureID[0])*/)
        {
        block["areTextured"] = true;
        /*block["nbTextureIDs"] = (int)header->m_textureID.size();
        auto& textureIDs = block["textureIDs"];
        for (size_t i = 0; i < header->m_textureID.size(); i++)
            {
            auto convertedID = ConvertBlockID(header->m_textureID[i]);
            if (convertedID != ISMStore::GetNullNodeID())
                {
                Json::Value& textureID = (uint32_t)i >= textureIDs.size() ? textureIDs.append(Json::Value()) : textureIDs[(uint32_t)i];
                textureID = header->m_textureID[i].IsValid() ? convertedID : ISMStore::GetNullNodeID();
                }
            }*/
        block["uvID"] = header->m_uvID.IsValid() ? ConvertBlockID(header->m_uvID) : ISMStore::GetNullNodeID();

        block["nbUVIDs"] = (int)header->m_uvsIndicesID.size();
        auto& uvIndiceIDs = block["uvIndiceIDs"];
        for (size_t i = 0; i < header->m_uvsIndicesID.size(); i++)
            {
            Json::Value& uvIndice = (uint32_t)i >= uvIndiceIDs.size() ? uvIndiceIDs.append(Json::Value()) : uvIndiceIDs[(uint32_t)i];
            uvIndice = header->m_uvsIndicesID[i].IsValid() ? ConvertBlockID(header->m_uvsIndicesID[i]) : ISMStore::GetNullNodeID();
            }
        }
    else {
        block["areTextured"] = false;
        }

    if (header->m_clipSetsID.size() > 0)
        {
        auto& clipSetsID = block["clipSetsID"];
        for (size_t i = 0; i < header->m_clipSetsID.size(); ++i)
            {
            auto& clip = (uint32_t)i >= clipSetsID.size() ? clipSetsID.append(Json::Value()) : clipSetsID[(uint32_t)i];
            clip = ConvertNeighborID(header->m_clipSetsID[i]);
            }
        }
    block["nbClipSets"] = (uint32_t)header->m_clipSetsID.size();
    }

    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::StoreNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)
    {
    size_t headerSize = 0;
    std::unique_ptr<Byte> headerData = nullptr;
    std::wstring extension;
    switch (m_formatType)
        {
        case FormatType::Binary:
            {
            extension = L".bin";
            SerializeHeaderToBinary(header, headerData, headerSize);
            break;
            }
        case FormatType::Json:
            {
            extension = L".json";
            Json::Value block;
            SerializeHeaderToJSON(header, blockID, block);
            auto utf8Block = Json::FastWriter().write(block);
            headerData.reset(new Byte[utf8Block.size()]);
            memcpy(headerData.get(), utf8Block.data(), utf8Block.size());
            break;
            }
        case FormatType::Cesium3DTiles:
            {
            extension = L".json";
            SerializeHeaderToCesium3DTile(header, blockID, headerData, headerSize);
            break;
            }
        default:
            assert(false); // unknown format type for streaming
        }

    DataSourceURL dataSourceURL = m_pathToHeaders;
    dataSourceURL.append(DataSourceURL(L"n_" + std::to_wstring(blockID.m_integerID) + extension));

    DataSource *dataSource = this->GetDataSourceAccount()->createDataSource();
    if (dataSource == nullptr)
        {
        assert(false); // problem creating new datasource
        return 0;
        }
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
    //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
    //}

    if (dataSource->open(dataSourceURL, DataSourceMode_Write).isFailed())
        {
        assert(false); // problem opening a datasource
        return 0;
        }

    if (dataSource->write(headerData.get(), headerSize).isFailed())
        {
        assert(false); // problem writing a datasource
        return 0;
        }

    if (dataSource->close().isFailed())
        {
        assert(false); // problem closing a datasource
        return 0;
        }

    this->GetDataSourceAccount()->destroyDataSource(dataSource);

    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
    //}

    return 1;
    }
    
template <class EXTENT> size_t SMStreamingStore<EXTENT>::LoadNodeHeader(SMIndexNodeHeader<EXTENT>* header, HPMBlockID blockID)            
    {
    if (s_stream_from_grouped_store)
        {
        if (s_stream_using_cesium_3d_tiles_format || s_import_from_bim_exported_cesium_3d_tiles)
            {
            auto jsonHeader = m_CesiumGroup->DownloadNodeHeader(blockID.m_integerID);
            if (jsonHeader == nullptr || jsonHeader->isNull())
                {
                // return a valid (empty) node header
                header->m_apSubNodeID.clear();
                header->m_nodeExtent = ExtentOp<EXTENT>::Create(0, 0, 0, 0, 0, 0);
                header->m_contentExtent = header->m_nodeExtent;
                header->m_totalCount = 0;
                return 1;
                }
            ReadNodeHeaderFromJSON(header, *jsonHeader);
            }
        else
            {
            auto group = this->GetGroup(blockID);
            auto node_header = group->GetNodeHeader(blockID.m_integerID);
            ReadNodeHeaderFromBinary(header, group->GetRawHeaders(node_header->offset), node_header->size);
            header->m_id = blockID;
            //group->removeNodeData(blockID.m_integerID);
            }
        }
    else if (s_stream_using_cesium_3d_tiles_format)
        {
        size_t headerSize = 0;
        std::unique_ptr<Byte> headerData = nullptr;
        this->GetNodeHeaderBinary(blockID, headerData, headerSize);
        if (!headerData && headerSize == 0) return 1;

        Json::Reader    reader;
        Json::Value     cesiumHeader;
        if (!reader.parse(reinterpret_cast<char *>(headerData.get()), reinterpret_cast<char *>(&(headerData.get()[headerSize])), cesiumHeader)
            || !(cesiumHeader.isMember("root") && cesiumHeader["root"].isMember("SMHeader")))
            {
            assert(false); // error reading Master Header
            return 0;
            }

        this->ReadNodeHeaderFromJSON(header, cesiumHeader["root"]["SMHeader"]);
        }
    else {
        //auto nodeHeader = this->GetNodeHeaderJSON(blockID);
        //ReadNodeHeaderFromJSON(header, nodeHeader);
        size_t headerSize = 0;
        std::unique_ptr<Byte> headerData = nullptr;
        this->GetNodeHeaderBinary(blockID, headerData, headerSize);
        if (!headerData && headerSize == 0) return 0;
        ReadNodeHeaderFromBinary(header, headerData.get(), headerSize);
        }
    header->m_id = blockID;
    return 1;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::SetProjectFilesPath(BeFileName& projectFilesPath)
    {
    return SMSQLiteSisterFile::SetProjectFilesPath(projectFilesPath);
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SaveProjectFiles()
    {
    __super::SaveSisterFiles();
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::CompactProjectFiles()
{
	__super::Compact();
}

template <class EXTENT> void SMStreamingStore<EXTENT>::PreloadData(const bvector<DRange3d>& tileRanges) 
    {
    assert(!"No implemented yet");
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::CancelPreloadData()
    {
    assert(!"No implemented yet");
    }

template<class EXTENT> void SMStreamingStore<EXTENT>::Register(const uint64_t & smID)
    {
    m_settings->SetSMID(smID);
    InitializeDataSourceAccount(*DataSourceManager::Get(), m_settings);
    }

template<class EXTENT> void SMStreamingStore<EXTENT>::Unregister(const uint64_t & smID)
    {
    auto const& account = GetDataSourceAccount();
    if (account)
        {
        DataSourceManager::Get()->getService(account->getServiceName())->destroyAccount(account->getAccountName());
        }
    }

template <class EXTENT> SMNodeGroupPtr SMStreamingStore<EXTENT>::FindGroup(HPMBlockID blockID)
    {
    auto nodeIDToFind = ConvertBlockID(blockID);
    for (auto& group : m_nodeHeaderGroups)
        {
        if (group->ContainsNode(nodeIDToFind))
            {
            return group;
            }
        }

    return nullptr;
    }

template <class EXTENT> SMNodeGroupPtr SMStreamingStore<EXTENT>::GetGroup(HPMBlockID blockID)
    {
    auto group = this->FindGroup(blockID);
    if (group == nullptr) 
        {
        assert(m_nodeHeaderGroups.empty());
        group = m_CesiumGroup;
        //// create first group
        //group = SMNodeGroup::CreateCesium3DTilesGroup(this->GetDataSourceAccount(), 0);
        //group->SetURL(L"n_0.json");
        //group->SetDataSourcePrefix(L"data");
        ////group->SetDataSourceExtension(L".json");
        }
    if (!group->IsLoaded())
        {
        group->Load(blockID.m_integerID);
        }
#ifdef DEBUG_STREAMING_DATA_STORE
    else {
        static std::atomic<uint64_t> s_NbAlreadyLoadedNodes = 0;
        s_NbAlreadyLoadedNodes += 1;
        {
        std::lock_guard<mutex> clk(s_consoleMutex);
        std::cout << "[" << blockID.m_integerID << ", " << group->GetID() << "] already loaded in temporary streaming data cache!" << std::endl;
        }
        }
#endif
    return group;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::ReadNodeHeaderFromBinary(SMIndexNodeHeader<EXTENT>* header, uint8_t* headerData, size_t& maxCountData) const
    {
    size_t dataIndex = 0;

    memcpy(&header->m_filtered, headerData + dataIndex, sizeof(header->m_filtered));
    dataIndex += sizeof(header->m_filtered);
    uint32_t parentNodeID;
    memcpy(&parentNodeID, headerData + dataIndex, sizeof(parentNodeID));
    header->m_parentNodeID = parentNodeID != ISMStore::GetNullNodeID() ? HPMBlockID(parentNodeID) : ISMStore::GetNullNodeID();
    dataIndex += sizeof(parentNodeID);
    uint32_t subNodeNoSplitID;
    memcpy(&subNodeNoSplitID, headerData + dataIndex, sizeof(subNodeNoSplitID));
    header->m_SubNodeNoSplitID = subNodeNoSplitID != ISMStore::GetNullNodeID() ? HPMBlockID(subNodeNoSplitID) : ISMStore::GetNullNodeID();
    dataIndex += sizeof(subNodeNoSplitID);
    memcpy(&header->m_level, headerData + dataIndex, sizeof(header->m_level));
    dataIndex += sizeof(header->m_level);
    memcpy(&header->m_IsBranched, headerData + dataIndex, sizeof(header->m_IsBranched));
    dataIndex += sizeof(header->m_IsBranched);
    memcpy(&header->m_IsLeaf, headerData + dataIndex, sizeof(header->m_IsLeaf));
    dataIndex += sizeof(header->m_IsLeaf);
    memcpy(&header->m_SplitTreshold, headerData + dataIndex, sizeof(header->m_SplitTreshold));
    dataIndex += sizeof(header->m_SplitTreshold);
    memcpy(&header->m_totalCount, headerData + dataIndex, sizeof(header->m_totalCount));
    dataIndex += sizeof(header->m_totalCount);
    memcpy(&header->m_nodeCount, headerData + dataIndex, sizeof(header->m_nodeCount));
    dataIndex += sizeof(header->m_nodeCount);
    memcpy(&header->m_arePoints3d, headerData + dataIndex, sizeof(header->m_arePoints3d));
    dataIndex += sizeof(header->m_arePoints3d);
    memcpy(&header->m_isTextured, headerData + dataIndex, sizeof(header->m_isTextured));
    dataIndex += sizeof(header->m_isTextured);
    memcpy(&header->m_nbFaceIndexes, headerData + dataIndex, sizeof(header->m_nbFaceIndexes));
    dataIndex += sizeof(header->m_nbFaceIndexes);
    uint32_t graphID;
    memcpy(&graphID, headerData + dataIndex, sizeof(graphID));
    header->m_graphID = graphID != ISMStore::GetNullNodeID() ? HPMBlockID(graphID) : ISMStore::GetNullNodeID();
    dataIndex += sizeof(graphID);

    memcpy(&header->m_nodeExtent, headerData + dataIndex, 6 * sizeof(double));
    dataIndex += 6 * sizeof(double);

    memcpy(&header->m_contentExtentDefined, headerData + dataIndex, sizeof(header->m_contentExtentDefined));
    dataIndex += sizeof(header->m_contentExtentDefined);
    if (header->m_contentExtentDefined)
        {
        memcpy(&header->m_contentExtent, headerData + dataIndex, 6 * sizeof(double));
        dataIndex += 6 * sizeof(double);
        }

    /* Indices */
    uint32_t idx;
    memcpy(&idx, headerData + dataIndex, sizeof(idx));
    dataIndex += sizeof(idx);
    header->m_ptsIndiceID.resize(1);
    header->m_ptsIndiceID[0] = idx;

    /* Texture */
    if (header->m_isTextured)
        {
        header->m_textureID = HPMBlockID();
        header->m_ptsIndiceID.resize(2);
        header->m_ptsIndiceID[1] = (int)idx;
        header->m_ptsIndiceID[0] = HPMBlockID();
        header->m_nbTextures = 1;
        header->m_uvsIndicesID.resize(1);
        header->m_uvsIndicesID[0] = idx;
        }

    /* Mesh components */
    size_t numberOfMeshComponents;
    memcpy(&numberOfMeshComponents, headerData + dataIndex, sizeof(numberOfMeshComponents));
    dataIndex += sizeof(numberOfMeshComponents);
    header->m_numberOfMeshComponents = numberOfMeshComponents;
    assert(header->m_meshComponents == nullptr);
    header->m_meshComponents = new int[numberOfMeshComponents];
    for (size_t componentIdx = 0; componentIdx < header->m_numberOfMeshComponents; componentIdx++)
        {
        int component;
        memcpy(&component, headerData + dataIndex, sizeof(component));
        dataIndex += sizeof(component);
        header->m_meshComponents[componentIdx] = component;
        }

    /* Clips */
    uint32_t nbClipSetsIDs;
    memcpy(&nbClipSetsIDs, headerData + dataIndex, sizeof(nbClipSetsIDs));
    dataIndex += sizeof(nbClipSetsIDs);
    header->m_clipSetsID.clear();
    header->m_clipSetsID.reserve(nbClipSetsIDs);
    for (size_t i = 0; i < nbClipSetsIDs; ++i)
        {
        uint32_t clip;
        memcpy(&clip, headerData + dataIndex, sizeof(clip));
        dataIndex += sizeof(clip);
        header->m_clipSetsID.push_back(clip != ISMStore::GetNullNodeID() ? HPMBlockID(clip) : ISMStore::GetNullNodeID());
        }

    /* Children */
    size_t nbChildren;
    memcpy(&nbChildren, headerData + dataIndex, sizeof(nbChildren));
    dataIndex += sizeof(nbChildren);
    header->m_apSubNodeID.clear();
    header->m_apSubNodeID.reserve(nbChildren);
    header->m_numberOfSubNodesOnSplit = nbChildren;
    for (size_t childInd = 0; childInd < nbChildren; childInd++)
        {
        uint32_t childID;
        memcpy(&childID, headerData + dataIndex, sizeof(childID));
        dataIndex += sizeof(childID);
        header->m_apSubNodeID.push_back(childID != ISMStore::GetNullNodeID() ? HPMBlockID(childID) : ISMStore::GetNullNodeID());
        }

    /* Neighbors */
    for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
        {
        size_t numNeighbors;
        memcpy(&numNeighbors, headerData + dataIndex, sizeof(numNeighbors));
        dataIndex += sizeof(numNeighbors);
        header->m_apNeighborNodeID[neighborPosInd].clear();
        header->m_apNeighborNodeID[neighborPosInd].reserve(numNeighbors);
        for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
            {
            uint32_t neighborId;
            memcpy(&neighborId, headerData + dataIndex, sizeof(neighborId));
            dataIndex += sizeof(neighborId);
            header->m_apNeighborNodeID[neighborPosInd].push_back(neighborId != ISMStore::GetNullNodeID() ? HPMBlockID(neighborId) : ISMStore::GetNullNodeID());
            }
        }
    if (dataIndex == maxCountData)
        {
        return;
        }
    uint64_t nbDataSizes;
    memcpy(&nbDataSizes, headerData + dataIndex, sizeof(nbDataSizes));
    dataIndex += sizeof(nbDataSizes);

    header->m_blockSizes.resize(nbDataSizes);
    memcpy(header->m_blockSizes.data(), headerData + dataIndex, nbDataSizes* sizeof(SMIndexNodeHeader<EXTENT>::BlockSize));
    dataIndex += nbDataSizes * sizeof(SMIndexNodeHeader<EXTENT>::BlockSize);

    assert(dataIndex == maxCountData);
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::ReadNodeHeaderFromJSON(SMIndexNodeHeader<EXTENT>* header, const Json::Value& cesiumNodeHeader)
    {
    const auto& nodeHeader = cesiumNodeHeader["SMHeader"];
    if (true/*s_import_from_bim_exported_cesium_3d_tiles*/)
        {
        header->m_id = nodeHeader["id"].asUInt();
        header->m_level = nodeHeader["level"].asUInt();
        //assert(header->m_level == nodeHeader["resolution"].asUInt());

        //header->m_isTextured = nodeHeader["areTextured"].asBool();
        header->m_isTextured = true; // Assume textured, update later if it is not...
        header->m_totalCountDefined = false; // Default not defined

        if (nodeHeader.isMember("totalCount"))
            {
            header->m_totalCount = nodeHeader["totalCount"].asUInt();
            header->m_totalCountDefined = true;
            }

        if (nodeHeader.isMember("nodeCount")) header->m_nodeCount = nodeHeader["nodeCount"].asUInt();
        //header->m_arePoints3d = nodeHeader["arePoints3d"].asBool();
        header->m_arePoints3d = false; // NEEDS_WORK_SM_STREAMING : Always true for Cesium original datasets?
        //assert(header->m_arePoints3d == nodeHeader["arePoints3d"].asBool());

        //header->m_nbFaceIndexes = nodeHeader["nbFaceIndexes"].asUInt();

        //header->m_uvID = nodeHeader["uvID"].asUInt();
        header->m_uvID = header->m_id; // Same as node ID?

        uint64_t parentNodeID = nodeHeader["parentID"].asUInt64();
        header->m_parentNodeID = parentNodeID != ISMStore::GetNullNodeID() ? HPMBlockID(parentNodeID) : ISMStore::GetNullNodeID();

        header->m_geometricResolution = cesiumNodeHeader["geometricError"].asFloat();
        header->m_textureResolution = header->m_geometricResolution;

        if (cesiumNodeHeader.isMember("transform"))
            {
            auto& transform = cesiumNodeHeader["transform"];
            m_transform = Transform::FromRowValues(transform[0].asDouble(), transform[1].asDouble(), transform[2].asDouble(), transform[12].asDouble(),
                                                   transform[4].asDouble(), transform[5].asDouble(), transform[6].asDouble(), transform[13].asDouble(),
                                                   transform[8].asDouble(), transform[9].asDouble(), transform[10].asDouble(), transform[14].asDouble());
            //m_transform = Transform::From(533459, 5212605, 0);
            }

        if (cesiumNodeHeader.isMember("boundingVolume"))
            {
            auto const& bv = cesiumNodeHeader["boundingVolume"];
            if (bv.isMember("box"))
                {
                auto const& boundingBox = bv["box"];
                DPoint3d center = DPoint3d::From(boundingBox[0].asDouble(), boundingBox[1].asDouble(), boundingBox[2].asDouble());
                DPoint3d direction = DPoint3d::From(boundingBox[3].asDouble(), boundingBox[7].asDouble(), boundingBox[11].asDouble()); // assumes boxes are aligned with axes
                ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, center.x - direction.x);
                ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, center.y - direction.y);
                ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, center.z - direction.z);
                ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, center.x + direction.x);
                ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, center.y + direction.y);
                ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, center.z + direction.z);
                }
            else
                {
                assert(bv.isMember("sphere")); // must be sphere if not a box
                auto const& boundingSphere = bv["sphere"];
                DPoint3d center = DPoint3d::From(boundingSphere[0].asDouble(), boundingSphere[1].asDouble(), boundingSphere[2].asDouble());
                double radius = boundingSphere[3].asDouble();
                ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, center.x - radius);
                ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, center.y - radius);
                ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, center.z - radius);
                ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, center.x + radius);
                ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, center.y + radius);
                ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, center.z + radius);
                }

            m_transform.Multiply(header->m_nodeExtent, header->m_nodeExtent);
            header->m_contentExtentDefined = cesiumNodeHeader.isMember("content") && cesiumNodeHeader["content"].isMember("boundingVolume");
            if (header->m_contentExtentDefined)
                {
                auto const& contentBV = cesiumNodeHeader["content"]["boundingVolume"];
                if (contentBV.isMember("box"))
                    {
                    auto& boundingBox = contentBV["box"];
                    DPoint3d center = DPoint3d::From(boundingBox[0].asDouble(), boundingBox[1].asDouble(), boundingBox[2].asDouble());
                    DPoint3d direction = DPoint3d::From(boundingBox[3].asDouble(), boundingBox[7].asDouble(), boundingBox[11].asDouble()); // assumes boxes are aligned with axes
                    ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, center.x - direction.x);
                    ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, center.y - direction.y);
                    ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, center.z - direction.z);
                    ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, center.x + direction.x);
                    ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, center.y + direction.y);
                    ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, center.z + direction.z);
                    }
                else
                    {
                    assert(contentBV.isMember("sphere")); // must be sphere if not a box
                    auto const& boundingSphere = contentBV["sphere"];
                    DPoint3d center = DPoint3d::From(boundingSphere[0].asDouble(), boundingSphere[1].asDouble(), boundingSphere[2].asDouble());
                    double radius = boundingSphere[3].asDouble();
                    ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, center.x - radius);
                    ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, center.y - radius);
                    ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, center.z - radius);
                    ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, center.x + radius);
                    ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, center.y + radius);
                    ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, center.z + radius);
                    }
                m_transform.Multiply(header->m_contentExtent, header->m_contentExtent);
                }
            else
                {
                header->m_contentExtent = header->m_nodeExtent;
                }
            }
        uint32_t idx = header->m_id.m_integerID;

        if (header->m_isTextured)
            {
            header->m_textureID = HPMBlockID();
            header->m_ptsIndiceID.resize(2);
            header->m_ptsIndiceID[1] = (int)idx;
            header->m_ptsIndiceID[0] = HPMBlockID();
            header->m_nbTextures = 1;
            header->m_uvsIndicesID.resize(1);
            header->m_uvsIndicesID[0] = idx;
            }

        // NEEDS_WORK_SM_STREAMING : Are clip sets ID required?
        //auto& clipSets = nodeHeader["clipSetsID"];
        //assert(clipSets.isArray());
        //if (clipSets.size() > 0)
        //    {
        //    header->m_clipSetsID.resize(clipSets.size());
        //    for (size_t i = 0; i < header->m_clipSetsID.size(); ++i) header->m_clipSetsID[i] = clipSets[(Json::ArrayIndex)i].asInt();
        //    }

        auto& children = cesiumNodeHeader["children"];
        assert(children.isArray());

        header->m_numberOfSubNodesOnSplit = children.size();
        header->m_IsBranched = children.size() > 1;
        header->m_IsLeaf = children.size() == 0;

        if (children.size() > 0)
            {
            assert((!header->m_IsBranched && children.size() == 1) || header->m_IsBranched);
            assert(!header->m_IsLeaf);
            header->m_apSubNodeID.resize(children.size());
            int childInd = 0;
            for (auto& child : children)
                {
                if (child.isMember("SMHeader"))
                    header->m_apSubNodeID[childInd++] = HPMBlockID(child["SMHeader"]["id"].asUInt());
                else
                    header->m_apSubNodeID[childInd++] = HPMBlockID(child["SMRootID"].asUInt());
                }
            header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
            }
        }
    else {
        header->m_level = nodeHeader["resolution"].asUInt();
        header->m_filtered = nodeHeader["filtered"].asBool();
        header->m_numberOfSubNodesOnSplit = nodeHeader["nbChildren"].asUInt();
        header->m_IsLeaf = nodeHeader["isLeaf"].asBool();
        header->m_isTextured = nodeHeader["areTextured"].asBool();
        header->m_IsBranched = nodeHeader["isBranched"].asBool();
        header->m_SplitTreshold = nodeHeader["splitThreshold"].asUInt();
        header->m_totalCount = nodeHeader["totalCount"].asUInt();
        header->m_nodeCount = nodeHeader["nodeCount"].asUInt();
        header->m_arePoints3d = nodeHeader["arePoints3d"].asBool();
        header->m_nbFaceIndexes = nodeHeader["nbFaceIndexes"].asUInt();
        header->m_graphID = nodeHeader["graphID"].asUInt();
        header->m_uvID = nodeHeader["uvID"].asUInt();

        uint32_t parentNodeID = nodeHeader["parentID"].asUInt();
        header->m_parentNodeID = parentNodeID != ISMStore::GetNullNodeID() ? HPMBlockID(parentNodeID) : ISMStore::GetNullNodeID();

        auto& nodeExtent = nodeHeader["nodeExtent"];
        assert(nodeExtent.isObject());
        ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, nodeExtent["xMin"].asDouble());
        ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, nodeExtent["yMin"].asDouble());
        ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, nodeExtent["zMin"].asDouble());
        ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, nodeExtent["xMax"].asDouble());
        ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, nodeExtent["yMax"].asDouble());
        ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, nodeExtent["zMax"].asDouble());

        header->m_contentExtentDefined = nodeHeader["contentExtentDefined"].asBool();
        if (header->m_contentExtentDefined)
            {
            auto& contentExtent = nodeHeader["contentExtent"];
            assert(contentExtent.isObject());
            ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, contentExtent["xMin"].asDouble());
            ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, contentExtent["yMin"].asDouble());
            ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, contentExtent["zMin"].asDouble());
            ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, contentExtent["xMax"].asDouble());
            ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, contentExtent["yMax"].asDouble());
            ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, contentExtent["zMax"].asDouble());
            }

        auto& indices = nodeHeader["indiceID"];
        assert(indices.isArray() && indices.size() <= 1);

        uint32_t idx = indices.empty() ? SQLiteNodeHeader::NO_NODEID : indices[(Json::ArrayIndex)0].asUInt();

        if (header->m_isTextured)
            {
            header->m_textureID = HPMBlockID();
            header->m_ptsIndiceID.resize(2);
            header->m_ptsIndiceID[1] = (int)idx;
            header->m_ptsIndiceID[0] = HPMBlockID();
            header->m_nbTextures = 1;
            header->m_uvsIndicesID.resize(1);
            header->m_uvsIndicesID[0] = idx;
            }

        header->m_numberOfMeshComponents = (size_t)nodeHeader["numberOfMeshComponents"].asUInt();
        if (header->m_numberOfMeshComponents > 0)
            {
            auto& meshComponents = nodeHeader["meshComponents"];
            assert(meshComponents.isArray());
            header->m_meshComponents = new int[header->m_numberOfMeshComponents];
            for (size_t i = 0; i < (size_t)header->m_numberOfMeshComponents; i++)
                {
                header->m_meshComponents[i] = meshComponents[(Json::ArrayIndex)i].asInt();
                }
            }

        auto& clipSets = nodeHeader["clipSetsID"];
        assert(clipSets.isArray());
        if (clipSets.size() > 0)
            {
            header->m_clipSetsID.resize(clipSets.size());
            for (size_t i = 0; i < header->m_clipSetsID.size(); ++i) header->m_clipSetsID[i] = clipSets[(Json::ArrayIndex)i].asInt();
            }

        auto& children = nodeHeader["children"];
        assert(children.isArray());
        if (!header->m_IsLeaf && children.size() > 0)
            {
            header->m_apSubNodeID.resize(header->m_numberOfSubNodesOnSplit);
            for (auto& child : children)
                {
                auto childInd = child["index"].asUInt();
                auto nodeId = child["id"].asUInt();
                header->m_apSubNodeID[childInd] = HPMBlockID(nodeId);
                }
            header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
            }

        auto& neighbors = nodeHeader["neighbors"];
        assert(neighbors.isArray());
        if (neighbors.size() > 0)
            {
            for (auto& neighbor : neighbors)
                {
                assert(neighbor.isObject());
                auto nodePos = neighbor["nodePos"].asUInt();
                auto nodeId = neighbor["nodeId"].asUInt();
                header->m_apNeighborNodeID[nodePos].push_back(nodeId);
                }
            }
        }
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, size_t& po_pDataSize)
    {
    //NEEDS_WORK_SM_STREAMING : are we loading node headers multiple times?
    std::unique_ptr<DataSource::Buffer[]>          dest;
    DataSource                                *    dataSource;
    DataSource::DataSize                           readSize;

    DataSourceURL    dataSourceURL(m_pathToHeaders);
    std::wstring extension = s_stream_using_cesium_3d_tiles_format ? L".json" : L".bin";
    dataSourceURL.append(L"n_" + std::to_wstring(blockID.m_integerID) + extension);

    DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

    dataSource = this->InitializeDataSource(dest, destSize);
    if (dataSource == nullptr ||
        dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed() ||
        dataSource->read(dest.get(), destSize, readSize, 0).isFailed()  ||
        dataSource->close().isFailed())
        return;

    if (readSize > 0)
        {
        po_pDataSize = readSize;

        uint8_t *buffer = new uint8_t[readSize];
        if (buffer)
            {
            po_pBinaryData.reset(buffer);
            memmove(po_pBinaryData.get(), dest.get(), po_pDataSize);
            }

        assert(buffer);
        assert(readSize > 0);
        }  
    this->GetDataSourceAccount()->destroyDataSource(dataSource);
    }


template <class EXTENT> bool SMStreamingStore<EXTENT>::GetSisterNodeDataStore(ISDiffSetDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile)
    {   
    if (!IsProjectFilesPathSet())
        return false;

    SMSQLiteFilePtr sqliteFilePtr = GetSisterSQLiteFile(SMStoreDataType::DiffSet, createSisterFile);

    if (!sqliteFilePtr.IsValid())
        return false;
    
    dataStore = new SMSQLiteNodeDataStore<DifferenceSet, EXTENT>(SMStoreDataType::DiffSet, nodeHeader, sqliteFilePtr);

    return true;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMMTGGraphDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {        
    assert(!"Should not be called");
    return false;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {
    assert(dataType != SMStoreDataType::Skirt && dataType != SMStoreDataType::ClipDefinition && dataType != SMStoreDataType::CoveragePolygon);
    
    auto nodeGroup = this->GetGroup(nodeHeader->m_id);
    // NEEDS_WORK_SM_STREAMING: validate node group if node headers are grouped
    //assert(nodeGroup.IsValid());
        dataStore = new SMStreamingNodeDataStore<DPoint3d, EXTENT>(m_dataSourceAccount, dataType, nodeHeader, m_settings->IsPublishing(), nodeGroup);
    
    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetSisterNodeDataStore(ISMCoverageNameDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, bool createSisterFile)
    {
    SMSQLiteFilePtr sqlFilePtr;

    sqlFilePtr = GetSisterSQLiteFile(SMStoreDataType::CoverageName, createSisterFile);

    if (!sqlFilePtr.IsValid())
        return false;

    dataStore = new SMSQLiteNodeDataStore<Utf8String, EXTENT>(SMStoreDataType::CoverageName, nodeHeader, sqlFilePtr);

    return true;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetSisterNodeDataStore(ISM3DPtDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType, bool createSisterFile)
    {
    assert(dataType == SMStoreDataType::Skirt || dataType == SMStoreDataType::ClipDefinition || dataType == SMStoreDataType::CoveragePolygon);

    SMSQLiteFilePtr sqlFilePtr = GetSisterSQLiteFile(dataType, createSisterFile);

    if (!sqlFilePtr.IsValid())
        return false;

    dataStore = new SMSQLiteNodeDataStore<DPoint3d, EXTENT>(dataType, nodeHeader, sqlFilePtr);

    return true;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMInt32DataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {                
    assert(dataType == SMStoreDataType::TriPtIndices || dataType == SMStoreDataType::TriUvIndices);
        
    dataStore = new SMStreamingNodeDataStore<int32_t, EXTENT>(m_dataSourceAccount, dataType, nodeHeader);
                    
    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMTextureDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {    
#ifdef WIP_MESH_IMPORT
    // NEEDS_WORK_SM_STREAMING: make this work (should return a node store that manipulates byes)
    //dataStore = new SMSQLiteNodeDataStore<Byte, EXTENT>(dataType, nodeHeader, m_smSQLiteFile);

    assert(false);
#endif

    dataStore = new StreamingNodeTextureStore<Byte, EXTENT>(m_dataSourceAccount, nodeHeader);
    
    return true;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMUVCoordsDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader, SMStoreDataType dataType)
    {
    assert(dataType == SMStoreDataType::UvCoords);
    dataStore = new SMStreamingNodeDataStore<DPoint2d, EXTENT>(m_dataSourceAccount, dataType, nodeHeader);

    return true;    
    }


//Multi-items loading store

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMPointTriPtIndDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {    
    //dataStore = new SMStreamingNodeDataStore<int32_t, EXTENT>(m_dataSourceAccount, SMStoreDataType::TriPtIndices, nodeHeader);
    assert(!"Not supported yet");

    return false;    
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMTileMeshDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {
    dataStore = new SMStreamingNodeDataStore<bvector<Byte>, EXTENT>(this->GetDataSourceAccount(), SMStoreDataType::Cesium3DTiles, nodeHeader, m_settings->IsPublishing());
    return true;
    }

template <class EXTENT> bool SMStreamingStore<EXTENT>::GetNodeDataStore(ISMCesium3DTilesDataStorePtr& dataStore, SMIndexNodeHeader<EXTENT>* nodeHeader)
    {
    assert(m_nodeHeaderCache.count(nodeHeader->m_id.m_integerID) == 1);
    dataStore = new SMStreamingNodeDataStore<Cesium3DTilesBase, EXTENT>(this->GetDataSourceAccount(), SMStoreDataType::Cesium3DTiles, nodeHeader, *m_nodeHeaderCache[nodeHeader->m_id.m_integerID], m_transform, m_settings->IsPublishing());
    return true;
    }

template <class EXTENT> DataSource* SMStreamingStore<EXTENT>::InitializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize) const
    {
    if (this->GetDataSourceAccount() == nullptr)
        return nullptr;
                                                    // Get the thread's DataSource or create a new one
    DataSource *dataSource = m_dataSourceAccount->createDataSource();
    if (dataSource == nullptr)
        return nullptr;
                                                    // Make sure caching is enabled for this DataSource
    dataSource->setCachingEnabled(s_stream_enable_caching);

    dest.reset(new unsigned char[destSize]);
                                                    // Return the DataSource
    return dataSource;
    }

template <class EXTENT> DataSourceAccount* SMStreamingStore<EXTENT>::GetDataSourceAccount(void) const
    {
    return m_dataSourceAccount;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SetDataSourceAccount(DataSourceAccount *dataSourceAccount)
    {
    m_dataSourceAccount = dataSourceAccount;
    }

template <class EXTENT> void SMStreamingStore<EXTENT>::SetDataFormatType(FormatType formatType)
    {
    m_formatType = formatType;
    }



//------------------SMStreamingNodeDataStore--------------------------------------------
template <class DATATYPE, class EXTENT> SMStreamingNodeDataStore<DATATYPE, EXTENT>::SMStreamingNodeDataStore(DataSourceAccount* dataSourceAccount, SMStoreDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, bool isPublishing, SMNodeGroupPtr nodeGroup, bool compress = true)
    : m_dataSourceAccount(dataSourceAccount),
      m_nodeHeader(nodeHeader),
      m_nodeGroup(nodeGroup),
      m_dataType(type)
    {
    switch (m_dataType)
        {
        case SMStoreDataType::Points: 
            m_dataSourceURL = L"points";
            break;
        case SMStoreDataType::TriPtIndices:
            m_dataSourceURL = L"indices";
            break;
        case SMStoreDataType::UvCoords:
            m_dataSourceURL = L"uvs";
            break;
        case SMStoreDataType::TriUvIndices:
            m_dataSourceURL = L"uvindices";
            break;
        case SMStoreDataType::Texture:
            m_dataSourceURL = L"textures";
            break;
        case SMStoreDataType::Cesium3DTiles:
            //m_dataSourceURL = L"data";
            break;
        default:
            assert(!"Unkown data type for streaming");
        }

    m_dataSourceURL.setSeparator(m_dataSourceAccount->getPrefixPath().getSeparator());

    // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
    //if (s_stream_from_disk)
    //    {
    //    // Create base directory structure to store information if not already done
    //    BeFileName path(m_dataSourceAccount->getPrefixPath().c_str());
    //    path.AppendToPath(m_dataSourceURL.c_str());
    //    BeFileNameStatus createStatus = BeFileName::CreateNewDirectory(path);
    //    assert(createStatus == BeFileNameStatus::Success || createStatus == BeFileNameStatus::AlreadyExists);
    //    }
    //else
    //    {
    //    // stream from azure
    //    }
    }
template <class DATATYPE, class EXTENT> SMStreamingNodeDataStore<DATATYPE, EXTENT>::SMStreamingNodeDataStore(DataSourceAccount* dataSourceAccount, SMStoreDataType type, SMIndexNodeHeader<EXTENT>* nodeHeader, const Json::Value& header, Transform& transform, bool isPublishing, bool compress)
    : m_dataSourceAccount(dataSourceAccount),
    m_nodeHeader(nodeHeader),
    m_jsonHeader(&header),
    m_dataType(type),
    m_transform(transform)
    {
    switch (m_dataType)
        {
        case SMStoreDataType::Points:
            m_dataSourceURL = L"points";
            break;
        case SMStoreDataType::TriPtIndices:
            m_dataSourceURL = L"indices";
            break;
        case SMStoreDataType::UvCoords:
            m_dataSourceURL = L"uvs";
            break;
        case SMStoreDataType::TriUvIndices:
            m_dataSourceURL = L"uvindices";
            break;
        case SMStoreDataType::Texture:
            m_dataSourceURL = L"textures";
            break;
        case SMStoreDataType::Cesium3DTiles:
            // NEEDS_WORK_SM_STREAMING : Remove hard coded path for dgndb 3DTile imports
            m_dataSourceURL = s_import_from_bim_exported_cesium_3d_tiles ? L"graz\\Production_Graz_3MX_1e" : L"";
            break;
        default:
            assert(!"Unkown data type for streaming");
        }
    if (isPublishing)
        {
        auto dataPath = m_dataSourceAccount->getPrefixPath() + L"\\" + m_dataSourceURL;
        dataPath = dataPath.substr(8, dataPath.size());
        if (!BeFileName::DoesPathExist(dataPath.c_str())) BeFileName::CreateNewDirectory(dataPath.c_str());
        }
    m_dataSourceURL.setSeparator(m_dataSourceAccount->getPrefixPath().getSeparator());
    }

template <class DATATYPE, class EXTENT> SMStreamingNodeDataStore<DATATYPE, EXTENT>::~SMStreamingNodeDataStore()
    {            
    //for (auto it = m_dataCache.begin(); it != m_dataCache.end(); ++it)
    //    {
    //    delete it->second;
    //    }
    m_dataCache.clear();
    }

template <class DATATYPE, class EXTENT> HPMBlockID SMStreamingNodeDataStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    HPRECONDITION(blockID.IsValid());

    if (NULL != DataTypeArray && countData > 0)
        {
        Byte* dataToWrite = nullptr;
        uint32_t sizeToWrite = 0;
        std::wstring extension;
        bool mustCleanup = false;
        switch (m_dataType)
            {
            case SMStoreDataType::Cesium3DTiles:
                {
                dataToWrite = reinterpret_cast<bvector<Byte>*>(DataTypeArray)->data();
                sizeToWrite = (uint32_t)countData;
                extension = L".b3dm";
                break;
                }
            default:
                {
                extension = L".bin";
                mustCleanup = true;
                HCDPacket uncompressedPacket, compressedPacket;
                size_t bufferSize = countData * sizeof(DATATYPE);
                uncompressedPacket.SetBuffer(DataTypeArray, bufferSize);
                uncompressedPacket.SetDataSize(bufferSize);
                WriteCompressedPacket(uncompressedPacket, compressedPacket);

                sizeToWrite = (uint32_t)compressedPacket.GetDataSize() + sizeof(uint32_t);
                dataToWrite = new Byte[sizeToWrite];
                reinterpret_cast<uint32_t&>(*dataToWrite) = (uint32_t)bufferSize;
                if (m_dataCache.count(blockID.m_integerID) > 0)
                    {
                    // must update data count in the cache
                    auto& block = this->m_dataCache[blockID.m_integerID];
                    block->resize(bufferSize);
                    memcpy(block->data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                    dataToWrite = block->data();
                    }
                memcpy(dataToWrite + sizeof(uint32_t), compressedPacket.GetBufferAddress(), compressedPacket.GetDataSize());

                // Save block size in node header to optimize download speeds using segmentation
                m_nodeHeader->m_blockSizes.push_back(SMIndexNodeHeader<EXTENT>::BlockSize{ compressedPacket.GetDataSize() + sizeof(uint32_t), (short)m_dataType });
                }
            }

        DataSourceStatus writeStatus;

        DataSourceURL url (m_dataSourceURL);
        url.append(L"p_" + std::to_wstring(blockID.m_integerID) + extension);

        DataSource *dataSource = m_dataSourceAccount->createDataSource();
        assert(dataSource != nullptr); // problem creating a new DataSource

        writeStatus = dataSource->open(url, DataSourceMode_Write);
        assert(writeStatus.isOK()); // problem opening a DataSource

        writeStatus = dataSource->write(dataToWrite, sizeToWrite);
        assert(writeStatus.isOK()); // problem writing a DataSource

        writeStatus = dataSource->close();
        assert(writeStatus.isOK()); // problem closing a DataSource

        m_dataSourceAccount->destroyDataSource(dataSource);

        if (mustCleanup)
            {
            delete[] dataToWrite;
            }
        }

    return blockID;   
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    assert(m_dataType != SMStoreDataType::PointAndTriPtIndices && m_dataType != SMStoreDataType::Cesium3DTiles); // consider using another function signature of GetBlockDataCount which takes into account the store data type

    return GetBlockDataCount(blockID, m_dataType);
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
    {
    size_t count = 0;
    switch (dataType)
        {
        case SMStoreDataType::Points : 
            {
            //count = m_nodeHeader->m_nodeCount;
            count = this->GetBlock(blockID).GetNumberOfPoints();
            m_nodeHeader->m_nodeCount = count;
            break;
            }
        case SMStoreDataType::TriPtIndices :
            {
            //count = m_nodeHeader->m_nbFaceIndexes;
            count = this->GetBlock(blockID).GetNumberOfIndices();
            //assert(m_nodeHeader->m_nbFaceIndexes == count);
            m_nodeHeader->m_nbFaceIndexes = count;
            break;
            }
        case SMStoreDataType::UvCoords:
            {
            count = this->GetBlock(blockID).GetNumberOfUvs();
            if (count == 0) this->m_nodeHeader->m_isTextured = false;
            break;
            }
        case SMStoreDataType::Texture:
            {
            count = this->GetBlock(blockID).GetTextureSize();
            if (count == 0) this->m_nodeHeader->m_isTextured = false;
            break;
            }
        default:
            {
            count = this->GetBlock(blockID).size() / sizeof(DATATYPE);
            break;
            }
        }
    return count;
    }

template <class DATATYPE, class EXTENT> void SMStreamingNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    switch (m_dataType)
        {
        case SMStoreDataType::Points : 
            assert((((int64_t)m_nodeHeader->m_nodeCount) + countDelta) >= 0);
            m_nodeHeader->m_nodeCount += countDelta;                
            break;
         case SMStoreDataType::TriPtIndices : 
            assert((((int64_t)m_nodeHeader->m_nbFaceIndexes) + countDelta) >= 0);
            m_nodeHeader->m_nbFaceIndexes += countDelta;                
            break;
        case SMStoreDataType::UvCoords :
        case SMStoreDataType::TriUvIndices :            
            //MST_TS
            break;
        default : 
            assert(!"Unsupported type");
            break;
        }        
    }

template <class DATATYPE, class EXTENT> void SMStreamingNodeDataStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) 
    {
    assert(!"Invalid call");
    }

template <class DATATYPE, class EXTENT> size_t SMStreamingNodeDataStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    auto& block = this->GetBlock(blockID);
    if (m_dataType != SMStoreDataType::Cesium3DTiles)
        {
        assert(block.size() <= maxCountData * sizeof(DATATYPE));
        memmove(DataTypeArray, block.data(), block.size());

        }
    else
        {
        if (m_jsonHeader->isMember("transform"))
            {
            auto& transform = (*m_jsonHeader)["transform"];
            m_transform = Transform::FromRowValues(transform[0].asDouble(), transform[1].asDouble(), transform[2].asDouble(), transform[12].asDouble(),
                                                   transform[4].asDouble(), transform[5].asDouble(), transform[6].asDouble(), transform[13].asDouble(),
                                                   transform[8].asDouble(), transform[9].asDouble(), transform[10].asDouble(), transform[14].asDouble());
            }

        Cesium3DTilesBase* pData = (Cesium3DTilesBase*)(DataTypeArray);
        assert(pData != 0 && pData->m_pointData != 0 && pData->m_indicesData != 0);
        assert(!m_nodeHeader->m_isTextured || (m_nodeHeader->m_isTextured && pData->m_uvData != 0 && pData->m_textureData != 0));
        assert(maxCountData >= block.GetNumberOfPoints() * sizeof(DPoint3d) + block.GetNumberOfUvs() * sizeof(DPoint2d) + block.GetNumberOfIndices() * sizeof(int32_t) + block.GetTextureSize());
        memmove(pData->m_textureData, block.GetTexture(), block.GetTextureSize());
        memmove(pData->m_uvData, block.GetUVs(), block.GetNumberOfUvs() * sizeof(DPoint2d));
        memmove(pData->m_pointData, block.GetPoints(), block.GetNumberOfPoints() * sizeof(DPoint3d));
        memmove(pData->m_indicesData, block.GetIndices(), block.GetNumberOfIndices() * sizeof(int32_t));
        }
    auto blockSize = block.size();
    // Data now resides in the pool, no longer need to keep it in the store
    m_dataCacheMutex.lock();
    block.UnLoad();
    m_dataCache.erase(block.GetID());
    m_dataCacheMutex.unlock();

    return blockSize;
    }

template <class DATATYPE, class EXTENT> bool SMStreamingNodeDataStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return true;
    }

template <class DATATYPE, class EXTENT> StreamingDataBlock& SMStreamingNodeDataStore<DATATYPE, EXTENT>::GetBlock(HPMBlockID blockID) const
    {
    // std::map [] operator is not thread safe while inserting new elements
    m_dataCacheMutex.lock();
    auto& block = m_dataCache[blockID.m_integerID];
    m_dataCacheMutex.unlock();
    if (!block)
        {
        block.reset(new StreamingDataBlock());
        //auto dataURL = this->m_nodeGroup->GetDataURLForNode(blockID);
        assert(!m_jsonHeader->isNull());
        if (m_jsonHeader->isMember("content"))
            {
            assert((*m_jsonHeader)["content"].isMember("url"));
            auto dataURL = WString((*m_jsonHeader)["content"]["url"].asCString(), BentleyCharEncoding::Utf8);
            block->SetURL(DataSourceURL(dataURL.c_str()));
            block->SetDataSourceURL(m_dataSourceURL);
            block->SetDataSourceExtension(s_stream_using_cesium_3d_tiles_format ? L".b3dm" : L".bin");
            block->Load(m_dataSourceAccount, m_dataType, m_nodeHeader->GetBlockSize((short)m_dataType));

            // Apply transform on result
            block->ApplyTransformOnPoints(m_transform);
            }
        }
    //assert(block->GetID() == blockID.m_integerID);

    //assert(block->IsLoaded() && !block->empty());

    return *block;
    }

void StreamingDataBlock::ApplyTransformOnPoints(const Transform& transform)
    {
    for (uint32_t i = 0; i < m_tileData.numPoints; i++)
        {
        transform.Multiply(m_tileData.m_pointData[i], m_tileData.m_pointData[i]);
        }
    }


bool StreamingDataBlock::IsLoading() 
    { 
    return m_pIsLoading; 
    }

bool StreamingDataBlock::IsLoaded() 
    { 
    return m_pIsLoaded; 
    }

void StreamingDataBlock::LockAndWait()
    {
    unique_lock<mutex> lock(m_pDataBlockMutex);
    m_pDataBlockCV.wait(lock, [this]() { return m_pIsLoaded; });
    }

void StreamingDataBlock::SetLoading() 
    { 
    m_pIsLoading = true; 
    }

DataSource* StreamingDataBlock::initializeDataSource(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize)
    {
    if (dataSourceAccount == nullptr)
        return nullptr;
                                                        // Get the thread's DataSource or create a new one
    DataSource *dataSource = dataSourceAccount->createDataSource();
    if (dataSource == nullptr)
        return nullptr;
                                                        // Make sure caching is enabled for this DataSource
    dataSource->setCachingEnabled(s_stream_enable_caching);

    dest.reset(new unsigned char[destSize]);
                                                        // Return the DataSource
    return dataSource;
    }
    
void StreamingDataBlock::Load(DataSourceAccount *dataSourceAccount, SMStoreDataType dataType, uint64_t dataSizeKnown)
    {
    if (IsLoaded()) return;

    if (IsLoading())
        {
        LockAndWait();
        }
    else
        {
        std::unique_ptr<DataSource::Buffer[]> dest;
        auto readSize = this->LoadDataBlock(dataSourceAccount, dest, dataSizeKnown);
        assert(readSize > 0); // something went wrong loading streaming data block
        if (readSize > 0)
            {
            m_pIsLoaded = true;

            if (dataType != SMStoreDataType::Cesium3DTiles)
                {
                uint32_t uncompressedSize = *reinterpret_cast<uint32_t *>(dest.get());
                //std::wcout << "node id:" << m_pID << "  source: " << m_pDataSource << "  size(bytes) : " << dataSize << std::endl;
                uint32_t sizeData = (uint32_t)readSize - sizeof(uint32_t);
                DecompressPoints(dest.get() + sizeof(uint32_t), sizeData, uncompressedSize);
                switch (dataType)
                    {
                    case SMStoreDataType::Points:
                        {
                        m_tileData.numPoints = (uint32_t)this->size() / sizeof(DPoint3d);
                        break;
                        }
                    case SMStoreDataType::TriPtIndices:
                    case SMStoreDataType::TriUvIndices:
                        {
                        m_tileData.numIndices = (uint32_t)this->size() / sizeof(int32_t);
                        break;
                        }
                    case SMStoreDataType::UvCoords:
                        {
                        m_tileData.numUvs = (uint32_t)this->size() / sizeof(DPoint2d);
                        break;
                        }
                    default:
                        {
                        assert(false); // unknown data block type
                        }
                    }
                }
            else
                {
                this->ParseCesium3DTilesData(dest.get(), readSize);
                }
            }
        }
    }
    
void StreamingDataBlock::UnLoad()
    {
    m_pIsLoaded = false;
    m_pIsLoading = false;
    this->clear();
    }
    
void StreamingDataBlock::SetLoaded()
    {
    m_pIsLoaded = true;
    m_pIsLoading = false;
    m_pDataBlockCV.notify_all();
    }

void StreamingDataBlock::SetID(const uint64_t& pi_ID)
    {
    m_pID = pi_ID;
    }

uint64_t StreamingDataBlock::GetID() 
    { 
    return m_pID; 
    }

void StreamingDataBlock::SetURL(const DataSourceURL & url)
    {
    m_url = url;
    }

void StreamingDataBlock::SetDataSourceURL(const DataSourceURL& pi_URL)
    {
    m_pDataSourceURL = pi_URL;
    }

inline void StreamingDataBlock::SetDataSourcePrefix(const std::wstring & prefix)
    {
    m_pPrefix = prefix;
    }

inline void StreamingDataBlock::SetDataSourceExtension(const std::wstring & extension)
    {
    m_extension = extension;
    }

void StreamingDataBlock::DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize)
    {
    HPRECONDITION(pi_CompressedDataSize <= (numeric_limits<uint32_t>::max) ());

    this->resize(pi_UncompressedDataSize);

    // initialize codec
    HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_CompressedDataSize);
    const size_t unCompressedDataSize = pCodec->DecompressSubset(pi_CompressedData,
                                                                 pi_CompressedDataSize,
                                                                 this->data(),
                                                                 pi_UncompressedDataSize);
    assert(unCompressedDataSize != 0 && pi_UncompressedDataSize == unCompressedDataSize);
    }

inline DPoint3d * StreamingDataBlock::GetPoints()
    {
    return reinterpret_cast<DPoint3d *>(this->data() + m_tileData.pointOffset);
    }

inline int32_t * StreamingDataBlock::GetIndices()
    {
    return reinterpret_cast<int32_t *>(this->data() + m_tileData.indiceOffset);
    }

inline DPoint2d * StreamingDataBlock::GetUVs()
    {
    return reinterpret_cast<DPoint2d *>(this->data() + m_tileData.uvOffset);
    }

inline Byte * StreamingDataBlock::GetTexture()
    {
    return reinterpret_cast<Byte *>(this->data() + m_tileData.textureOffset);
    }

inline uint32_t StreamingDataBlock::GetNumberOfPoints()
    {
    return m_tileData.numPoints;
    }

inline uint32_t StreamingDataBlock::GetNumberOfIndices()
    {
    return m_tileData.numIndices;
    }

inline uint32_t StreamingDataBlock::GetNumberOfUvs()
    {
    return m_tileData.numUvs;
    }

inline uint32_t StreamingDataBlock::GetTextureSize()
    {
    return m_tileData.textureSize;
    }

inline DataSource::DataSize StreamingDataBlock::LoadDataBlock(DataSourceAccount *dataSourceAccount, std::unique_ptr<DataSource::Buffer[]>& destination, uint64_t dataSizeKnown)
    {
    DataSource                                *  dataSource;
    DataSource::DataSize                         readSize;

    DataSourceURL    dataSourceURL(m_pDataSourceURL); 
    //dataSourceURL.append(m_pPrefix + std::to_wstring(m_pID) + m_extension);
    dataSourceURL.append(m_url);

    DataSourceBuffer::BufferSize    destSize = 5 * 1024 * 1024;

    dataSource = initializeDataSource(dataSourceAccount, destination, destSize);
    assert(destination != nullptr);
    if (dataSource == nullptr)
        return 0;

    if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
        return 0;

    if (dataSizeKnown == uint64_t(-1)) dataSizeKnown = 0;
    if (dataSource->read(destination.get(), destSize, readSize, dataSizeKnown).isFailed())
        return 0;

    if (dataSource->close().isFailed())
        return 0;

    dataSourceAccount->destroyDataSource(dataSource);

    return readSize;
    }

inline void StreamingDataBlock::ParseCesium3DTilesData(const Byte* cesiumData, const size_t& cesiumDataSize)
    {
    auto batchTable = cesiumData;
    uint32_t version = *(uint32_t*)(batchTable + sizeof(uint32_t));
    assert(version == 1);
    uint32_t byteLength = *(uint32_t*)(batchTable + 2 * sizeof(uint32_t));
    assert(byteLength == cesiumDataSize);

    uint32_t featureTableJSONByteLength = *(uint32_t*)(batchTable + 3 * sizeof(uint32_t));
    uint32_t featureTableBinaryByteLength = *(uint32_t*)(batchTable + 4 * sizeof(uint32_t));

    uint32_t batchTableJSONByteLength = *(uint32_t*)(batchTable + 5 * sizeof(uint32_t));
    uint32_t batchTableBinaryByteLength = *(uint32_t*)(batchTable + 6 * sizeof(uint32_t));

    assert(batchTableJSONByteLength == 0 || batchTableJSONByteLength >= 570425344 || 
           batchTableBinaryByteLength == 0 || batchTableBinaryByteLength >= 570425344); // Support b3dm-legacy-headers


    // From Cesium 3DTiles Web Viewer source Batched3DModel3DTileContent.js :
    // ---------------------------------------------------------------------------------------------------------------------------------------------------
    // Legacy header #1: [batchLength] [batchTableByteLength]
    // Legacy header #2: [batchTableJsonByteLength] [batchTableBinaryByteLength] [batchLength]
    // Current header: [featureTableJsonByteLength] [featureTableBinaryByteLength] [batchTableJsonByteLength] [batchTableBinaryByteLength]
    // If the header is in the first legacy format 'batchTableJsonByteLength' will be the start of the JSON string (a quotation mark) or the glTF magic.
    // Accordingly its first byte will be either 0x22 or 0x67, and so the minimum uint32 expected is 0x22000000 = 570425344 = 570MB. 
    // It is unlikely that the feature table JSON will exceed this length.
    // The check for the second legacy format is similar, except it checks 'batchTableBinaryByteLength' instead
    // ---------------------------------------------------------------------------------------------------------------------------------------------------
    uint32_t batchLength = 0, 
             batchHeaderLength = 28;
    if (batchTableJSONByteLength >= 570425344)
        {
        batchLength = featureTableJSONByteLength;
        batchTableJSONByteLength = featureTableBinaryByteLength;
        batchTableBinaryByteLength = 0;
        featureTableJSONByteLength = 0;
        featureTableBinaryByteLength = 0;
        batchHeaderLength = 20;
        }
    else if (batchTableBinaryByteLength >= 570425344)
        {
        batchLength = batchTableJSONByteLength;
        batchTableJSONByteLength = featureTableJSONByteLength;
        batchTableBinaryByteLength = featureTableBinaryByteLength;
        featureTableJSONByteLength = 0;
        featureTableBinaryByteLength = 0;
        batchHeaderLength = 24;
        }

    uint32_t gltfHeaderLength = 20;
    uint32_t gltfOffset = batchHeaderLength + featureTableJSONByteLength + batchTableJSONByteLength;
    uint32_t gltfJsonStartOffset = gltfOffset + gltfHeaderLength;
    uint32_t gltfJsonHeaderSize = *(uint32_t*)(batchTable + gltfOffset + 3 * sizeof(uint32_t));
    uint32_t gltfBinaryStartOffset = gltfJsonStartOffset + gltfJsonHeaderSize;

    Json::Reader    reader;
    const char* beginGLTFJsonHeader = reinterpret_cast<const char *>(batchTable + gltfJsonStartOffset);
    const char* endGLTFJsonHeader = reinterpret_cast<const char *>(&(beginGLTFJsonHeader[gltfJsonHeaderSize]));
    Json::Value     cesiumBatchTableHeader;
    if (!reader.parse(beginGLTFJsonHeader, endGLTFJsonHeader, cesiumBatchTableHeader))
        {
        assert(!"Could not parse b3dm binary batch table header into json");
        }

    // Cesium accessors to data are taken from the gltf Json header : scene->nodes->meshes->primitives->attributes->{POSITION, TEXCOORD, indices, material}
    auto sceneName = cesiumBatchTableHeader["scene"].asString();
    auto& scenes = cesiumBatchTableHeader["scenes"];
    assert(scenes.size() == 1); // should contain only one scene
    auto& nodes = scenes[sceneName]["nodes"];
    assert(nodes.size() == 1); // should contain only one node
    auto nodeName = nodes[0].asString();
    auto& meshes = cesiumBatchTableHeader["nodes"][nodeName]["meshes"];
    if (meshes.isNull()) return;
    assert(meshes.size() == 1); // should contain only one mesh
    auto meshName = meshes[0].asString();
    auto& primitives = cesiumBatchTableHeader["meshes"][meshName]["primitives"];
    assert(primitives.size() == 1); // should contain only one primitive
    auto indiceAccName = primitives[0]["indices"].asString();
    auto textureAccName = primitives[0]["material"].asString();
    auto& attributes = primitives[0]["attributes"];
    auto pointAccName = attributes["POSITION"].asString();

    auto isTextured = attributes.isMember("TEXCOORD_0");
    auto uvAccName = isTextured ? attributes["TEXCOORD_0"].asString() : "";

    auto& accessors = cesiumBatchTableHeader["accessors"];
    auto& indiceAccessor = accessors[indiceAccName];
    auto& pointAccessor = accessors[pointAccName];
    auto indiceBufferName = indiceAccessor["bufferView"].asString();
    auto pointBufferName = pointAccessor["bufferView"].asString();
    auto extensions = cesiumBatchTableHeader.isMember("extensions") ? cesiumBatchTableHeader["extensions"] : Json::Value();
    auto RTCExtension = extensions.isMember("CESIUM_RTC") ? extensions["CESIUM_RTC"] : Json::Value();
    auto points_are_quantized = pointAccessor.isMember("extensions") && pointAccessor["extensions"].isMember("WEB3D_quantized_attributes");
    auto indiceType = indiceAccessor["componentType"].asUInt();

    struct buffer_object_pointer
        {
        uint32_t count;
        uint32_t byte_size;
        uint32_t offset;
        };
    auto& bufferViews = cesiumBatchTableHeader["bufferViews"];
    auto& indiceBV = bufferViews[indiceBufferName];
    auto& pointBV = bufferViews[pointBufferName];

    buffer_object_pointer indice_buffer_pointer = { indiceAccessor["count"].asUInt(), indiceBV["byteLength"].asUInt(), indiceBV["byteOffset"].asUInt() };
    buffer_object_pointer point_buffer_pointer = { pointAccessor["count"].asUInt(), pointBV["byteLength"].asUInt(), pointBV["byteOffset"].asUInt() };

    m_tileData.numPoints = point_buffer_pointer.count;
    m_tileData.numIndices = indice_buffer_pointer.count;

    auto buffer = batchTable + gltfBinaryStartOffset;
    if (isTextured)
        {
        auto& uvAccessor = accessors[uvAccName];
        auto uvBufferName = uvAccessor["bufferView"].asString();
        auto uvs_are_quantized = uvAccessor.isMember("extensions") && uvAccessor["extensions"].isMember("WEB3D_quantized_attributes");
        auto textureName = cesiumBatchTableHeader["materials"][textureAccName]["values"]["tex"].asString();
        auto imageSourceName = cesiumBatchTableHeader["textures"][textureName]["source"].asString();
        auto imageSourceBufferName = cesiumBatchTableHeader["images"][imageSourceName]["extensions"]["KHR_binary_glTF"]["bufferView"].asString();
        auto imageHeight = cesiumBatchTableHeader["images"][imageSourceName]["extensions"]["KHR_binary_glTF"]["height"].asUInt();
        auto imageWidth = cesiumBatchTableHeader["images"][imageSourceName]["extensions"]["KHR_binary_glTF"]["width"].asUInt();
        auto& uvBV = bufferViews[uvBufferName];
        auto& textureBV = bufferViews[imageSourceBufferName];
        buffer_object_pointer uv_buffer_pointer = { uvAccessor["count"].asUInt(), uvBV["byteLength"].asUInt(), uvBV["byteOffset"].asUInt() };
        buffer_object_pointer texture_buffer_pointer = { 3 * imageHeight * imageWidth, textureBV["byteLength"].asUInt(), textureBV["byteOffset"].asUInt() };
        m_tileData.numUvs = uv_buffer_pointer.count;
        m_tileData.textureSize = texture_buffer_pointer.count + 3 * sizeof(uint32_t);
        auto& uvDecodeMatrixJson = uvAccessor["extensions"]["WEB3D_quantized_attributes"]["decodeMatrix"];

        this->resize(m_tileData.numIndices * sizeof(int32_t) + m_tileData.numPoints * sizeof(DPoint3d) + m_tileData.numUvs * sizeof(DPoint2d) + m_tileData.textureSize);

        if (m_tileData.numUvs > 0)
            {
            m_tileData.uvOffset = 0; // m_tileData.pointOffset + m_tileData.numPoints * sizeof(DPoint3d);
            m_tileData.m_uvData = reinterpret_cast<DPoint2d *>(this->data() + m_tileData.uvOffset);
            if (uvs_are_quantized)
                {
                const FPoint3d scale = { uvDecodeMatrixJson[0].asFloat(), uvDecodeMatrixJson[4].asFloat() };
                const FPoint3d translate = { uvDecodeMatrixJson[6].asFloat(), uvDecodeMatrixJson[7].asFloat() };
                //const DPoint3d scale = { uvDecodeMatrixJson[0].asDouble(), uvDecodeMatrixJson[4].asDouble() };
                //const DPoint3d translate = { uvDecodeMatrixJson[6].asDouble(), uvDecodeMatrixJson[7].asDouble() };

                auto uv_array = (uint16_t*)(buffer + uv_buffer_pointer.offset);
                for (uint32_t i = 0; i < m_tileData.numUvs; i++)
                    {
                    m_tileData.m_uvData[i] = DPoint2d::From(scale.x*(uv_array[2 * i] - 0.5f) + translate.x, 1.0 - (scale.y*(uv_array[2 * i + 1] - 0.5f) + translate.y));
                    }

                }
            else
                {
                auto uv_array = (float*)(buffer + uv_buffer_pointer.offset);
                for (uint32_t i = 0; i < m_tileData.numUvs; i++)
                    {
                    m_tileData.m_uvData[i] = DPoint2d::From(uv_array[2 * i], 1.0 - uv_array[2 * i + 1]);
                    }
                }
            }

        if (m_tileData.textureSize > 3 * sizeof(uint32_t))
            {
            m_tileData.textureOffset = m_tileData.uvOffset + m_tileData.numUvs * sizeof(DPoint2d);
            m_tileData.m_textureData = reinterpret_cast<Byte *>(this->data() + m_tileData.textureOffset);
            memcpy(m_tileData.m_textureData, &imageWidth, sizeof(uint32_t));
            memcpy(m_tileData.m_textureData + sizeof(uint32_t), &imageHeight, sizeof(uint32_t));
            memset(m_tileData.m_textureData + 2 * sizeof(uint32_t), 0, sizeof(uint32_t));

            // Decompress texture
            try {
                auto texture_jpeg = (Byte*)(buffer + texture_buffer_pointer.offset);
                auto codec = new HCDCodecIJG(imageWidth, imageHeight, 3 * 8);// 3 * 8 bits per pixels (rgb)
                codec->SetQuality(70);
                codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
                HFCPtr<HCDCodec> pCodec = codec;
                auto textureSize = (uint32_t)(imageWidth*imageHeight * 3);
                assert(texture_buffer_pointer.count == textureSize);
                const size_t uncompressedDataSize = pCodec->DecompressSubset(texture_jpeg, texture_buffer_pointer.byte_size, m_tileData.m_textureData + 3 * sizeof(uint32_t), textureSize);
                assert(textureSize == uncompressedDataSize);
                }
            catch (const std::exception& e)
                {
                assert(!"There is an error decompressing texture");
                std::wcout << L"Error: " << e.what() << std::endl;
                }
            catch (const HCDException& e)
                {
                //assert(!"There is an error decompressing texture");
                std::wcout << L"Error: " << e.GetExceptionMessage() << std::endl;
                }
            }
        }
    else
        {
        this->resize(m_tileData.numIndices * sizeof(int32_t) + m_tileData.numPoints * sizeof(DPoint3d));
        }

    if (m_tileData.numIndices > 0)
        {
        m_tileData.indiceOffset = m_tileData.textureOffset + m_tileData.textureSize;
        m_tileData.m_indicesData = reinterpret_cast<int32_t *>(this->data() + m_tileData.indiceOffset);
        if (indiceType == 5123 && indice_buffer_pointer.byte_size / indice_buffer_pointer.count == sizeof(uint16_t))
            {
            auto indice_array = (uint16_t*)(buffer + indice_buffer_pointer.offset);
            for (uint32_t i = 0; i < indice_buffer_pointer.count; i++)
                {
                m_tileData.m_indicesData[i] = (uint32_t)indice_array[i] + 1; // QV wants indices to start at 1 for the moment
                assert(m_tileData.m_indicesData[i] > 0);
                }
            }
        else if (indiceType == 5125 && indice_buffer_pointer.byte_size / indice_buffer_pointer.count == sizeof(uint32_t))
            {
            auto indice_array = (uint32_t*)(buffer + indice_buffer_pointer.offset);
            for (uint32_t i = 0; i < indice_buffer_pointer.count; i++)
                {
                m_tileData.m_indicesData[i] = (uint32_t)indice_array[i] + 1; // QV wants indices to start at 1 for the moment
                assert(m_tileData.m_indicesData[i] > 0);
                }
            }
        else
            {
            memcpy(m_tileData.m_indicesData, buffer + indice_buffer_pointer.offset, indice_buffer_pointer.count * sizeof(int32_t));
            }

        if (isTextured) m_tileData.m_uvIndicesData = m_tileData.m_indicesData;
        }
    if (m_tileData.numPoints > 0)
        {
        m_tileData.pointOffset = m_tileData.indiceOffset + indice_buffer_pointer.count * sizeof(int32_t);
        m_tileData.m_pointData = reinterpret_cast<DPoint3d *>(this->data() + m_tileData.pointOffset);
        auto transform = Transform::FromIdentity();
        bool isTransformIdentity = true;
        if (!RTCExtension.isNull() && RTCExtension.isMember("center"))
            {
            auto center = RTCExtension["center"];
            Transform rtcTransform = Transform::From(center[0].asDouble(), center[1].asDouble(), center[2].asDouble());
            transform = Transform::FromProduct(rtcTransform, transform);
            isTransformIdentity = transform.IsIdentity();
            }
        // NEEDS_WORK_SM_STREAMING : Ray's TilePublisher stores vertices using the Z-up convention while CC follows the 3DTiles spec which stores vertices using the Y-up convention
        if (points_are_quantized)
            {
            auto& decodeMatrixJson = pointAccessor["extensions"]["WEB3D_quantized_attributes"]["decodeMatrix"];
            // decode matrix is stored as column major

            //const FPoint3d scale = { decodeMatrixJson[0].asFloat(), decodeMatrixJson[5].asFloat(), decodeMatrixJson[10].asFloat() };
            //const FPoint3d translate = { decodeMatrixJson[12].asFloat(), decodeMatrixJson[13].asFloat(), decodeMatrixJson[14].asFloat() };
            const DPoint3d scale = { decodeMatrixJson[0].asDouble(), decodeMatrixJson[5].asDouble(), decodeMatrixJson[10].asDouble() };
            const DPoint3d translate = { decodeMatrixJson[12].asDouble(), decodeMatrixJson[13].asDouble(), decodeMatrixJson[14].asDouble() };

            auto point_array = (uint16_t*)(buffer + point_buffer_pointer.offset);
            for (uint32_t i = 0; i < m_tileData.numPoints; i++)
                {
                DPoint3d point = DPoint3d::From(scale.x*(point_array[3 * i] - 0.5f) + translate.x, scale.y*(point_array[3 * i + 1] - 0.5f) + translate.y, scale.z*(point_array[3 * i + 2] - 0.5f) + translate.z);
                m_tileData.m_pointData[i] = DPoint3d::From(point.x, -point.z, point.y);
                if (!isTransformIdentity) transform.Multiply(m_tileData.m_pointData[i], m_tileData.m_pointData[i]);
                }

            }
        else
            {
            auto point_array = (float*)(buffer + point_buffer_pointer.offset);
            for (uint32_t i = 0; i < m_tileData.numPoints; i++)
                {
                // The 3DTiles spec suggests using y-up so convert those back to z-up here
                m_tileData.m_pointData[i] = DPoint3d::From(point_array[3 * i], -point_array[3 * i + 2], point_array[3 * i + 1]);
                if (!isTransformIdentity) transform.Multiply(m_tileData.m_pointData[i], m_tileData.m_pointData[i]);
                }
            }
        }
    }

template <class DATATYPE, class EXTENT> StreamingTextureBlock& StreamingNodeTextureStore<DATATYPE, EXTENT>::GetTexture(HPMBlockID blockID) const
    {
    // std::map [] operator is not thread safe while inserting new elements
    m_dataCacheMutex.lock();
    StreamingTextureBlock* texture = static_cast<StreamingTextureBlock*>(m_dataCache[blockID.m_integerID].get());
    if (!texture) texture = new StreamingTextureBlock();
    m_dataCacheMutex.unlock();
    assert((texture->GetID() != uint64_t(-1) ? texture->GetID() == blockID.m_integerID : true));
    if (!texture->IsLoaded())
        {
            auto blockSize = m_nodeHeader->GetBlockSize(5);
            texture->SetID(blockID.m_integerID);
            texture->SetDataSourceURL(m_dataSourceURL);
            texture->Load(this->GetDataSourceAccount(), blockSize);
        }
    assert(texture->IsLoaded() && !texture->empty());
    return *texture;
    }


template <class DATATYPE, class EXTENT> StreamingNodeTextureStore<DATATYPE, EXTENT>::StreamingNodeTextureStore(DataSourceAccount *dataSourceAccount, SMIndexNodeHeader<EXTENT>* nodeHeader)
    : Super(dataSourceAccount, SMStoreDataType::Texture, nodeHeader)
    {
    }

template <class DATATYPE, class EXTENT> bool StreamingNodeTextureStore<DATATYPE, EXTENT>::DestroyBlock(HPMBlockID blockID)
    {
    return false;
    }
                        
template <class DATATYPE, class EXTENT> HPMBlockID StreamingNodeTextureStore<DATATYPE, EXTENT>::StoreBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(blockID.IsValid());

    // The data block starts with 12 bytes of metadata (texture header), followed by pixel data
    StreamingTextureBlock texture(((int*)DataTypeArray)[0], ((int*)DataTypeArray)[1], ((int*)DataTypeArray)[2]);
    texture.SetDataSourceURL(m_dataSourceURL);
    texture.Store(m_dataSourceAccount, DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int), blockID);

    return blockID;
    }

template <class DATATYPE, class EXTENT> HPMBlockID StreamingNodeTextureStore<DATATYPE, EXTENT>::StoreCompressedBlock(DATATYPE* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    assert(blockID.IsValid());

    DataSourceStatus writeStatus;
    DataSourceURL    url(m_dataSourceURL);
    url.append(L"t_" + std::to_wstring(blockID.m_integerID) + L".bin");

    DataSource *dataSource = this->GetDataSourceAccount()->createDataSource();
    assert(dataSource != nullptr);
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
    //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
    //}

    writeStatus = dataSource->open(url, DataSourceMode_Write_Segmented);
    assert(writeStatus.isOK());

    writeStatus = dataSource->write(DataTypeArray, (uint32_t)countData);
    assert(writeStatus.isOK());

    writeStatus = dataSource->close();
    assert(writeStatus.isOK());
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
    //}
    this->GetDataSourceAccount()->destroyDataSource(dataSource);

    return HPMBlockID(blockID.m_integerID);
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const
    {
    return this->GetTexture(blockID).size() + 3 * sizeof(int);
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const
    {
    assert(!"Not supported");
    return 0;
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta) 
    {
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::ModifyBlockDataCount(HPMBlockID blockID, int64_t countDelta, SMStoreDataType dataType) 
    {
    assert(!"Not supported");    
    }

template <class DATATYPE, class EXTENT> size_t StreamingNodeTextureStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    auto& texture = this->GetTexture(blockID);
    auto textureSize = texture.size();
    assert(textureSize + 3 * sizeof(int) == maxCountData);
    ((int*)DataTypeArray)[0] = (int)texture.GetWidth();
    ((int*)DataTypeArray)[1] = (int)texture.GetHeight();
    ((int*)DataTypeArray)[2] = (int)texture.GetNbChannels();
    assert(maxCountData >= texture.size());
    memmove(DataTypeArray + 3 * sizeof(int), texture.data(), std::min(texture.size(), maxCountData));
    m_dataCacheMutex.lock();
    auto textureID = texture.GetID();
    //delete m_dataCache[textureID];
    m_dataCache.erase(textureID);
    m_dataCacheMutex.unlock();
    return std::min(textureSize + 3 * sizeof(int), maxCountData);
    }

template <class DATATYPE, class EXTENT> void StreamingNodeTextureStore<DATATYPE, EXTENT>::SetDataSourceAccount(DataSourceAccount *dataSourceAccount)  
    {
    m_dataSourceAccount = dataSourceAccount;
    }

template <class DATATYPE, class EXTENT> DataSourceAccount* StreamingNodeTextureStore<DATATYPE, EXTENT>::GetDataSourceAccount(void) const                            
    {
    return m_dataSourceAccount;
    }

StreamingTextureBlock::StreamingTextureBlock(void)
    {
    this->SetDataSourcePrefix(L"t_");
    }

inline StreamingTextureBlock::StreamingTextureBlock(const int & width, const int & height, const int & numChannels)
    : m_Width{ width }, m_Height{ height }, m_NbChannels{ numChannels }
    {
    this->SetDataSourcePrefix(L"t_");
    }

inline void StreamingTextureBlock::Store(DataSourceAccount * dataSourceAccount, uint8_t * DataTypeArray, size_t countData, const HPMBlockID & blockID)
    {
    // First, compress the texture
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
    pi_uncompressedPacket.SetDataSize(countData);

    CompressTexture(pi_uncompressedPacket, pi_compressedPacket);

    // Second, save to DataSource
    int format = 0; // Keep an int to define the format and possible other options

    bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
    int *pHeader = (int*)(texData.data());
    pHeader[0] = m_Width;
    pHeader[1] = m_Height;
    pHeader[2] = m_NbChannels;
    pHeader[3] = format;
    memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());

    DataSourceStatus writeStatus;
    DataSourceURL    url(m_pDataSourceURL);
    url.append(L"t_" + std::to_wstring(blockID.m_integerID) + L".bin");

    DataSource *dataSource = dataSourceAccount->createDataSource();
    assert(dataSource != nullptr);
    //{
    //std::lock_guard<mutex> clk(s_consoleMutex);
    //if (!created) std::cout << "[" << std::this_thread::get_id() << "] A datasource is being reused by thread" << std::endl;
    //else std::cout << "[" << std::this_thread::get_id() << "] New thread DataSource created" << std::endl;
    //}

    writeStatus = dataSource->open(url, DataSourceMode_Write_Segmented);
    assert(writeStatus.isOK()); // problem opening a DataSource

    writeStatus = dataSource->write(texData.data(), (uint32_t)(texData.size()));
    assert(writeStatus.isOK()); // problem writing a DataSource

    writeStatus = dataSource->close();
    assert(writeStatus.isOK()); // problem closing a DataSource
                                //{
                                //std::lock_guard<mutex> clk(s_consoleMutex);
                                //std::cout << "[" << std::this_thread::get_id() << "] Thread DataSource finished" << std::endl;
                                //}
    dataSourceAccount->destroyDataSource(dataSource);
    }

inline void StreamingTextureBlock::Load(DataSourceAccount * dataSourceAccount, uint64_t blockSizeKnown)
    {
    std::unique_ptr<DataSource::Buffer[]> dest;
    auto readSize = this->LoadDataBlock(dataSourceAccount, dest, blockSizeKnown);

    if (readSize > 0)
        {
        m_Width = reinterpret_cast<int&>(dest.get()[0]);
        m_Height = reinterpret_cast<int&>(dest.get()[sizeof(int)]);
        m_NbChannels = reinterpret_cast<int&>(dest.get()[2 * sizeof(int)]);
        m_Format = reinterpret_cast<int&>(dest.get()[3 * sizeof(int)]);

        auto textureSize = (uint32_t)(m_Width*m_Height*m_NbChannels);
        uint32_t compressedSize = (uint32_t)readSize - sizeof(4 * sizeof(int));

        DecompressTexture(&(dest.get())[0] + 4 * sizeof(int), compressedSize, textureSize);
        }

    m_pIsLoaded = true;
    }

inline void StreamingTextureBlock::DecompressTexture(uint8_t * pi_CompressedTextureData, uint32_t pi_CompressedTextureSize, uint32_t pi_TextureSize)
    {
    assert(m_Width > 0 && m_Height > 0 && m_NbChannels > 0);

    auto codec = new HCDCodecIJG(m_Width, m_Height, m_NbChannels * 8);// m_NbChannels * 8 bits per pixels
    codec->SetQuality(70);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec = codec;
    try {
        this->resize(pi_TextureSize);
        const size_t uncompressedDataSize = pCodec->DecompressSubset(pi_CompressedTextureData, pi_CompressedTextureSize, this->data(), pi_TextureSize);
        assert(pi_TextureSize == uncompressedDataSize);
        }
    catch (const std::exception& e)
        {
        assert(!"There is an error decompressing texture");
        std::wcout << L"Error: " << e.what() << std::endl;
        }
    }

inline bool StreamingTextureBlock::CompressTexture(const HCDPacket & pi_uncompressedPacket, HCDPacket & pi_compressedPacket)
    {
    HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

    // initialize codec
    auto codec = new HCDCodecIJG(m_Width, m_Height, 8 * m_NbChannels);
    codec->SetQuality(70);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec = codec;
    pi_compressedPacket.SetBufferOwnership(true);
    size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
    pi_compressedPacket.SetBuffer(new uint8_t[compressedBufferSize], compressedBufferSize * sizeof(uint8_t));
    const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(uint8_t), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(uint8_t));
    pi_compressedPacket.SetDataSize(compressedDataSize);

    return true;
    }
