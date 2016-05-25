/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MemoryManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeSystemInfo.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MemoryManager::AddConsumer(MemoryConsumer& consumer, MemoryConsumer::Priority priority)
    {
    auto existing = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_consumer == &consumer; });
    BeAssert(m_entries.end() == existing && "Consumer already registered");
    if (m_entries.end() == existing)
        {
        auto at = m_entries.end();
        switch (priority)
            {
            case MemoryConsumer::Priority::VeryLow:
                at = m_entries.begin();
                break;
            case MemoryConsumer::Priority::Highest:
                BeAssert(nullptr != dynamic_cast<DgnElements*>(&consumer) && "Highest priority is reserved for DgnElements table");
                break;
            default:
                at = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return static_cast<int>(arg.m_priority) >= static_cast<int>(priority); });
                break;
            }

        Entry newEntry(consumer, priority);
        if (m_entries.end() == at)
            m_entries.push_back(newEntry);
        else
            m_entries.insert(at, newEntry);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MemoryManager::DropConsumer(MemoryConsumer& consumer)
    {
    auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_consumer == & consumer; });
    BeAssert(m_entries.end() != found && "Consumer not registered");
    if (m_entries.end() != found)
        m_entries.erase(found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t MemoryManager::CalculateBytesConsumed() const
    {
    uint64_t total = 0;
    for (auto const& entry : m_entries)
        total += entry.m_consumer->_CalculateBytesConsumed();

    return total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool MemoryManager::PurgeUntil(uint64_t memTarget)
    {
    // Calculating bytes consumed should be reasonably efficient, but avoid doing so more than necessary
    uint64_t totalConsumed = 0;
    ScopedArray<uint64_t> bytesPerConsumer(m_entries.size());
    for (size_t i = 0; i < m_entries.size(); i++)
        {
        uint64_t consumed = m_entries[i].m_consumer->_CalculateBytesConsumed();
        bytesPerConsumer.GetData()[i] = consumed;
        totalConsumed += consumed;
        }

    if (totalConsumed <= memTarget)
        return true;

    // traverse in order of lowest to highest priority, so that more important memory is not reclaimed before less important
    for (size_t i = 0; i < m_entries.size(); ++i)
        {
        uint64_t consumed = bytesPerConsumer.GetData()[i];
        if (consumed == 0)
            continue;

        // Ask all but the highest-priority consumer to relinquish all reclaimable memory
        uint64_t consumerTarget = (m_entries.size()-1 == i) ? memTarget : 0;
        MemoryConsumer& consumer = *(m_entries[i].m_consumer);
        uint64_t nowConsumed = consumer._Purge(consumerTarget);
        BeAssert(nowConsumed <= consumed && "Purging memory caused more memory to be consumed?");

        uint64_t reclaimed = consumed - nowConsumed;
        totalConsumed -= reclaimed;

        if (totalConsumed <= memTarget)
            return true; // cumulative purges have satisfied our memory target
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryManager::MemoryManager()
    {
    enum Sizes : uint64_t {K=1024, MEG=K*K, GIG=K*MEG,};
#if defined (_X64_)
    m_targetMemorySize = 4 * GIG;
#elif defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    m_targetMemorySize =  2 * GIG;
#else
    m_targetMemorySize = (BeSystemInfo::GetAmountOfPhysicalMemory() > (600 * MEG)) ? 50*MEG : 30*MEG;
#endif
    }
