/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MemoryManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS(MemoryConsumer);
DGNPLATFORM_TYPEDEFS(MemoryManager);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface adopted by an object associated with a DgnDb which consumes memory - 
//! typically some type of cache.
//! Such objects should be registered with the MemoryManager associated with the DgnDb
//! so that they may cooperate in ensuring limitations on memory consumption are not
//! exceeded.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MemoryConsumer
{
    //! Identifies the relative importance of the memory allocated to a consumer
    //! When the memory manager needs to reclaim memory, it does so from consumers with lower priority first.
    enum class Priority
    {
        VeryLow = 10, //!< Memory is reclaimed from this consumer before any higher-priority consumers
        Low = 20, //!< Memory is reclaimed from VeryLow-priority consumers before this consumer
        Moderate = 30, //!< Memory is reclaimed from Low- and VeryLow-priority consumers before this consumer
        High = 40, //!< Memory is reclaimed from all but Critical-priority consumers before this consumer
        Critical = 50, //!< Memory is reclaimed from all lower-priority consumers before this consumer
        Highest = 100, //!< Reserved for the DgnElements table
    };

    //! Calculate to a reasonable approximation the total amount of memory allocated to this consumer
    //! @return The number of bytes allocated by this consumer.
    virtual uint64_t _CalculateBytesConsumed() const = 0;

    //! Purge reclaimable memory until the desired target is attained or all such memory is released.
    //! @param[in]      memTarget The maximum number of bytes that should be used by this consumer.
    //! @return The number of remaining bytes allocated by this consumer following the purge.
    virtual uint64_t _Purge(uint64_t memTarget) = 0;
};

//=======================================================================================
//! Manages the memory associated with a DgnDb that has been allocated to registered memory consumers. 
//! When the system requires that memory be reclaimed, the MemoryManager requests 
//! each consumer to relinquish reclaimableable memory in order of consumer priority, 
//! until the desired memory target is reached or no further memory can be reclaimed.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct MemoryManager
{
private:
    struct Entry
    {
        MemoryConsumer* m_consumer;
        MemoryConsumer::Priority m_priority;

        Entry() : m_consumer(nullptr), m_priority(MemoryConsumer::Priority::VeryLow) { }
        Entry(MemoryConsumer& consumer, MemoryConsumer::Priority priority) : m_consumer(&consumer), m_priority(priority) { }
    };

    typedef bvector<Entry> Entries;
    Entries m_entries;
    uint64_t m_targetMemorySize;

public:
    DGNPLATFORM_EXPORT MemoryManager();

    //! Set the target number of bytes for future Purge operations.
    void SetMemoryTarget(uint64_t numBytes) {m_targetMemorySize = numBytes;}
    
    //! Get the current target number of bytes for Purge operations.
    uint64_t GetCurrentMemoryTarget() const {return m_targetMemorySize;}

    //! Register a memory consumer, in order to participate in reclaiming memory when needed.
    //! @param[in] consumer The consumer to register
    //! @param[in] priority The relative priority of the memory managed by this consumer. Memory is reclaimed from lower-priority consumers first.
    DGNPLATFORM_EXPORT void AddConsumer(MemoryConsumer& consumer, MemoryConsumer::Priority priority);

    //! Unregister a previously-registered memory consumer
    //! @param[in] consumer The consumer to unregister
    DGNPLATFORM_EXPORT void DropConsumer(MemoryConsumer& consumer);

    //! Calculates to a reasonable approximation the total amount of memory allocated to all consumers registered to this manager.
    //! @return The number of bytes of memory allocated to all consumers
    DGNPLATFORM_EXPORT uint64_t CalculateBytesConsumed() const;

    //! Purge memory until only a target amount of memory is consumed. Memory is freed from lowest to highest priority consumers, 
    //! until the specified memory target is reached or no further memory can be reclaimed.
    //! @param[in] memTarget The desired maximum total number of bytes to allow for all consumers.
    //! @return true if the specified memory target was attained.
    //! @note There is no guarantee that the specified target is attainable, as the memory managed by consumers may be referenced and therefore not reclaimable.
    DGNPLATFORM_EXPORT bool PurgeUntil(uint64_t memTarget);

    //! Purge memory until current memory target is reached. 
    //! @see PurgeUntil, GetCurrentMemoryTarget
    DGNPLATFORM_EXPORT bool Purge() {return PurgeUntil(GetCurrentMemoryTarget());}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

