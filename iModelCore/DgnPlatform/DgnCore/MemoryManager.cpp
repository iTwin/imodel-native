/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MemoryManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/ScopedArray.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MemoryManager::AddConsumer(IMemoryConsumer& consumer, Priority priority)
    {
    auto existing = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_consumer == &consumer; });
    BeAssert(m_entries.end() == existing && "Consumer already registered");
    if (m_entries.end() == existing)
        {
        auto at = m_entries.end();
        switch (priority)
            {
            case Priority::VeryLow:
                at = m_entries.begin();
                break;
            case Priority::Highest:
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
void MemoryManager::DropConsumer(IMemoryConsumer& consumer)
    {
    auto found = std::find_if(m_entries.begin(), m_entries.end(), [&](Entry const& arg) { return arg.m_consumer == & consumer; });
    BeAssert(m_entries.end() != found && "Consumer not registered");
    if (m_entries.end() != found)
        m_entries.erase(found);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t MemoryManager::CalculateBytesConsumed() const
    {
    int64_t total = 0;
    for (auto const& entry : m_entries)
        {
        auto consumed = entry.m_consumer->CalculateBytesConsumed();
        BeAssert(consumed >= 0);
        total += consumed;
        }

    return total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool MemoryManager::Purge(int64_t memTarget)
    {
    if (memTarget < 0)
        memTarget = 0;

    // Calculating bytes consumed should be reasonably efficient, but avoid doing so more than necessary
    int64_t totalConsumed = 0;
    ScopedArray<int64_t> bytesPerConsumer(m_entries.size());
    for (size_t i = 0; i < m_entries.size(); i++)
        {
        int64_t consumed = m_entries[i].m_consumer->CalculateBytesConsumed();
        bytesPerConsumer.GetData()[i] = consumed;
        totalConsumed += consumed;
        }

    if (totalConsumed <= memTarget)
        return true;

    // traverse in order of lowest to highest priority, so that more important memory is not reclaimed before less important
    for (size_t i = 0; i < m_entries.size(); i++)
        {
        int64_t consumed = bytesPerConsumer.GetData()[i];
        if (consumed <= 0)
            continue;

        // Ask all but the highest-priority consumer to relinquish all reclaimable memory
        int64_t consumerTarget = (m_entries.size()-1 == i) ? memTarget : 0;
        IMemoryConsumer& consumer = *(m_entries[i].m_consumer);
        int64_t nowConsumed = consumer.Purge(consumerTarget);
        BeAssert(nowConsumed <= consumed && "Purging memory caused more memory to be consumed?");

        int64_t reclaimed = consumed - nowConsumed;
        totalConsumed -= reclaimed;

        if (totalConsumed <= memTarget)
            return true; // cumulative purges have satisfied our memory target
        }

    return false;
    }

