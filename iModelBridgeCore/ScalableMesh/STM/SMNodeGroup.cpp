//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMNodeGroup.h"

uint32_t s_max_number_nodes_in_group = 100;
size_t s_max_group_size = 256 << 10; // 256 KB
size_t s_max_group_depth = 2;
size_t s_max_group_common_ancestor = 2;

StatusInt SMNodeGroup::Load(const uint64_t& priorityNodeID)
    {
    unique_lock<mutex> lk(m_pGroupMutex, std::defer_lock);
    auto& nodeHeader = this->GetNodeHeader(priorityNodeID);
    if (!lk.try_lock() || m_pIsLoading)
        {
        // Someone else is loading the group
        if (nodeHeader.size == 0)
            {
            // Data not ready yet, wait until it becomes ready
#ifdef DEBUG_GROUPS
            globalGroupMtx.lock();
            std::cout << "[" << this->m_pGroupHeader->GetID() << "] is being put to sleep" << std::endl;
            globalGroupMtx.unlock();
#endif
            if (!lk.owns_lock()) lk.lock();
            m_pGroupCV.wait(lk, [this, &nodeHeader]
                {
                if (!m_pIsLoading && m_pIsLoaded) return true;
                return nodeHeader.size > 0;
                });
#ifdef DEBUG_GROUPS
            globalGroupMtx.lock();
            std::cout << "[" << this->m_pGroupHeader->GetID() << "] is waking up..." << std::endl;
            globalGroupMtx.unlock();
#endif
            }
        }
    else {
        assert(m_pIsLoading == false);
        m_pIsLoading = true;
        if (s_is_virtual_grouping)
            {
            this->LoadGroupParallel();
            if (!lk.owns_lock()) lk.lock();
            m_pGroupCV.wait(lk, [this, &nodeHeader]
                {
                if (!m_pIsLoading && m_pIsLoaded) return true;
                return nodeHeader.size > 0;
                }); // not ready yet
            }
        else {
            std::unique_ptr<uint8_t> inBuffer = nullptr;
            uint32_t bytes_read = 0;
            m_pIsLoading = true;
            if (s_stream_from_disk && SUCCESS != this->LoadFromLocal(inBuffer, bytes_read))
                {
                m_pIsLoading = false;
                m_pGroupCV.notify_all();
                return ERROR;
                }
            if (!s_stream_from_disk && SUCCESS != this->LoadFromAzure(inBuffer, bytes_read))
                {
                m_pIsLoading = false;
                m_pGroupCV.notify_all();
                return ERROR;
                }
            uint32_t position = 0;
            size_t id;
            memcpy(&id, inBuffer.get(), sizeof(size_t));
            assert(m_pGroupHeader->GetID() == id);
            position += sizeof(size_t);

            size_t numNodes;
            memcpy(&numNodes, inBuffer.get() + position, sizeof(numNodes));
            assert(m_pGroupHeader->size() == numNodes);
            position += sizeof(numNodes);

            memcpy(m_pGroupHeader->data(), inBuffer.get() + position, numNodes * sizeof(SMNodeHeader));
            position += (uint32_t)numNodes * sizeof(SMNodeHeader);

            const auto headerSectionSize = bytes_read - position;
            m_pRawHeaders.resize(headerSectionSize);
            memcpy(m_pRawHeaders.data(), inBuffer.get() + position, headerSectionSize);
            m_pIsLoading = false;
            m_pIsLoaded = true;
            m_pGroupCV.notify_all();
            }
        }
    assert(nodeHeader.size > 0);
    return SUCCESS;
    }

void SMNodeGroup::LoadGroupParallel()
    {
    std::condition_variable cv;
    std::thread thread(std::bind([&cv](SMNodeGroup* group) ->void
        {
        std::shared_ptr<uint32_t> currentPosition(new uint32_t(0));
        std::shared_ptr<std::mutex> rawHeadersUpdateMutex(new std::mutex());
#ifdef DEBUG_GROUPS
        static uint64_t s_numProcessedNodeId = 0;
        globalGroupMtx.lock();
        std::cout << "[" << group->GetID() << "] Distributing... " << std::endl;
        globalGroupMtx.unlock();
#endif
        group->m_pNodeFetchDistributor.SetJob([group, currentPosition, rawHeadersUpdateMutex](uint64_t nodeId)
            {
            //#ifdef DEBUG_GROUPS
            //                std::cout << "Processing... " << nodeId << std::endl;
            //#endif
            uint8_t* rawHeader = new uint8_t[10000];
            auto& nodeHeader = group->GetNodeHeader(nodeId);
            auto headerSize = group->GetSingleNodeFromStore(nodeId, rawHeader);
            std::lock_guard<std::mutex> lock(*rawHeadersUpdateMutex);
            nodeHeader.size = headerSize;
            nodeHeader.offset = *currentPosition;
            memmove(group->m_pRawHeaders.data() + *currentPosition, rawHeader, nodeHeader.size);
            *currentPosition += nodeHeader.size;
            delete rawHeader;
            group->m_pGroupCV.notify_all();
            });
        for (auto nodeHeader : *group->m_pGroupHeader) group->m_pNodeFetchDistributor(std::move(nodeHeader.blockid));
#ifdef DEBUG_GROUPS
        globalGroupMtx.lock();
        std::cout << "[" << group->GetID() << "] waiting for nodes to process... " << std::endl;
        globalGroupMtx.unlock();
#endif
        group->m_pNodeFetchDistributor.Wait();
#ifdef DEBUG_GROUPS
        s_numProcessedNodeId += group->m_pGroupHeader->size();
        globalGroupMtx.lock();
        std::cout << "[" << group->GetID() << "] " << group->m_pGroupHeader->size() << " nodes (total " << s_numProcessedNodeId << ")" << std::endl;
        globalGroupMtx.unlock();
#endif
        group->m_pIsLoading = false;
        group->m_pIsLoaded = true;
        group->m_pGroupCV.notify_all();
        }, this));
    thread.detach();
    }

