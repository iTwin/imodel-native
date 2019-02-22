//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.cpp $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMNodeGroup.h"
#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <ScalableMesh/IScalableMeshPolicy.h>

//#ifndef NDEBUG
//std::mutex s_consoleMutex;
//#endif


uint32_t s_max_number_nodes_in_group = 100;
size_t s_max_group_size = 256 << 10; // 256 KB
uint32_t s_max_group_depth = 4;
uint32_t s_max_group_common_ancestor = 2;

mutex SMNodeGroup::s_mutex;
SMGroupingStrategy<DRange3d>* s_groupingStrategy = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertToJsonFromBytes(Json::Value & outputJson, std::vector<DataSourceBuffer::BufferData>& dest)
    {
    char* jsonBlob = reinterpret_cast<char *>(dest.data());
    return Json::Reader().parse(jsonBlob, jsonBlob + dest.size(), outputJson);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroupMasterHeader::SaveToFile(const WString pi_pOutputDirPath) const
    {
    assert(!m_oldMasterHeader.empty()); // Old master header must be set!

#if _WIN32                                        // NEEDS_WORK_SM_STREAMING : use new CloudDataSource
    wchar_t buffer[10000];
#else
    char buffer[10000];
    Utf8String outDirPath(pi_pOutputDirPath.c_str());
#endif
    switch (m_parametersPtr->GetStrategyType())
        {
        case SMGroupGlobalParameters::StrategyType::NORMAL:
        {
#if _WIN32
        swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"");
#else
        sprintf(buffer, "%s/MasterHeaderWith%sGroups.bin", outDirPath.c_str(), "");
#endif
        break;
        }
        case SMGroupGlobalParameters::StrategyType::VIRTUAL:
        {
#if _WIN32
        swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"Virtual");
#else
        sprintf(buffer, "%s/MasterHeaderWith%sGroups.bin", outDirPath.c_str(), L"Virtual");
#endif
        break;
        }
        case SMGroupGlobalParameters::StrategyType::CESIUM:
        {
#if _WIN32
        swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"Cesium");
#else
        sprintf(buffer, "%s/MasterHeaderWith%sGroups.bin", outDirPath.c_str(), L"Cesium");
#endif
        break;
        }
        default:
        {
        assert(!"Unknown grouping type");
        return;
        }
        }

    // Put group information in a single binary blob
    bvector<uint8_t> masterBlob(20 * 1000 * 1000);

    // Old Master Header part
    auto const oldMasterHeaderSize = m_oldMasterHeader.size();
    size_t totalSize = 0;

    memcpy(masterBlob.data(), &oldMasterHeaderSize, sizeof(oldMasterHeaderSize));
    totalSize += sizeof(uint32_t);

    memcpy(masterBlob.data() + totalSize, m_oldMasterHeader.data(), oldMasterHeaderSize);
    totalSize += m_oldMasterHeader.size();

    short mode = (short)m_parametersPtr->GetStrategyType();
    memcpy(masterBlob.data() + totalSize, &mode, sizeof(mode));
    totalSize += sizeof(mode);

    // Append group information
    for (auto& group : *this)
        {
        auto const& groupInfo = group.second;
        auto const gid = group.first;
        auto const gNumNodes = groupInfo.size();

        uint64_t group_size = sizeof(gid) + sizeof(groupInfo.m_sizeOfRawHeaders) + sizeof(gNumNodes) + gNumNodes * sizeof(uint64_t);
        if (totalSize + group_size > masterBlob.size())
            {
            // increase the size of the blob
            size_t oldSize = masterBlob.size();
            size_t newSize = oldSize + 1000 * 1000;
            masterBlob.resize(newSize);
            }
        memcpy(masterBlob.data() + totalSize, &gid, sizeof(gid));
        totalSize += sizeof(gid);

        // Group total size of headers
        if (m_parametersPtr->GetStrategyType() == SMGroupGlobalParameters::StrategyType::VIRTUAL)
            {
            memcpy(masterBlob.data() + totalSize, &groupInfo.m_sizeOfRawHeaders, sizeof(groupInfo.m_sizeOfRawHeaders));
            totalSize += sizeof(groupInfo.m_sizeOfRawHeaders);
            }

        // Group number of nodes
        memcpy(masterBlob.data() + totalSize, &gNumNodes, sizeof(gNumNodes));
        totalSize += sizeof(gNumNodes);

        // Group node ids
        memcpy(masterBlob.data() + totalSize, groupInfo.data(), gNumNodes * sizeof(uint64_t));
        totalSize += gNumNodes * sizeof(uint64_t);
        }
    HCDPacket uncompressedPacket, compressedPacket;
    uncompressedPacket.SetBuffer(masterBlob.data(), totalSize);
    uncompressedPacket.SetDataSize(totalSize);
    WriteCompressedPacket(uncompressedPacket, compressedPacket);

#if _WIN32
    std::wstring group_header_filename(buffer);
#else
    std::string group_header_filename(buffer);
#endif
    BeFile file;
    if (OPEN_OR_CREATE_FILE(file, group_header_filename.c_str(), BeFileAccess::Write))
        {
        uint32_t NbChars = 0;
        file.Write(&NbChars, &totalSize, (uint32_t)sizeof(totalSize));
        assert(NbChars == (uint32_t)sizeof(totalSize));

        file.Write(&NbChars, compressedPacket.GetBufferAddress(), (uint32_t)compressedPacket.GetDataSize());
        assert(NbChars == compressedPacket.GetDataSize());
        }
    else
        {
        assert(!"Could not open or create file for writing the group master header");
        }
    file.Close();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::SaveTilesetToCache(Json::Value & tileset, const uint64_t& priorityNodeID, bool generateIDs)
    {
    assert(tileset.isMember("asset") && tileset.isMember("root"));

    if (tileset["asset"].isMember("gltfUpAxis"))
        {
        auto gltfUpAxis = tileset["asset"]["gltfUpAxis"].asString();
        if (gltfUpAxis == "Z") m_upAxis = UpAxis::Z;
        else if (gltfUpAxis == "Y") m_upAxis = UpAxis::Y;
        else if (gltfUpAxis == "X") m_upAxis = UpAxis::X;
        else
            {
            BeAssert(false); // Unknown up axis for gltf
            }
        }
    if (generateIDs)
        {
        auto& currentSMHeader = m_tilesetRootNode["SMHeader"];
        uint64_t parentID = currentSMHeader.isMember("parentID") ? currentSMHeader["parentID"].asUInt() : uint64_t(-1);
        uint64_t rootLevel = currentSMHeader.isMember("level") ? currentSMHeader["level"].asUInt() : 0;

        m_tilesetRootNode = tileset;

        // Generate tileset map in the cache
        return this->SaveTileToCacheWithNewTileIDs(m_tilesetRootNode["root"], priorityNodeID, parentID, true, rootLevel);
        }

    // Replace old root node with new one
    m_tilesetRootNode = tileset;

    return this->SaveTileToCacheWithExistingTileIDs(m_tilesetRootNode["root"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::SaveTileToCacheWithNewTileIDs(Json::Value & tile, uint64_t tileID, uint64_t parentID, bool isRootNode, uint64_t level)
    {
    // Compute next tile ID
    tileID = isRootNode && tileID != uint64_t(-1) && parentID != uint64_t(-1) ? tileID : m_parametersPtr->GetNextNodeID();

    auto& smHeader = tile["SMHeader"];
    smHeader["parentID"] = Json::Value(parentID);
    if (smHeader.isMember("id"))
        {
        smHeader["oldID"] = smHeader["id"];
        }

    smHeader["id"] = Json::Value(tileID);
    assert((smHeader.isMember("resolution") && smHeader["resolution"].asUInt() == level) || !smHeader.isMember("resolution"));
    smHeader["level"] = Json::Value(level);

    if (SUCCESS != this->SaveTileToCache(tile, tileID))
        return ERROR;

    if (tile.isMember("children"))
        {
        for (auto& child : tile["children"])
            {
            // The child tileID parameter will be updated within the next call
            if (SUCCESS != SaveTileToCacheWithNewTileIDs(child, tileID, tileID, false, level + 1))
                return ERROR;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::SaveTileToCacheWithExistingTileIDs(Json::Value & tile)
    {
    // Check availability of tile ID
    if (!tile.isMember("SMRootID") && !(tile.isMember("SMHeader") && tile["SMHeader"].isMember("id")))
        {
        assert(!"Node IDs are not available in the tile");
        return ERROR;
        }

    uint64_t tileID = tile.isMember("SMRootID") ? tile["SMRootID"].asUInt() : tile["SMHeader"]["id"].asUInt();
    uint64_t tileLevel = tile["SMHeader"].isMember("level") ? tile["SMHeader"]["level"].asUInt() : this->m_level;
    tile["SMHeader"]["level"] = Json::Value(tileLevel);

    if (SUCCESS != this->SaveTileToCache(tile, tileID))
        return ERROR;

    if (!tile.isMember("SMRootID") && tile.isMember("children"))
        {
        for (auto& child : tile["children"])
            {
            child["SMHeader"]["level"] = Json::Value(tileLevel + 1);
            if (SUCCESS != SaveTileToCacheWithExistingTileIDs(child))
                return ERROR;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::SaveTileToCache(Json::Value & tile, uint64_t tileID)
    {
    if (tile.isMember("content"))
        {
        auto& content = tile["content"];
        if (content.isMember("url"))
            {
            BeFileName contentURL(content["url"].asString());
            if (L"json" == BEFILENAME(GetExtension, contentURL))
                {
                assert(!tile.isMember("children"));
                // the tile references a new tileset (group)
                static std::atomic<uint32_t> s_currentGroupID = {0};

                auto newPrefix = this->m_dataSourcePrefix;
                newPrefix.append(BEFILENAME(GetDirectoryName, contentURL));
                auto tilesetURL = BEFILENAME(GetFileNameAndExtension, contentURL);
                SMNodeGroupPtr newGroup = SMNodeGroup::Create(this->m_parametersPtr, this->m_groupCachePtr, s_currentGroupID);
                newGroup->SetLevel(tile["SMHeader"]["level"].asUInt());

                newGroup->SetURL(DataSourceURL(tilesetURL.c_str()));

                newGroup->SetDataSourcePrefix(newPrefix);
                //newGroup->SetDataSourceExtension(this->m_dataSourceExtension);
                newGroup->m_tilesetRootNode = tile;

                // we are done processing this part of the tileset, save and exit
                return newGroup->SaveNode(tileID, &newGroup->m_tilesetRootNode);
                }
            else
                {
                assert(L"b3dm" == BEFILENAME(GetExtension, contentURL)); // only b3dm supported at the moment
                auto newURLUtf16 = this->m_dataSourcePrefix;
                newURLUtf16.append(contentURL.c_str());
                auto newURLUtf8 = Utf8String(newURLUtf16.c_str());
                content["url"] = Json::Value(newURLUtf8.c_str());
                }
            }
        }
    return this->SaveNode(tileID, &tile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataSource * SMNodeGroup::InitializeDataSource()
    {
    assert(GetDataSourceAccount() != nullptr);      // The data source account must be set

                                                     // Get the thread's DataSource or create a new one
    DataSource *dataSource;
    if ((dataSource = DataSourceManager::Get()->getOrCreateThreadDataSource(*GetDataSourceAccount(), GetDataSourceSessionName())) == nullptr)
        {
        assert(!"Could not initialize data source");
        return nullptr;
        }

    return dataSource;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::Load()
    {
    unique_lock<mutex> lk(m_groupMutex);
    if (m_isLoading)
        {
        m_groupCV.wait(lk, [this] {return !m_isLoading; });
        }
    else
        {
        m_isLoading = true;
        if (m_parametersPtr->GetStrategyType() == SMGroupGlobalParameters::VIRTUAL)
            {
            this->LoadGroupParallel();
            }
        else
            {
            std::vector<DataSourceBuffer::BufferData> dest;

            m_isLoading = true;

            if (this->DownloadFromID(dest))
                {
                m_isLoading = false;
                m_groupCV.notify_all();
                return ERROR;
                }


            if (!dest.empty())
                {
                uint32_t position = 0;
                uint32_t id;
                memcpy(&id, dest.data(), sizeof(uint32_t));
                assert(m_groupHeader->GetID() == id);
                position += sizeof(uint32_t);

                uint32_t numNodes;
                memcpy(&numNodes, dest.data() + position, sizeof(numNodes));
                assert(m_groupHeader->size() == numNodes);
                position += sizeof(numNodes);

                memcpy(m_groupHeader->data(), dest.data() + position, numNodes * sizeof(SMNodeHeader));
                position += numNodes * sizeof(SMNodeHeader);

                const auto headerSectionSize = dest.size() - position;
                m_rawHeaders.resize(headerSectionSize);
                memcpy(m_rawHeaders.data(), dest.data() + position, headerSectionSize);
                }
            else
                {
                m_isLoading = false;
                m_groupCV.notify_all();
                return ERROR;
                }
            }

        m_isLoading = false;
        m_groupCV.notify_all();
        }

    m_isLoaded = true;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::Load(const uint64_t& priorityNodeID)
    {
    unique_lock<mutex> lk(m_groupMutex, std::defer_lock);
    //auto nodeHeader = this->GetNodeHeader(priorityNodeID);
    //assert(nodeHeader != nullptr);
    if (!lk.try_lock() || m_isLoading)
        {
        //// Someone else is loading the group
        //if (nodeHeader->offset == (uint32_t)-1)
        //    {
        //    // Data not ready yet, wait until it becomes ready
        //    if (lk.owns_lock()) lk.unlock();
        //    this->WaitFor(*nodeHeader);
        //    }
        if (!m_isLoaded)
            {
            return ERROR;
            }
        }
    else
        {
        if (m_parametersPtr->GetStrategyType() == SMGroupGlobalParameters::CESIUM)
            {
            assert(m_isLoading == false);
            m_isLoading = true;

            DataSourceURL url(this->m_dataSourcePrefix.c_str());
            url.append(this->GetURL());

            Json::Value tileset;
            if (!this->DownloadCesiumTileset(url, tileset))
                {
                if (this->m_isRoot)
                    {
                    // This is a serious error, the root tileset could not be downloaded...
                    // Either the dataset does not exist or a connection problem occured.
                    // Since this is not immediately recoverable, we throw here.
                    throw runtime_error("Error loading root Cesium tileset!");
                    }
                // The connection may be temporarily lost and the entire tileset will be missing 
                // but can be recoverable when the connection comes back live.
                m_isLoading = false;
                return ERROR;
                }

            if (!tileset.isMember("root"))
                {
                assert(!"error reading Cesium 3D tileset");
                return ERROR;
                }

            bool mustGenerateIDs = !(tileset["root"].isMember("SMHeader") && tileset["root"]["SMHeader"].isMember("id"));
            if (SUCCESS != this->SaveTilesetToCache(tileset, priorityNodeID, mustGenerateIDs))
                {
                m_isLoading = false;
                return ERROR;
                }
            }
        else
            {
            // No longer supported
            return ERROR;
            //assert(m_isLoading == false);
            //m_isLoading = true;
            //if (m_strategyType == VIRTUAL)
            //    {
            //    this->LoadGroupParallel();
            //
            //    if (lk.owns_lock()) lk.unlock();
            //
            //    this->WaitFor(*nodeHeader);
            //    }
            //else
            //    {
            //    std::unique_ptr<DataSource::Buffer[]>       dest;
            //    DataSource                              *   dataSource;
            //    DataSource::DataSize                        readSize;
            //
            //    DataSourceBuffer::BufferSize                destSize = 5 * 1024 * 1024;
            //
            //    dataSource = this->InitializeDataSource(dest, destSize);
            //    if (dataSource == nullptr)
            //        {
            //        m_isLoading = false;
            //        m_groupCV.notify_all();
            //        return ERROR;
            //        }
            //
            //    this->LoadFromDataSource(dataSource, dest.get(), destSize, readSize);
            //
            //    if (readSize > 0)
            //        {
            //        size_t position = 0;
            //        uint32_t id;
            //        memcpy(&id, dest.get(), sizeof(id));
            //        assert(m_groupHeader->GetID() == id);
            //        position += sizeof(id);
            //
            //        size_t numNodes;
            //        memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
            //        assert(m_groupHeader->size() == numNodes);
            //        position += sizeof(numNodes);
            //
            //        memcpy(m_groupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
            //        position += (uint32_t)numNodes * sizeof(SMNodeHeader);
            //
            //        const auto headerSectionSize = readSize - position;
            //        m_rawHeaders.resize(headerSectionSize);
            //        memcpy(m_rawHeaders.data(), dest.get() + position, headerSectionSize);
            //        }
            //    else
            //        {
            //        m_isLoading = false;
            //        m_groupCV.notify_all();
            //        return ERROR;
            //        }
            //    }
            //assert(nodeHeader->offset != (uint32_t)-1);
            }
        }

    // Loading completed successfully
    m_isLoading = false;
    m_isLoaded = true;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::LoadGroupParallel()
    {
    SMNodeGroupPtr group(this);
    std::thread thread([group]()
        {
#ifdef DEBUG_GROUPS
        static uint64_t s_numProcessedNodeId = 0;
        {
        //std::lock_guard<mutex> lk(s_consoleMutex);
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] Distributing... " << std::endl;
        }
#endif
        assert(group->m_nodeDistributorPtr != nullptr);
        for (auto nodeHeader : *group->m_groupHeader)
            {
            group->m_nodeDistributorPtr->AddWorkItem(DistributeData(nodeHeader.blockid, group.get()));
            }
#ifdef DEBUG_GROUPS
        {
        std::lock_guard<mutex> lk(s_consoleMutex);
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] waiting for nodes to process... " << std::endl;
        }
#endif
        group->m_nodeDistributorPtr->Wait([group]()
            {
            return group->m_progress == group->m_groupHeader->size();
            });
#ifdef DEBUG_GROUPS
        {
       // std::lock_guard<mutex> lk(s_consoleMutex);
        s_numProcessedNodeId += group->m_groupHeader->size();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] " << group->m_groupHeader->size() << " nodes (total " << s_numProcessedNodeId << ")" << std::endl;
        }
#endif
        group->m_isLoading = false;
        group->m_isLoaded = true;
        group->m_groupCV.notify_all();
        });
    thread.detach();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceURL SMNodeGroup::GetDataURLForNode(HPMBlockID blockID)
    {
    const auto& jsonNode = *this->m_tileTreeMap[blockID.m_integerID];

    BeFileName dataURL(jsonNode["content"]["url"].asString());
    assert(BEFILENAME(GetExtension, dataURL) == L"b3dm");
    return DataSourceURL(dataURL.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceURL SMNodeGroup::GetURL()
    {
    return m_url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::SetURL(DataSourceURL url)
    {
    m_url = url;
    }


Json::Value* SMNodeGroup::GetSMMasterHeaderInfo()
    {
    if (m_tilesetRootNode.isMember("root") && m_tilesetRootNode["root"].isMember("SMMasterHeader"))
        return &m_tilesetRootNode["root"]["SMMasterHeader"];
    return nullptr;
    }

uint64_t SMNodeGroup::GetRootTileID()
    {
    assert(m_tilesetRootNode.isMember("root"));
    return m_tilesetRootNode["root"]["SMHeader"]["id"].asUInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value* SMNodeGroup::DownloadNodeHeader(const uint64_t & id)
    {
    uint64_t idToLoad = id;
    // *this* is the root tileset, try to load it if needed
    if (!this->IsLoaded())
        {
        if (SUCCESS != this->Load(id)) return nullptr;
        if (m_isRoot) idToLoad = GetRootTileID();
        }

    // Find the actual group that contains the node *id* (may not be the root tileset) and then load it if needed
    auto group = m_groupCachePtr->GetGroupForNodeIDFromCache(idToLoad);
    if (group == nullptr) return nullptr;
    if (!group->IsLoaded())
        {
        // Remove node from cache so that it gets replaced with appropriate node when loading the group
        m_groupCachePtr->RemoveNodeFromCache(idToLoad);
        if (SUCCESS != group->Load(idToLoad)) return nullptr;
        }
    return m_groupCachePtr->GetNodeFromCache(idToLoad);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::SetHeaderDataAtCurrentPosition(const uint64_t& nodeID, const uint8_t* rawHeader, const uint64_t& headerSize)
    {
    std::lock_guard<std::mutex> lock(m_groupMutex);
    auto nodeHeader = this->GetNodeHeader(nodeID);
    nodeHeader->size = headerSize;
    nodeHeader->offset = m_currentPosition;
    memmove(m_rawHeaders.data() + m_currentPosition, rawHeader, nodeHeader->size);
    m_currentPosition += nodeHeader->size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceAccount * SMNodeGroup::GetDataSourceAccount(void)
    {
    return m_parametersPtr->GetDataSourceAccount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lee.Bull         02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
const DataSource::SessionName &SMNodeGroup::GetDataSourceSessionName(void)
    {
    return m_parametersPtr->GetDataSourceSessionName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SMNodeGroup::DownloadCesiumTileset(const DataSourceURL & url, Json::Value & tileset)
    {
    std::vector<DataSourceBuffer::BufferData>         dest;
    if (this->DownloadBlob(dest, url))
        {
        return ConvertToJsonFromBytes(tileset, dest);
        }

    assert(!"A problem occured while downloading a group.");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SMNodeGroup::DownloadBlob(std::vector<DataSourceBuffer::BufferData>& dest, const DataSourceURL & url)
    {
    DataSourceStatus                status;
    bool                            isFromCache = false;

    DataSource *dataSource = nullptr;
    if (nullptr ==  (dataSource = this->InitializeDataSource()))
        return false;

    try
        {
        if ((status = dataSource->open(url, DataSourceMode_Read)).isFailed())
            throw status;

        if ((status = dataSource->read(dest)).isFailed())
            throw status;

        isFromCache = dataSource->isFromCache();

        if ((status = dataSource->close()).isFailed())
            throw status;
        }
    catch (DataSourceStatus)
        {
        assert(false);
        }

    DataSourceManager::Get()->destroyDataSource(dataSource);

    if (status.isFailed())
        return false;

    if (m_isRoot && isFromCache)
        { // Must download so that we can check timestamp and clear cache if necessary
        Json::Value cachedTileset;
        if (ConvertToJsonFromBytes(cachedTileset, dest) &&
            !cachedTileset.isNull() &&
            cachedTileset["root"].isMember("SMMasterHeader") &&
            cachedTileset["root"]["SMMasterHeader"].isMember("LastModifiedDateTime"))
            {
            std::vector<DataSourceBuffer::BufferData> newDest;

            if ((dataSource = this->InitializeDataSource()) != nullptr)
                {
                dataSource->setCachingEnabled(false);

                try
                    {
                    if ((status = dataSource->open(url, DataSourceMode_Read)).isFailed())
                        throw status;

                    if((status = dataSource->read(newDest)).isFailed())
                        throw status;
                    }
                catch (DataSourceStatus)
                    {
                    DataSourceManager::Get()->destroyDataSource(dataSource);
                    assert(false);
                    return false;
                    }

                    // Compare timestamps
                Json::Value networkTileset;

                if (ConvertToJsonFromBytes(networkTileset, newDest) &&
                    !networkTileset.isNull() &&
                    networkTileset["root"].isMember("SMMasterHeader") &&
                    networkTileset["root"]["SMMasterHeader"].isMember("LastModifiedDateTime"))
                    {
                    DateTime cachedTimestamp;
                    DateTime networkTimestamp;

                    if (BentleyStatus::SUCCESS == DateTime::FromString(cachedTimestamp, cachedTileset["root"]["SMMasterHeader"]["LastModifiedDateTime"].asCString()) &&
                        BentleyStatus::SUCCESS == DateTime::FromString(networkTimestamp, networkTileset["root"]["SMMasterHeader"]["LastModifiedDateTime"].asCString()) &&
                        DateTime::CompareResult::EarlierThan == DateTime::Compare(cachedTimestamp, networkTimestamp))
                        {
                        // Construct path to tempory folder
                        BeFileName tempPath;
#if defined(VANCOUVER_API) || defined(DGNDB06_API)
                        if (BeFileNameStatus::Success != BeFileName::BeGetTempPath(tempPath))                        
#else
                        if (BeFileNameStatus::Success != Desktop::FileSystem::BeGetTempPath(tempPath))
#endif
                            BeAssert(false); // Couldn't retrieve the temporary path
                        tempPath.AppendToPath(L"RealityDataCache");
                        auto accountName = m_parametersPtr->GetDataSourceAccount()->getAccountName();
                        tempPath.AppendToPath(accountName.c_str());
                        DataSourceURL cachedDataSourcePath;
                        dataSource->getURL(cachedDataSourcePath);
                        tempPath.AppendToPath(cachedDataSourcePath.c_str());

                        // Clear cache
                        if (BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(BeFileName::GetDirectoryName(tempPath.c_str()).c_str()))
                            {
                            BeAssert(false); // Couldn't remove temp directory
                            }

                        // Update data
                        dest = std::move(newDest);

                        // Recreate cache with updated data
                        dataSource->setForceWriteToCache();
                        }
                    }
                }

            dataSource->setCachingEnabled(isFromCache);

            // Closing the DataSource will write to cache if need be
            dataSource->close();

            DataSourceManager::Get()->destroyDataSource(dataSource);
            }
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t SMNodeGroup::GetSingleNodeFromStore(const uint64_t & pi_pNodeID, bvector<uint8_t>& pi_pData)
    {

    std::vector<DataSourceBuffer::BufferData> dest;

    DataSourceURL dataSourceURL(m_dataSourcePrefix.c_str());
    dataSourceURL += std::to_wstring(pi_pNodeID) + m_dataSourceExtension.c_str();

    if (this->DownloadBlob(dest, dataSourceURL))
        {
        pi_pData.resize(dest.size());
        memmove(pi_pData.data(), reinterpret_cast<char *>(dest.data()), dest.size());
        }

    assert(!pi_pData.empty()); // A problem occured while downloading a blob
    return pi_pData.size();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMNodeGroup::SaveNode(const uint64_t & id, Json::Value * header)
    {
    if (m_groupCachePtr == nullptr)
        {
        assert(!"Cannot save node header in invalid group cache");
        return ERROR;
        }
    return m_groupCachePtr->AddNodeToGroupCache(this, id, header);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::WaitFor(SMNodeHeader& pi_pNode)
    {
    std::unique_lock<mutex> lk(m_groupMutex);
    {
#ifdef DEBUG_GROUPS
    //std::lock(m_groupMutex, s_consoleMutex);
    {
    std::lock_guard<mutex> consoleLock(s_consoleMutex);
    std::cout << "[" << std::this_thread::get_id() << "," << this->m_groupHeader->GetID() << "," << pi_pNode.blockid << "] waiting..." << std::endl;
    }
#endif
    assert(lk.owns_lock());
    m_groupCV.wait(lk, [this, &pi_pNode]
        {
        if (!m_isLoading && m_isLoaded)
            {
            assert(pi_pNode.offset != (uint32_t)-1);
            return true;
            }
        return pi_pNode.offset != (uint32_t)-1;
        });
#ifdef DEBUG_GROUPS
        {
        std::lock_guard<mutex> consoleLock(s_consoleMutex);
        std::cout << "[" << std::this_thread::get_id() << "," << this->m_groupHeader->GetID() << "," << pi_pNode.blockid << "] ready!" << std::endl;
        }
#endif
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::Append3DTile(const uint64_t& nodeID, const uint64_t& parentNodeID, const Json::Value& tile)
    {
    if (!m_tileTreeMap.empty())
        {
        assert(parentNodeID != uint32_t(-1));
        assert(m_tileTreeMap.count(parentNodeID) == 1);
        auto parentTilePtr = m_tileTreeMap[parentNodeID];
        assert(parentTilePtr != nullptr);

        Json::Value& children = (*parentTilePtr)["children"];
        m_tileTreeMap[nodeID] = &children.append(tile);
        }
    else
        { 
        // We are adding the first tile in a new group
        // So keep the tile for later reference
        m_tilesetRootNode = tile;
        m_tileTreeMap[nodeID] = &m_tilesetRootNode;

        if (m_ParentGroup.IsValid())
            {
            assert(m_ParentGroup->m_tileTreeMap.count(parentNodeID) == 1);

            // We must update the parent's content url so we know how to reference the new tile set
            auto& parentNodeTile = *m_ParentGroup->m_tileTreeMap[parentNodeID];
            auto& parentNodeTileChildren = parentNodeTile["children"];

            // Keep the index before updating the parent tile
            m_tilesetRootNode["index"] = parentNodeTileChildren.size();
            m_tileTreeMap[parentNodeID] = &parentNodeTile;

            auto& childTile = parentNodeTileChildren.append(tile);
            childTile.removeMember("children");

            childTile["content"]["url"] = Utf8String(("n_" + std::to_string(this->GetID()) + ".json").c_str());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::AppendChildGroup(SMNodeGroupPtr childGroup)
    {
    Json::Value childReferenceNode;
    childReferenceNode["boundingVolume"] = childGroup->m_tilesetRootNode["boundingVolume"];
    childReferenceNode["geometricError"] = childGroup->m_tilesetRootNode["geometricError"];
    childReferenceNode["refine"] = childGroup->m_tilesetRootNode["refine"];
    auto& childReferenceContent = childReferenceNode["content"];
    childReferenceContent["boundingVolume"] = childGroup->m_tilesetRootNode["content"]["boundingVolume"];

    BeFileName tilesetDir(childGroup->m_outputDirPath.c_str());
#ifdef VANCOUVER_API
    tilesetDir.StripSeparatorAtEnd();
#else
    BeFileName::FixPathName(tilesetDir, tilesetDir.c_str(), false);
#endif
    WString tilesetUrl = BeFileName::GetFileNameWithoutExtension(tilesetDir.c_str()) + L"/n_0.json";
    childReferenceContent["url"] = Utf8String(tilesetUrl.c_str());
    m_tilesetRootNode["children"].append(childReferenceNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMNodeGroup::MergeChild(SMNodeGroupPtr child)
    {
    assert(child->m_ParentGroup == this);
    if (!child->m_tilesetRootNode) return;
    assert(child->m_tilesetRootNode.isMember("SMHeader") && child->m_tilesetRootNode["SMHeader"].isMember("parentID"));

    auto& childTileTreeNode = child->m_tilesetRootNode;
    auto childIndex = childTileTreeNode["index"].asUInt();
    childTileTreeNode.removeMember("index");

    auto parentID = childTileTreeNode["SMHeader"]["parentID"].asUInt();
    auto& parentTileChildren = (*this->m_tileTreeMap[parentID])["children"];

    // Replace tile in the parent tile
    parentTileChildren[childIndex] = childTileTreeNode;
    auto childID = childTileTreeNode["SMHeader"]["id"].asUInt();
    child->m_tileTreeMap[childID] = &parentTileChildren[childIndex];

    // No longer need child tile tree node
    childTileTreeNode = Json::Value();

    this->m_tileTreeMap.insert(child->m_tileTreeMap.begin(), child->m_tileTreeMap.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SMGroupCache::AddNodeToGroupCache(SMNodeGroupPtr group, const uint64_t & id, Json::Value * header)
    {
    std::lock_guard<std::mutex> lock(m_cacheMutex);

    // Check cache state
    if (m_nodeHeadersPtr == nullptr || m_downloadedGroupsPtr == nullptr || m_downloadedGroupsPtr->count(id) > 0)
        {
        assert(!"Cannot add group to cache because cache is not setup properly or adding will corrupt existing data");
        return ERROR;
        }

    // Add to cache
    m_nodeHeadersPtr->operator[](id) = header;
    m_downloadedGroupsPtr->operator[](id) = group;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SMGroupCache::SMGroupCache(node_header_cache* nodeCache)
    : m_nodeHeadersPtr(nodeCache),
      m_downloadedGroupsPtr(std::make_shared<group_cache>())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SMNodeGroupPtr SMGroupCache::GetGroupForNodeIDFromCache(const uint64_t& nodeId)
    {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    if (m_downloadedGroupsPtr->count(nodeId) == 0)
        return nullptr;
    return m_downloadedGroupsPtr->operator[](nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SMGroupCache::RemoveNodeFromCache(const uint64_t & nodeId)
    {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_nodeHeadersPtr->erase(nodeId);
    m_downloadedGroupsPtr->erase(nodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value * SMGroupCache::GetNodeFromCache(const uint64_t & nodeId)
    {
    assert(m_nodeHeadersPtr->count(nodeId) == 1); // Node was not properly downloaded
    return m_nodeHeadersPtr->operator[](nodeId);
    }


#if !defined(VANCOUVER_API) && !defined(DGNDB06_API)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre 08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SMGroupCache::_GetExcessiveRefCountThreshold() const 
    { 
    return numeric_limits<uint32_t>::max(); 
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SMGroupCache::Ptr SMGroupCache::Create(node_header_cache* nodeCache)
    {
    return new SMGroupCache(nodeCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SMGroupGlobalParameters::SMGroupGlobalParameters(StrategyType strategy, DataSourceAccount* account, const DataSource::SessionName &session)
    : m_strategyType(strategy),
      m_dataSourceAccount(account),
      m_dataSourceSessionName(session)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DataSourceAccount * SMGroupGlobalParameters::GetDataSourceAccount()
    {
    return m_dataSourceAccount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lee.Bull        02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
const DataSource::SessionName &SMGroupGlobalParameters::GetDataSourceSessionName()
    {
    return m_dataSourceSessionName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SMGroupGlobalParameters::Ptr SMGroupGlobalParameters::Create(StrategyType strategy, DataSourceAccount * account, const DataSource::SessionName &session)
    {
    return new SMGroupGlobalParameters(strategy, account, session);
    }


