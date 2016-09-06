//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMNodeGroup.h"

#ifndef NDEBUG
std::mutex s_consoleMutex;
#endif

uint32_t s_max_number_nodes_in_group = 100;
size_t s_max_group_size = 256 << 10; // 256 KB
size_t s_max_group_depth = 2;
size_t s_max_group_common_ancestor = 2;

StatusInt SMNodeGroup::Load(const uint64_t& priorityNodeID)
{
    unique_lock<mutex> lk(m_groupMutex, std::defer_lock);
    auto& nodeHeader = this->GetNodeHeader(priorityNodeID);
    if (!lk.try_lock() || m_isLoading)
    {
        // Someone else is loading the group
        if (nodeHeader.size == 0)
            {
            // Data not ready yet, wait until it becomes ready
            if (lk.owns_lock()) lk.unlock();
            this->WaitFor(nodeHeader);
            }
        }
    else 
    {
        assert(m_isLoading == false);
        m_isLoading = true;
        if (s_is_virtual_grouping)
            {
            this->LoadGroupParallel();

            if (lk.owns_lock()) lk.unlock();

            this->WaitFor(nodeHeader);
            }
        else 
        {
            assert(m_isLoading == false);

            std::unique_ptr<DataSource::Buffer[]>       dest;
            DataSource                              *   dataSource;
            DataSource::DataSize                        readSize;

            DataSourceBuffer::BufferSize                destSize = 5 * 1024 * 1024;

            m_isLoading = true;

            dataSource = this->InitializeDataSource(dest, destSize);
            if (dataSource == nullptr)
            {
                m_isLoading = false;
                m_groupCV.notify_all();
                return ERROR;
            }

            this->LoadFromDataSource(dataSource, dest.get(), destSize, readSize);

            if (readSize > 0)
            {
                uint32_t position = 0;
                size_t id;
                memcpy(&id, dest.get(), sizeof(size_t));
                assert(m_groupHeader->GetID() == id);
                position += sizeof(size_t);

                size_t numNodes;
                memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
                assert(m_groupHeader->size() == numNodes);
                position += sizeof(numNodes);

                memcpy(m_groupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
                position += (uint32_t)numNodes * sizeof(SMNodeHeader);

                const auto headerSectionSize = readSize - position;
                m_rawHeaders.resize(headerSectionSize);
                memcpy(m_rawHeaders.data(), dest.get() + position, headerSectionSize);
            }
            else
            {
                m_isLoading = false;
                m_groupCV.notify_all();
                return ERROR;
            }
        }
    }
    assert(nodeHeader.size > 0);
    return SUCCESS;
}

void SMNodeGroup::LoadGroupParallel()
    {
    HFCPtr<SMNodeGroup> group(this);
    std::thread thread([group]()
        {
#ifdef DEBUG_GROUPS
        static uint64_t s_numProcessedNodeId = 0;
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] Distributing... " << std::endl;
        s_consoleMutex.unlock();
#endif
        assert(group->m_nodeDistributorPtr != nullptr);
        for (auto nodeHeader : *group->m_groupHeader)
            {
            group->m_nodeDistributorPtr->AddWorkItem(DistributeData(nodeHeader.blockid, group.GetPtr()));
            }
#ifdef DEBUG_GROUPS
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] waiting for nodes to process... " << std::endl;
        s_consoleMutex.unlock();
#endif
        group->m_nodeDistributorPtr->Wait([group]()
            {
            return group->m_progress == group->m_groupHeader->size();
            });
#ifdef DEBUG_GROUPS
        s_numProcessedNodeId += group->m_groupHeader->size();
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] " << group->m_groupHeader->size() << " nodes (total " << s_numProcessedNodeId << ")" << std::endl;
        s_consoleMutex.unlock();
#endif
        group->m_isLoading = false;
        group->m_isLoaded = true;
        group->m_groupCV.notify_all();
        });
    thread.detach();
    }

void SMNodeGroup::SetHeaderDataAtCurrentPosition(const uint64_t& nodeID, const uint8_t* rawHeader, const uint64_t& headerSize)
    {
    std::lock_guard<std::mutex> lock(m_groupMutex);
    auto& nodeHeader = this->GetNodeHeader(nodeID);
    nodeHeader.size = headerSize;
    nodeHeader.offset = m_currentPosition;
    memmove(m_rawHeaders.data() + m_currentPosition, rawHeader, nodeHeader.size);
    m_currentPosition += nodeHeader.size;
    }
