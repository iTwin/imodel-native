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
            if (lk.owns_lock()) lk.unlock();
            this->WaitFor(nodeHeader);
            }
        }
    else 
	{
        assert(m_pIsLoading == false);
        m_pIsLoading = true;
        if (s_is_virtual_grouping)
            {
            this->LoadGroupParallel();

            if (lk.owns_lock()) lk.unlock();

            this->WaitFor(nodeHeader);
		    }
		else 
		{
			assert(m_pIsLoading == false);

			std::unique_ptr<DataSource::Buffer[]>		dest;
			DataSource								*	dataSource;
			DataSource::DataSize						readSize;

			DataSourceBuffer::BufferSize				destSize = 5 * 1024 * 1024;

			m_pIsLoading = true;

			dataSource = initializeDataSource(dest, destSize);
			if (dataSource == nullptr)
			{
				m_pIsLoading = false;
				m_pGroupCV.notify_all();
				return ERROR;
			}

			loadFromDataSource(dataSource, dest.get(), destSize, readSize);

			if (readSize > 0)
			{
				uint32_t position = 0;
				size_t id;
				memcpy(&id, dest.get(), sizeof(size_t));
				assert(m_pGroupHeader->GetID() == id);
				position += sizeof(size_t);

				size_t numNodes;
				memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
				assert(m_pGroupHeader->size() == numNodes);
				position += sizeof(numNodes);

				memcpy(m_pGroupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
				position += (uint32_t)numNodes * sizeof(SMNodeHeader);

				const auto headerSectionSize = readSize - position;
				m_pRawHeaders.resize(headerSectionSize);
				memcpy(m_pRawHeaders.data(), dest.get() + position, headerSectionSize);
			}
			else
			{
				m_pIsLoading = false;
				m_pGroupCV.notify_all();
				return ERROR;
			}
		}

		m_pIsLoading = false;
		m_pIsLoaded = true;
		m_pGroupCV.notify_all();
	}

	assert(nodeHeader.size > 0);
	return SUCCESS;
}


StatusInt SMNodeGroup::Load_Old(const uint64_t& priorityNodeID)
    {
    unique_lock<mutex> lk(m_pGroupMutex, std::defer_lock);
    auto& nodeHeader = this->GetNodeHeader(priorityNodeID);
    if (!lk.try_lock() || m_pIsLoading)
        {
        // Someone else is loading the group
        if (nodeHeader.size == 0)
            {
            // Data not ready yet, wait until it becomes ready
            if (lk.owns_lock()) lk.unlock();
            this->WaitFor(nodeHeader);
            }
        }
    else {
        assert(m_pIsLoading == false);
        m_pIsLoading = true;
        if (s_is_virtual_grouping)
            {
            this->LoadGroupParallel();
            if (lk.owns_lock()) lk.unlock();
            this->WaitFor(nodeHeader);
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
    HFCPtr<SMNodeGroup> group(this);
    std::thread thread([group]()
        {
#ifdef DEBUG_GROUPS
        static uint64_t s_numProcessedNodeId = 0;
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] Distributing... " << std::endl;
        s_consoleMutex.unlock();
#endif
        for (auto nodeHeader : *group->m_pGroupHeader)
            {
            group->m_NodeDistributorPtr->AddWorkItem(DistributeData(nodeHeader.blockid, group.GetPtr()));
            }
#ifdef DEBUG_GROUPS
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] waiting for nodes to process... " << std::endl;
        s_consoleMutex.unlock();
#endif
        group->m_NodeDistributorPtr->Wait([group]()
            {
            return group->m_pProgress == group->m_pGroupHeader->size();
            });
#ifdef DEBUG_GROUPS
        s_numProcessedNodeId += group->m_pGroupHeader->size();
        s_consoleMutex.lock();
        std::cout << "[" << std::this_thread::get_id() << "," << group->GetID() << "] " << group->m_pGroupHeader->size() << " nodes (total " << s_numProcessedNodeId << ")" << std::endl;
        s_consoleMutex.unlock();
#endif
        group->m_pIsLoading = false;
        group->m_pIsLoaded = true;
        group->m_pGroupCV.notify_all();
        });
    thread.detach();
    }

void SMNodeGroup::SetHeaderDataAtCurrentPosition(const uint64_t& nodeID, const uint8_t* rawHeader, const uint64_t& headerSize)
    {
    std::lock_guard<std::mutex> lock(m_pGroupMutex);
    auto& nodeHeader = this->GetNodeHeader(nodeID);
    nodeHeader.size = headerSize;
    nodeHeader.offset = m_pCurrentPosition;
    memmove(m_pRawHeaders.data() + m_pCurrentPosition, rawHeader, nodeHeader.size);
    m_pCurrentPosition += nodeHeader.size;
    }
