//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMNodeGroup.h"

//#ifndef NDEBUG
std::mutex s_consoleMutex;
//#endif


uint32_t s_max_number_nodes_in_group = 100;
size_t s_max_group_size = 256 << 10; // 256 KB
uint32_t s_max_group_depth = 4;
uint32_t s_max_group_common_ancestor = 2;

unordered_map<uint64_t, SMNodeGroup::Ptr> SMNodeGroup::s_downloadedGroups;
mutex SMNodeGroup::s_mutex;

StatusInt SMNodeGroup::Load(const uint64_t& priorityNodeID, bool mustGenerateIDs)
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
        }
    else
        {
        if (m_strategyType == CESIUM)
            {
            assert(m_isLoading == false);
            m_isLoading = true;

            std::unique_ptr<DataSource::Buffer[]>       dest;
            DataSource::DataSize                        readSize;

            DataSourceURL url = this->m_dataSourcePrefix.c_str();
            url.append(this->GetURL());
            if (!this->DownloadCesiumTileset(dest, readSize, url))
                {
                m_isLoading = false;
                return ERROR;
                }

            if (readSize > 0)
            {
                if (mustGenerateIDs)
                {
                    uint32_t currentNodeID = m_RootTileTreeNode["SMHeader"].isMember("id") ? m_RootTileTreeNode["SMHeader"]["id"].asUInt() : uint32_t(-1);
                    uint32_t parentID = m_RootTileTreeNode["SMHeader"].isMember("parentID") ? m_RootTileTreeNode["SMHeader"]["parentID"].asUInt() : uint32_t(-1);
                    uint32_t rootLevel = m_RootTileTreeNode["SMHeader"].isMember("level") ? m_RootTileTreeNode["SMHeader"]["level"].asUInt() : 0;
                    Json::Reader    reader;
                    char* jsonBlob = reinterpret_cast<char *>(dest.get());
                    reader.parse(jsonBlob, jsonBlob + readSize, m_RootTileTreeNode);

                    if (!(m_RootTileTreeNode.isMember("root") /*&& m_RootTileTreeNode["root"].isMember("SMHeader")*/))
                    {
                        assert(!"error reading Cesium 3D tiles header");
                        return 0;
                    }

                    // Generate tile tree map
                    std::function<void(Json::Value&, uint32_t, bool, uint32_t)> tileTreeMapGenerator = [this, &tileTreeMapGenerator, &currentNodeID](Json::Value& jsonNode, uint32_t parentID, bool isRootNode, uint32_t level)
                    {
                        jsonNode["SMHeader"]["parentID"] = parentID;
                        static std::atomic<uint32_t> s_currentNodeID = 0;
                        uint32_t nodeID = isRootNode && parentID != uint32_t(-1) && currentNodeID != uint32_t(-1) ? currentNodeID : s_currentNodeID++;
                        if (jsonNode["SMHeader"].isMember("id"))
                        {
                            jsonNode["SMHeader"]["oldID"] = jsonNode["SMHeader"]["id"];
                        }

                        jsonNode["SMHeader"]["id"] = nodeID;
                        assert((jsonNode["SMHeader"].isMember("resolution") && jsonNode["SMHeader"]["resolution"].asUInt() == level) || !jsonNode["SMHeader"].isMember("resolution"));
                        jsonNode["SMHeader"]["level"] = level;
                        if (!jsonNode.isMember("children"))
                        {
                            if (jsonNode.isMember("content") && jsonNode["content"].isMember("url"))
                            {
                                BeFileName contentURL(jsonNode["content"]["url"].asString());
                                if (L"json" == BEFILENAME(GetExtension, contentURL))
                                {
                                    // the tile references a new tileset (group)
                                    static std::atomic<uint32_t> s_currentGroupID = 0;
                                    //assert(this->m_tileTreeChildrenGroups.count(++s_currentGroupID) == 0);

                                    SMNodeGroup::Ptr newGroup = SMNodeGroup::CreateCesium3DTilesGroup(this->GetDataSourceAccount(), *this->m_nodeHeaders, s_currentGroupID);
                                    newGroup->SetURL(DataSourceURL(contentURL.c_str()));
                                    newGroup->SetDataSourcePrefix(this->m_dataSourcePrefix);
                                    //newGroup->SetDataSourceExtension(this->m_dataSourceExtension);
                                    newGroup->m_RootTileTreeNode = jsonNode;
                                    newGroup->SaveNode(nodeID, &newGroup->m_RootTileTreeNode);

                                    // we are done processing this part of the tileset
                                    return;
                                }
                            }
                            jsonNode["isLeafNode"] = true;
                            this->SaveNode(nodeID, &jsonNode);

                            // we are done processing this part of the tileset
                            return;
                        }

                        this->SaveNode(nodeID, &jsonNode);
                        for (auto& child : jsonNode["children"])
                        {
                            tileTreeMapGenerator(child, nodeID, false, level + 1);
                        }
                    };
                    tileTreeMapGenerator(m_RootTileTreeNode["root"], parentID, true, rootLevel);
                }
                else {
                    Json::Reader    reader;
                    char* jsonBlob = reinterpret_cast<char *>(dest.get());
                    reader.parse(jsonBlob, jsonBlob + readSize, m_RootTileTreeNode);

                    if (!(m_RootTileTreeNode.isMember("root") && m_RootTileTreeNode["root"].isMember("SMHeader")))
                    {
                        assert(!"error reading Cesium 3D tiles header");
                        return 0;
                    }
                    bool checkResult = false;
                    auto nodeIDChecker = [priorityNodeID, &checkResult](uint64_t id = 0) -> bool
                        {
                        if (!checkResult) checkResult = priorityNodeID == id;
                        return checkResult;
                        };
                    // Generate tile tree map
                    std::function<void(Json::Value&)> tileTreeMapGenerator = [this, &tileTreeMapGenerator, &nodeIDChecker](Json::Value& jsonNode)
                    {
                        if (jsonNode.isMember("SMHeader") && jsonNode["SMHeader"].isMember("id"))
                        {
                            uint32_t nodeID = jsonNode["SMHeader"]["id"].asUInt();
                            nodeIDChecker(nodeID);
                            //this->m_tileTreeMap[nodeID] = &jsonNode["SMHeader"];
                            //{
                            //    // Indicate that the header for the node with id nodeID is ready to be consumed
                            //    // This is to unblock other threads that might be waiting for this group to complete loading.
                            //    auto nodeHeader = this->GetNodeHeader(nodeID);
                            //    assert(nodeHeader != nullptr);
                            //    nodeHeader->offset = 0;
                            //}
                            this->SaveNode(nodeID, &jsonNode);
                            if (jsonNode.isMember("children"))
                            {
                                for (auto& child : jsonNode["children"])
                                {
                                    tileTreeMapGenerator(child);
                                }
                            }
                        }
                        else
                        {
                            assert(jsonNode.isMember("content") && jsonNode["content"].isMember("url"));
                            BeFileName contentURL(jsonNode["content"]["url"].asString());
                            WString groupIDStr = BEFILENAME(GetFileNameWithoutExtension, contentURL);
                            WString prefix = groupIDStr.substr(0, 2);
                            WString extension = BEFILENAME(GetExtension, contentURL);
                            groupIDStr = groupIDStr.substr(2, groupIDStr.size()); // remove prefix
                            uint64_t groupID = std::wcstoull(groupIDStr.begin(), nullptr, 10);
                            SMNodeGroup::Ptr newGroup = SMNodeGroup::CreateCesium3DTilesGroup(this->GetDataSourceAccount(), *this->m_nodeHeaders, groupID);
                            newGroup->SetURL(DataSourceURL(contentURL.c_str()));
                            newGroup->SetDataSourcePrefix(this->m_dataSourcePrefix);
                            //newGroup->SetDataSourceExtension(this->m_dataSourceExtension);
                            newGroup->m_RootTileTreeNode = jsonNode;
                            assert(jsonNode.isMember("SMRootID"));
                            nodeIDChecker(jsonNode["SMRootID"].asUInt());
                            newGroup->SaveNode(jsonNode["SMRootID"].asUInt(), &newGroup->m_RootTileTreeNode);

                            //assert(this->m_tileTreeChildrenGroups.count(groupID) == 0);
                            //SMNodeGroup::Ptr newGroup = SMNodeGroup::CreateCesium3DTilesGroup(this->GetDataSourceAccount(), groupID);
                            //newGroup->SetDataSourcePrefix(this->m_dataSourcePrefix);
                            //newGroup->SetDataSourceExtension(this->m_dataSourceExtension);
                            //this->m_tileTreeChildrenGroups[groupID] = newGroup;
                        }
                    };
                    tileTreeMapGenerator(m_RootTileTreeNode["root"]);
                    assert(nodeIDChecker());
                }
            }
            else
                {
                m_isLoading = false;
                return ERROR;
                }
            m_isLoading = false;
            m_isLoaded = true;
            }
        else
            {
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
    return SUCCESS;
    }

void SMNodeGroup::LoadGroupParallel()
    {
    SMNodeGroup::Ptr group(this);
    std::thread thread([group]()
        {
#ifdef DEBUG_GROUPS
        static uint64_t s_numProcessedNodeId = 0;
        {
        std::lock_guard<mutex> lk(s_consoleMutex);
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
        std::lock_guard<mutex> lk(s_consoleMutex);
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

DataSourceURL SMNodeGroup::GetDataURLForNode(HPMBlockID blockID)
    {
    const auto& jsonNode = *this->m_tileTreeMap[blockID.m_integerID];

    BeFileName dataURL(jsonNode["content"]["url"].asString());
    assert(BEFILENAME(GetExtension, dataURL) == L"b3dm");
    return DataSourceURL(dataURL.c_str());
    }

DataSourceURL SMNodeGroup::GetURL()
    {
    return m_url;
    }

void SMNodeGroup::SetURL(DataSourceURL url)
    {
    m_url = url;
    }

void SMNodeGroup::SetHeaderDataAtCurrentPosition(const uint64_t& nodeID, const uint8_t* rawHeader, const uint64_t& headerSize)
    {
    std::lock_guard<std::mutex> lock(m_groupMutex);
    auto nodeHeader = this->GetNodeHeader(nodeID);
    nodeHeader->size = headerSize;
    nodeHeader->offset = m_currentPosition;
    memmove(m_rawHeaders.data() + m_currentPosition, rawHeader, nodeHeader->size);
    m_currentPosition += nodeHeader->size;
    }

void SMNodeGroup::SaveNode(const uint64_t & id, Json::Value * header)
    {
    std::lock_guard<std::mutex> lock(s_mutex);
    //this->m_tileTreeMap[id] = header;
    (*this->m_nodeHeaders)[id] = header;
    this->s_downloadedGroups[id] = this;
    }

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

void SMNodeGroup::Append3DTile(const uint64_t& nodeID, const uint64_t& parentNodeID, const Json::Value& tile)
    {
    if (!m_tileTreeMap.empty())
        {
        assert(parentNodeID != uint32_t(-1));
        auto parentTilePtr = m_tileTreeMap[parentNodeID];
        assert(parentTilePtr != nullptr);

        Json::Value& children = (*parentTilePtr)["children"];
        m_tileTreeMap[nodeID] = &children.append(tile);
        }
    else
        { 
        // We are adding the first tile in a new group
        // So keep the tile for later reference
        m_RootTileTreeNode = tile;
        m_tileTreeMap[nodeID] = &m_RootTileTreeNode;

        if (m_ParentGroup.IsValid())
            {
            assert(m_ParentGroup->m_tileTreeMap.count(parentNodeID) == 1);

            // We must update the parent's content url so we know how to reference the new tile set
            auto& parentNodeTile = *m_ParentGroup->m_tileTreeMap[parentNodeID];
            auto& parentNodeTileChildren = parentNodeTile["children"];

            // Keep the index before updating the parent tile
            m_RootTileTreeNode["index"] = parentNodeTileChildren.size();

            auto& childTile = parentNodeTileChildren.append(tile);
            childTile.removeMember("children");
            childTile.removeMember("SMHeader");

            childTile["content"]["url"] = Utf8String(("n_" + std::to_string(this->GetID()) + ".json").c_str());
            }
        }
    }

void SMNodeGroup::MergeChild(SMNodeGroup::Ptr child)
    {
    assert(child->m_ParentGroup == this);
    assert(child->m_RootTileTreeNode.isMember("SMHeader") && child->m_RootTileTreeNode["SMHeader"].isMember("parentID"));

    auto& childTileTreeNode = child->m_RootTileTreeNode;
    auto childIndex = childTileTreeNode["index"].asUInt();
    childTileTreeNode.removeMember("index");

    auto parentID = childTileTreeNode["SMHeader"]["parentID"].asUInt();
    auto& parentTileChildren = (*this->m_tileTreeMap[parentID])["children"];

    // Replace tile in the parent tile
    parentTileChildren[childIndex] = childTileTreeNode;

    this->m_tileTreeMap.insert(child->m_tileTreeMap.begin(), child->m_tileTreeMap.end());
    }



